#include "pch.h"
#include "ScriptEngine.h"
#include "InputHandler/InputHandler.h"

#include "Core/ECS.h"


//Everything should be remove. This is just to test the calling of the runtime works.
#include <fstream>

#include "Audio/AudioSystem.h"
#include "Graphics/Camera.h"
#include "Graphics/MeshRenderer.h"

namespace TRE
{

#pragma region ScriptEngine
    MonoDomain* ScriptEngine::s_RootDomain = nullptr;
    MonoDomain* ScriptEngine::s_AppDomain = nullptr;
    MonoAssembly* ScriptEngine::s_MonoAssembly = nullptr;

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

	void ScriptEngine::InitMono()
	{
        mono_set_assemblies_path("mono");

        MonoDomain* rootDomain = mono_jit_init("MyScriptRuntime");
        if (rootDomain == nullptr)
        {
            // Maybe log some error here
            return;
        }

        // Store the root domain pointer
        s_RootDomain = rootDomain;

        s_AppDomain = mono_domain_create_appdomain(const_cast<char*>("TREScriptRuntime"), nullptr);
        mono_domain_set(s_AppDomain, true);

        s_MonoAssembly = LoadCSharpAssembly("../Resources/Scripts/TRE-ScriptCore.dll");
        PrintAssemblyTypes(s_MonoAssembly);

	}
    void ScriptEngine::BindFunctions()
    {
        // ECS Bindings
        mono_add_internal_call("TRE.ECSManager::CreateEntity", BindCreateEntity);
		std::cout << "CreateEntity" << std::endl;
		mono_add_internal_call("TRE.ECSManager::AddComponent", BindAddComponent);
		std::cout << "AddComponent" << std::endl;
		mono_add_internal_call("TRE.ECSManager::RemoveComponent", BindRemoveComponent);
        std::cout << "Remove Component" << std::endl;
    }

    void ScriptEngine::UpdateScriptingEngine()
    {
        
    }

    void ScriptEngine::TestScriptingEngine()
    {
        MonoImage* assemblyImage = mono_assembly_get_image(s_MonoAssembly);
        MonoClass* testClass = mono_class_from_name(assemblyImage, "TRE", "Main");

		// creates new instance of the class
		MonoObject* instance = mono_object_new(s_AppDomain, testClass);
        // Run constructor of the object class
    	mono_runtime_object_init(instance);

    }

    void ScriptEngine::TestAddComponent()
    {
        MonoImage* assemblyImage = mono_assembly_get_image(s_MonoAssembly);
        MonoClass* testClass = mono_class_from_name(assemblyImage, "TRE", "Main");

		MonoObject* Instance = mono_object_new(s_AppDomain, testClass);
		mono_runtime_object_init(Instance);

		MonoMethod* method = mono_class_get_method_from_name(testClass, "Test", 0);
		mono_runtime_invoke(method, Instance, nullptr, nullptr);
    }


#pragma endregion

#pragma region FuntionBindings

    MonoString* ScriptEngine::BindCreateEntity(MonoString* name)
    {
		char* nameString = mono_string_to_utf8(name);
        std::string str(nameString);
        mono_free(nameString);

		Entity Temp = ECSManager::Instance().CreateEntity(str);
        std::cout << "Created Entity from C#: " << str << std::endl;

		MonoString* GUID = mono_string_new(s_AppDomain, Temp->GetGUID().c_str());
        return GUID;
	}

    void ScriptEngine::BindAddComponent(MonoString* ID , int componenttype)
    {
		// Retrive the entity from the ID
		Entity Temp = ECSManager::Instance().FindEntity(mono_string_to_utf8(ID));

		// Use a switch case to determine which component to add
		switch (componenttype)
		{
		case 0: // Mesh
			Temp->AddComponent<MeshRenderer>();
            std::cout << "Mesh Renderer added by C#!" << std::endl;
			break;
		case 1: // Camera
			Temp->AddComponent<Camera>();
			std::cout << "Camera added by C#!" << std::endl;
			break;
		case 2: // Audio
			Temp->AddComponent<Audio>();
			std::cout << "Audio added by C#!" << std::endl;
            break;
		default:
            std::cout << "The component does not exist!" << std::endl;
            break;
        }
        
    }

	void ScriptEngine::BindRemoveComponent(MonoString* id, int componenttype)
	{
		Entity Temp = ECSManager::Instance().FindEntity(mono_string_to_utf8(id));

        switch (componenttype)
        {
        case 0: // mesh
			Temp->RemoveComponent<MeshRenderer>();
			std::cout << "Mesh Renderer removed by C#!" << std::endl;
			break;
        case 1:
			Temp->RemoveComponent<Camera>();
			std::cout << "Camera removed by C#!" << std::endl;
            break;
        case 2:
			Temp->RemoveComponent<Audio>();
			std::cout << "Audio removed by C#!" << std::endl;
            break;
        default:
			std::cout << "The component does not exist!" << std::endl;
            break;
        }
	}

#pragma endregion
}
