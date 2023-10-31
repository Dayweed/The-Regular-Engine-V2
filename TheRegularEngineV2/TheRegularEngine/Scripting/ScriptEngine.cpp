#include "pch.h"
#include "ScriptEngine.h"

#include "mono/metadata/object.h"
#include "mono/metadata/tabledefs.h"
#include "mono/metadata/mono-debug.h"
#include "mono/metadata/threads.h"

//Everything should be remove. This is just to test the calling of the runtime works.
#include <fstream>

#include "ScriptBind.h"
#include "ScriptComponent.h"
#include "Core/Logger.h"

namespace TRE
{

#pragma region ScriptEngine

	ScriptEngineData* ScriptEngine::s_ScriptEngineData = nullptr;
	static void GetCSEntityData(Entity entity);
	static void UpdateCSEntityData(Entity entity);
	static void CreateCSEntityData(Entity entity);

	static std::unordered_map<std::string, ScriptFieldTypes> s_ScriptFieldTypeMap =
	{
		{ "System.Single", ScriptFieldTypes::Float },
		{ "System.Double", ScriptFieldTypes::Double },
		{ "System.Boolean", ScriptFieldTypes::Boolean },
		{ "System.Char", ScriptFieldTypes::Char },
		{ "System.Int16", ScriptFieldTypes::Short },
		{ "System.Int32", ScriptFieldTypes::Int },
		{ "System.Int64", ScriptFieldTypes::Long },
		{ "System.Byte", ScriptFieldTypes::Byte },
		{ "System.UInt16", ScriptFieldTypes::UnsignedShort },
		{ "System.UInt32", ScriptFieldTypes::UnsignedInt },
		{ "System.UInt64", ScriptFieldTypes::UnsignedLong },
		{ "System.String", ScriptFieldTypes::String },

		{ "TRE.Vector2", ScriptFieldTypes::Vector2 },
		{ "TRE.Vector3", ScriptFieldTypes::Vector3 },
		{ "TRE.Vector4", ScriptFieldTypes::Vector4 },

		{ "TRE.Entity", ScriptFieldTypes::Entity },
	};

	namespace Tools
	{ 
		char* ReadBytes(const std::string& filepath, uint32_t* outSize)
		{
			std::ifstream stream(filepath, std::ios::binary | std::ios::ate);

			if (!stream)
			{
				// Failed to open the file
				return nullptr;
			}

			std::streampos end = stream.tellg();
			stream.seekg(0, std::ios::beg);
			uint32_t size = static_cast<uint32_t>(end - stream.tellg());

			if (size == 0)
			{
				// File is empty
				return nullptr;
			}

			char* buffer = new char[size];
			stream.read((char*)buffer, size);
			stream.close();

			*outSize = size;
			return buffer;
		}

		MonoAssembly* LoadCSharpAssembly(const std::string& assemblyPath)
		{
			uint32_t fileSize = 0;
			char* fileData = ReadBytes(assemblyPath, &fileSize);

			// NOTE: We can't use this image for anything other than loading the assembly because this image doesn't have a reference to the assembly
			MonoImageOpenStatus status;
			MonoImage* image = mono_image_open_from_data_full(fileData, fileSize, 1, &status, 0);

			if (status != MONO_IMAGE_OK)
			{
				const char* errorMessage = mono_image_strerror(status);
				(void)errorMessage;
				// Log some error message using the errorMessage data
				return nullptr;
			}

			MonoAssembly* assembly = mono_assembly_load_from_full(image, assemblyPath.c_str(), &status, 0);
			mono_image_close(image);

			// Don't forget to free the file data
			delete[] fileData;

			return assembly;
		}

		void PrintAssemblyTypes(MonoAssembly* assembly)
		{
			MonoImage* image = mono_assembly_get_image(assembly);
			const MonoTableInfo* typeDefinitionsTable = mono_image_get_table_info(image, MONO_TABLE_TYPEDEF);
			int32_t numTypes = mono_table_info_get_rows(typeDefinitionsTable);

			for (int32_t i = 0; i < numTypes; i++)
			{
				uint32_t cols[MONO_TYPEDEF_SIZE];
				mono_metadata_decode_row(typeDefinitionsTable, i, cols, MONO_TYPEDEF_SIZE);

				const char* nameSpace = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAMESPACE]);
				const char* name = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAME]);

				printf("%s.%s\n", nameSpace, name);
			}
		}

		ScriptFieldTypes ConvertMonoType(MonoType* monoType)
		{
			std::string typeName = mono_type_get_name(monoType);

			auto it = s_ScriptFieldTypeMap.find(typeName);
			if(it == s_ScriptFieldTypeMap.end())
			{
				return ScriptFieldTypes::None;
			}

			return it->second;
		}

		void PrintAllContainers()
		{

			for(auto& scriptInstance : ScriptEngine::s_ScriptEngineData->ScriptInstances)
			{
				TRE_CORE_INFO("Instance: {0} ", scriptInstance.first);
			}

			
		}

	}



	void ScriptEngine::Init()
	{
		s_ScriptEngineData = new ScriptEngineData();

		InitMono();
		ScriptBind::RegisterFunctions();

		bool status = LoadAssembly("../Resources/Scripts/TRE-ScriptCore.dll");
		if(!status)
		{
			TRE_CORE_ERROR("Failed to load assembly");
		}

		// Here we will load the project assembly when the project script and core script is separated.

		// Load all the classes from the assembly
		LoadClassesFromAssembly();

		// Register all ECS components to the scripting engine

		s_ScriptEngineData->MainClass = ScriptClass("TRE", "Entity");
	}

	void ScriptEngine::Shutdown()
	{
		ShutdownMono();
		delete s_ScriptEngineData;
	}

	void ScriptEngine::InitMono()
	{
		mono_set_assemblies_path("../TheRegularEditor/mono");

		MonoDomain* rootDomain = mono_jit_init("MyScriptRuntime");
		if (rootDomain == nullptr)
		{
			// Maybe log some error here
			return;
		}

		// Store the root domain pointer
		s_ScriptEngineData->RootDomain = rootDomain;
	}

	void ScriptEngine::ShutdownMono()
	{
		//unload the root domain
		mono_domain_set(mono_get_root_domain(), false);

		// here add in unloading off app domain

		mono_jit_cleanup(s_ScriptEngineData->RootDomain);
		s_ScriptEngineData->RootDomain = nullptr;

	}

	bool ScriptEngine::LoadAssembly(const std::string& assemblyPath)
	{
		s_ScriptEngineData->AppDomain = mono_domain_create_appdomain(const_cast<char*>("TREScriptRuntime"), nullptr);
		mono_domain_set(s_ScriptEngineData->AppDomain, true);


		s_ScriptEngineData->MonoAssemblyPath = assemblyPath;
		s_ScriptEngineData->MonoAssembly = Tools::LoadCSharpAssembly(assemblyPath);
		if(s_ScriptEngineData->MonoAssembly == nullptr)
			return false;

		// Store the assembly image
		s_ScriptEngineData->AssemblyImage = mono_assembly_get_image(s_ScriptEngineData->MonoAssembly);

		// For Debugging to check what classes are in the assembly
		//Tools::PrintAssemblyTypes(s_ScriptEngineData->MonoAssembly);

		return true;
	}

	// Setup all the classes that should be linked to the scripting engine.
	void ScriptEngine::LoadClassesFromAssembly()
	{
		// clear the unordered map
		s_ScriptEngineData->ScriptClasses.clear();

		// Change the AssemblyImage to AppAssemblyImage when project script and core script is separated.
		const MonoTableInfo* typeDefinitionsTable = mono_image_get_table_info(s_ScriptEngineData->AssemblyImage , MONO_TABLE_TYPEDEF);
		int32_t numTypes = mono_table_info_get_rows(typeDefinitionsTable);
		MonoClass* entity = mono_class_from_name(s_ScriptEngineData->AssemblyImage, "TRE", "Entity");

		for (int32_t i = 0; i < numTypes; i++)
		{
			uint32_t cols[MONO_TYPEDEF_SIZE];
			mono_metadata_decode_row(typeDefinitionsTable, i, cols, MONO_TYPEDEF_SIZE);

			const char* nameSpace = mono_metadata_string_heap(s_ScriptEngineData->AssemblyImage, cols[MONO_TYPEDEF_NAMESPACE]);
			const char* name = mono_metadata_string_heap(s_ScriptEngineData->AssemblyImage, cols[MONO_TYPEDEF_NAME]);

			std::string className;

			if(strlen(nameSpace) != 0)
				className = nameSpace + std::string(".") + name;
			else
				className = name;

			printf("%s.%s\n", nameSpace, name);

			//Create main class called entity, Also change the assembly image to AppAssemblyImage when project script and core script is separated.
			MonoClass* monoClass = mono_class_from_name(s_ScriptEngineData->AssemblyImage, nameSpace, name);

			if(monoClass == entity)
				continue;

			if(!mono_class_is_subclass_of(monoClass, entity, false))
				continue;

			TRE_CORE_INFO("Found class: {0}", className);
			std::shared_ptr<ScriptClass> scriptClass = std::make_shared<ScriptClass>(nameSpace, name);
			s_ScriptEngineData->ScriptClasses.insert(std::make_pair(className, scriptClass));

			// Load fields of each of the classes that is not the entity class

			// int fieldCount = mono_class_num_fields(monoClass);
			//TRE_CORE_INFO("Field count: {0}", fieldCount);
			void* iterator = nullptr;
			while(MonoClassField* field = mono_class_get_fields(monoClass, &iterator))
			{
				const char* fieldName = mono_field_get_name(field);
				uint32_t flags = mono_field_get_flags(field);
				if(flags & FIELD_ATTRIBUTE_PUBLIC)
				{
					MonoType* fieldType = mono_field_get_type(field);
					ScriptFieldTypes scriptFieldType = Tools::ConvertMonoType(fieldType);
					scriptClass->m_Fields.insert({fieldName,{scriptFieldType, fieldName, field}} );
					//TRE_CORE_INFO("Found field: {0} of type {1}", fieldName, scriptFieldType);
				}
			}
			
		}
	}

	void ScriptEngine::ReloadAssembly()
	{
		// Unload the assembly
		mono_domain_set(mono_get_root_domain(), false);

		mono_domain_unload(s_ScriptEngineData->AppDomain);

		// Load the assembly again
		LoadAssembly(s_ScriptEngineData->MonoAssemblyPath);

		// add loading application assembly when project script and core script is separated.
		// add loading all the classes from the assembly

		LoadClassesFromAssembly();

		// Register back all the components to the scripting engine

		s_ScriptEngineData->MainClass = ScriptClass("TRE", "Entity");

		// Retrieve and instantiate the main class
		InitScriptingMain();
	}

	void ScriptEngine::CreateCSEntityData(Entity entity)
	{
		const auto& scriptComponent = entity->GetComponent<ScriptComponent>();
		if (EntityClassExists(scriptComponent.m_StoredClass))
		{
			std::string GUID = entity->GetGUID();

			if (s_ScriptEngineData->ScriptInstances.find(GUID) != s_ScriptEngineData->ScriptInstances.end())
			{
				std::string function{ __FUNCTION__ };
				TRE_CORE_WARN("[" + function + "] Found Entity " + entity->GetName() + " in s_ScriptEngineData->ScriptInstances!\n");
				return;
			}

			std::shared_ptr<ScriptInstance> instance = std::make_shared<ScriptInstance>(s_ScriptEngineData->ScriptClasses[scriptComponent.m_StoredClass], GUID);
			s_ScriptEngineData->ScriptInstances[GUID] = instance;

			s_ScriptEngineData->EntityFieldMap[GUID];

			ScriptFieldMap& fieldMap = s_ScriptEngineData->EntityFieldMap[GUID];
			
			for (auto& field : fieldMap)
			{
				instance->SetInternalFieldValue(field.first, field.second.m_buffer);
			}
		}
	}
	void ScriptEngine::GetCSEntityData(Entity entity)
	{
		const auto& scriptComponent = entity->GetComponent<ScriptComponent>();
		if (EntityClassExists(scriptComponent.m_StoredClass))
		{
			std::string GUID = entity->GetGUID();

			if (s_ScriptEngineData->ScriptInstances.find(GUID) == s_ScriptEngineData->ScriptInstances.end()) CreateCSEntityData(entity);

			std::shared_ptr<ScriptInstance> instance = s_ScriptEngineData->ScriptInstances[GUID];

			if (s_ScriptEngineData->EntityFieldMap.find(GUID) != s_ScriptEngineData->EntityFieldMap.end())
			{
				ScriptFieldMap& fieldMap = s_ScriptEngineData->EntityFieldMap[GUID];
				for (auto& field : fieldMap)
				{
					instance->SetInternalFieldValue(field.first, field.second.m_buffer);
				}
			}
			else
			{
				std::string function{ __FUNCTION__ };
				TRE_CORE_ERROR("[" + function + "] Can't find " + entity->GetName() + " in s_ScriptEngineData->EntityFieldMap!\n");
			}
		}
	}

	void ScriptEngine::UpdateCSEntityData(Entity entity)
	{
		const auto& scriptComponent = entity->GetComponent<ScriptComponent>();
		if (EntityClassExists(scriptComponent.m_StoredClass))
		{
			std::string GUID = entity->GetGUID();

			if (s_ScriptEngineData->ScriptInstances.find(GUID) == s_ScriptEngineData->ScriptInstances.end()) CreateCSEntityData(entity);

			std::shared_ptr<ScriptInstance> instance = s_ScriptEngineData->ScriptInstances[GUID];

			if (s_ScriptEngineData->EntityFieldMap.find(GUID) != s_ScriptEngineData->EntityFieldMap.end())
			{
				ScriptFieldMap& fieldMap = s_ScriptEngineData->EntityFieldMap[GUID];
				for (auto& field : fieldMap)
				{
					instance->GetInternalFieldValue(field.first, field.second.m_buffer);
				}
			}
			else
			{
				std::string function{ __FUNCTION__ };
				TRE_CORE_ERROR("[" + function + "] Can't find " + entity->GetName() + " in s_ScriptEngineData->EntityFieldMap!\n");
			}
		}
	}

	void ScriptEngine::InitScriptingMain()
	{
		MonoImage* assemblyImage = mono_assembly_get_image(s_ScriptEngineData->MonoAssembly);
		MonoClass* testClass = mono_class_from_name(assemblyImage, "TRE", "Main");

		// creates new instance of the class
		s_ScriptEngineData->DemoObject = mono_object_new(s_ScriptEngineData->AppDomain, testClass);
		// Run constructor of the object class
		mono_runtime_object_init(s_ScriptEngineData->DemoObject);

	}

	void ScriptEngine::UpdateScriptingMain()
	{
		MonoImage* assemblyImage = mono_assembly_get_image(s_ScriptEngineData->MonoAssembly);
		MonoClass* testClass = mono_class_from_name(assemblyImage, "TRE", "Main");
		MonoMethod* method = mono_class_get_method_from_name(testClass, "Update", 0);
		mono_runtime_invoke(method, s_ScriptEngineData->DemoObject, nullptr, nullptr);
	}

	MonoObject* ScriptEngine::InstantiateClass(MonoClass* monoClass)
	{
		MonoObject* instance = mono_object_new(s_ScriptEngineData->AppDomain, monoClass);
		mono_runtime_object_init(instance);
		return instance;
	}

	bool ScriptEngine::EntityClassExists(const std::string& className)
	{
		return s_ScriptEngineData->ScriptClasses.find(className) != s_ScriptEngineData->ScriptClasses.end();
	}

	void ScriptEngine::OnEnableEntity(Entity e)
	{
		std::string GUID = e->GetGUID();
		if (s_ScriptEngineData->ScriptInstances.find(GUID) != s_ScriptEngineData->ScriptInstances.end())
		{
			std::shared_ptr<ScriptInstance> instance = s_ScriptEngineData->ScriptInstances[GUID];
			instance->OnEnableInvoke();
		}
		else
		{
			TRE_CORE_ERROR("Cannot find ScriptInstance for entity {}", GUID);
		}
	}

	void ScriptEngine::OnDisableEntity(Entity e)
	{
		std::string GUID = e->GetGUID();
		if (s_ScriptEngineData->ScriptInstances.find(GUID) != s_ScriptEngineData->ScriptInstances.end())
		{
			std::shared_ptr<ScriptInstance> instance = s_ScriptEngineData->ScriptInstances[GUID];
			instance->OnDisableInvoke();
		}
		else
		{
			TRE_CORE_ERROR("Cannot find ScriptInstance for entity {}", GUID);
		}
	}

	void ScriptEngine::OnDestroyEntity(Entity e)
	{
		std::string GUID = e->GetGUID();
		if (s_ScriptEngineData->ScriptInstances.find(GUID) != s_ScriptEngineData->ScriptInstances.end())
		{
			std::shared_ptr<ScriptInstance> instance = s_ScriptEngineData->ScriptInstances[GUID];
			instance->OnDestroyInvoke();
		}
		else
		{
			TRE_CORE_ERROR("Cannot find ScriptInstance for entity {}", GUID);
		}
	}

	void ScriptEngine::OnCreateEntity(Entity entity)
	{
		const auto& scriptComponent = entity->GetComponent<ScriptComponent>();
		if(EntityClassExists(scriptComponent.m_StoredClass))
		{
			std::string GUID = entity->GetGUID();

			std::shared_ptr<ScriptInstance> instance = std::make_shared<ScriptInstance>(s_ScriptEngineData->ScriptClasses[scriptComponent.m_StoredClass], GUID);
			s_ScriptEngineData->ScriptInstances[GUID] = instance;

			if (s_ScriptEngineData->EntityFieldMap.find(GUID) != s_ScriptEngineData->EntityFieldMap.end())
			{
				ScriptFieldMap& fieldMap = s_ScriptEngineData->EntityFieldMap[GUID];
				for (auto& field : fieldMap)
				{
					instance->SetInternalFieldValue(field.first, field.second.m_buffer);
				}
			}

			instance->OnCreateInvoke();
		}
	}

	void ScriptEngine::OnStartEntity(Entity e)
	{
		std::string GUID = e->GetGUID();
		if(s_ScriptEngineData->ScriptInstances.find(GUID) != s_ScriptEngineData->ScriptInstances.end())
		{
			std::shared_ptr<ScriptInstance> instance = s_ScriptEngineData->ScriptInstances[GUID];
			instance->OnStartInvoke();
		}
		else
		{
			TRE_CORE_ERROR("Cannot find ScriptInstance for entity {}", GUID);
		}
	}

	void ScriptEngine::OnUpdateEntity(Entity e)
	{
		std::string GUID = e->GetGUID();
		if(s_ScriptEngineData->ScriptInstances.find(GUID) != s_ScriptEngineData->ScriptInstances.end())
		{
			std::shared_ptr<ScriptInstance> instance = s_ScriptEngineData->ScriptInstances[GUID];
			instance->OnUpdateInvoke();
		}
		else
		{
			TRE_CORE_ERROR("Cannot find ScriptInstance for entity {}", GUID);
		}
	}

	void ScriptEngine::OnLateUpdateEntity(Entity e)
	{
		std::string GUID = e->GetGUID();
		if(s_ScriptEngineData->ScriptInstances.find(GUID) != s_ScriptEngineData->ScriptInstances.end())
		{
			std::shared_ptr<ScriptInstance> instance = s_ScriptEngineData->ScriptInstances[GUID];
			instance->OnLateUpdateInvoke();
		}
		else
		{
			TRE_CORE_ERROR("Cannot find ScriptInstance for entity {}", GUID);
		}
	}

	void ScriptEngine::OnTriggerStay(Entity e, Entity other)
	{
		std::string GUID = e->GetGUID();
		if (s_ScriptEngineData->ScriptInstances.find(GUID) != s_ScriptEngineData->ScriptInstances.end())
		{
			std::shared_ptr<ScriptInstance> instance = s_ScriptEngineData->ScriptInstances[GUID];
			instance->OnTriggerStayInvoke(other);
		}
		else
		{
			TRE_CORE_ERROR("Cannot find ScriptInstance for entity {}", GUID);
		}
	}

	void ScriptEngine::OnCollisionStay(Entity e, Entity other)
	{
		std::string GUID = e->GetGUID();
		if (s_ScriptEngineData->ScriptInstances.find(GUID) != s_ScriptEngineData->ScriptInstances.end())
		{
			std::shared_ptr<ScriptInstance> instance = s_ScriptEngineData->ScriptInstances[GUID];
			instance->OnCollisionStayInvoke(other);
		}
		else
		{
			TRE_CORE_ERROR("Cannot find ScriptInstance for entity {}", GUID);
		}
	}

	MonoString* ScriptEngine::CreateMonoString(const std::string& string)
	{
		return mono_string_new(s_ScriptEngineData->AppDomain, string.c_str());
	}

	void ScriptEngine::PrintAllContainersHere()
	{
		Tools::PrintAllContainers();
	}

	MonoObject* ScriptEngine::GetManagedInstance(std::string GUID)
	{
		if (s_ScriptEngineData->ScriptInstances.find(GUID) == s_ScriptEngineData->ScriptInstances.end())
		{
			return nullptr;
		}
		else
		{
			return s_ScriptEngineData->ScriptInstances[GUID]->m_Instance;
		}
	}

#pragma endregion

#pragma region ScriptClass

	ScriptClass::ScriptClass(const std::string& classNamespace, const std::string& className ) : m_ClassNamespace(classNamespace), m_ClassName(className)
	{
		m_MonoClass = mono_class_from_name(ScriptEngine::s_ScriptEngineData->AssemblyImage, m_ClassNamespace.c_str(), m_ClassName.c_str());
	}

	MonoObject* ScriptClass::Instantiate()
	{
		return ScriptEngine::InstantiateClass(m_MonoClass);
	}

	MonoMethod* ScriptClass::GetMethod(const std::string& name, int paramCount)
	{
		return mono_class_get_method_from_name(m_MonoClass, name.c_str(), paramCount);
	}

	MonoObject* ScriptClass::InvokeMethod(MonoObject* instance, MonoMethod* method, void** params)
	{
		MonoObject* exception = nullptr;
		MonoObject* result = mono_runtime_invoke(method, instance, params, &exception);
		if (exception)
		{
			mono_print_unhandled_exception(exception);
		}
		return result;
	}

	MonoClass* ScriptClass::GetMonoClass()
	{
		return m_MonoClass;
	}

#pragma endregion

#pragma region ScriptInstance

	ScriptInstance::ScriptInstance(std::shared_ptr<ScriptClass> scriptClass, std::string entity) : m_ScriptClass(scriptClass)
	{
		m_Instance = scriptClass->Instantiate();
		
		m_Constructor = ScriptEngine::s_ScriptEngineData->MainClass.GetMethod(".ctor", 1);
		m_EnableMethod = scriptClass->GetMethod("OnEnable", 0);
		m_DisableMethod = scriptClass->GetMethod("OnDisable", 0);
		m_DestroyMethod = scriptClass->GetMethod("OnDestroy", 0);
		m_CreateMethod = scriptClass->GetMethod("OnCreate", 0);
		m_StartMethod = scriptClass->GetMethod("Start", 0);
		m_UpdateMethod = scriptClass->GetMethod("Update", 0);
		m_LateUpdateMethod = scriptClass->GetMethod("LateUpdate", 0);
		m_TriggerStayMethod = scriptClass->GetMethod("OnTriggerStay", 1);
		m_CollisionStayMethod = scriptClass->GetMethod("OnCollisionStay", 1);

		{
			unsigned long long id = std::stoull(entity);
			void* param = &id;
			m_ScriptClass->InvokeMethod(m_Instance, m_Constructor, &param);
		}
	}

	void ScriptInstance::OnEnableInvoke()
	{
		if(m_EnableMethod)
			m_ScriptClass->InvokeMethod(m_Instance, m_EnableMethod, nullptr);
	}

	void ScriptInstance::OnDisableInvoke()
	{
		if(m_DisableMethod)
			m_ScriptClass->InvokeMethod(m_Instance, m_DisableMethod, nullptr);
	}

	void ScriptInstance::OnDestroyInvoke()
	{
		if(m_DestroyMethod)
			m_ScriptClass->InvokeMethod(m_Instance, m_DestroyMethod, nullptr);
	}

	void ScriptInstance::OnCreateInvoke()
	{
		if(m_CreateMethod)
			m_ScriptClass->InvokeMethod(m_Instance, m_CreateMethod, nullptr);
	}

	void ScriptInstance::OnStartInvoke()
	{
		if(m_StartMethod)
			m_ScriptClass->InvokeMethod(m_Instance, m_StartMethod, nullptr);
	}

	void ScriptInstance::OnUpdateInvoke()
	{
		if(m_UpdateMethod)
			m_ScriptClass->InvokeMethod(m_Instance, m_UpdateMethod, nullptr);
	}

	void ScriptInstance::OnLateUpdateInvoke()
	{
		if(m_LateUpdateMethod)
			m_ScriptClass->InvokeMethod(m_Instance, m_LateUpdateMethod, nullptr);
	}

	void ScriptInstance::OnTriggerStayInvoke(Entity other)
	{
		
		if (m_TriggerStayMethod)
		{
			unsigned long long id = std::stoull(other->GetGUID());
			void* param = &id;
			m_ScriptClass->InvokeMethod(m_Instance, m_TriggerStayMethod, &param);
		}
	}
	

	void ScriptInstance::OnCollisionStayInvoke(Entity other)
	{
		if (m_CollisionStayMethod)
		{
			unsigned long long id = std::stoull(other->GetGUID());
			void* param = &id;
			m_ScriptClass->InvokeMethod(m_Instance, m_CollisionStayMethod, &param);
		}
	}

	bool ScriptInstance::GetInternalFieldValue(const std::string& name, void* buffer)
	{
		const auto& field = m_ScriptClass->GetFields();
		auto iter = field.find(name);
		if (iter == field.end())
			return false;

		const ScriptField& scriptField = iter->second;
		mono_field_get_value(m_Instance, scriptField.m_MonoField, buffer);
		return true;
	}

	bool ScriptInstance::SetInternalFieldValue(const std::string& name, const void* buffer)
	{
		const auto& field = m_ScriptClass->GetFields();
		auto iter = field.find(name);
		if (iter == field.end())
			return false;

		const ScriptField& scriptField = iter->second;
		mono_field_set_value(m_Instance, scriptField.m_MonoField, (void*) buffer);
		return true;
	}

#pragma endregion
	
}
