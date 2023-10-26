#include "pch.h"
#include "ScriptEngine.h"

//Everything should be remove. This is just to test the calling of the runtime works.
#include <fstream>

#include "ScriptBind.h"
#include "Core/Logger.h"

namespace TRE
{

#pragma region ScriptEngine

    ScriptEngineData* ScriptEngine::s_ScriptEngineData = nullptr;
    std::string ScriptEngine::TestGUID = "";
    bool ScriptEngine::CreatedScriptObject = false;

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
			return;
		}

		// Here we will load the project assembly when the project script and core script is separated.

		// Load all the classes from the assembly

		// Register all ECS components to the scripting engine


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

	void ScriptEngine::ReloadAssembly()
	{
		// Unload the assembly
		mono_domain_set(mono_get_root_domain(), false);

		mono_domain_unload(s_ScriptEngineData->AppDomain);

		// Load the assembly again
		LoadAssembly(s_ScriptEngineData->MonoAssemblyPath);

		// add loading application assembly when project script and core script is separated.
		// add loading all the classes from the assembly

		// Register back all the components to the scripting engine

		// Retrieve and instantiate the main class

	}

	

    void ScriptEngine::UpdateScriptingEngine()
    {
        MonoImage* assemblyImage = mono_assembly_get_image(s_ScriptEngineData->MonoAssembly);
        MonoClass* testClass = mono_class_from_name(assemblyImage, "TRE", "Main");

        MonoObject * instance = mono_object_new(s_ScriptEngineData->AppDomain, testClass);
        MonoMethod* method = mono_class_get_method_from_name(testClass, "Update", 0);
        mono_runtime_invoke(method, instance, nullptr, nullptr);
    }

    void ScriptEngine::TestScriptingEngine()
    {
        MonoImage* assemblyImage = mono_assembly_get_image(s_ScriptEngineData->MonoAssembly);
        MonoClass* testClass = mono_class_from_name(assemblyImage, "TRE", "Main");

		// creates new instance of the class
		s_ScriptEngineData->DemoObject = mono_object_new(s_ScriptEngineData->AppDomain, testClass);
        // Run constructor of the object class
    	mono_runtime_object_init(s_ScriptEngineData->DemoObject);

    }

    void ScriptEngine::TestAddComponent()
    {
        MonoImage* assemblyImage = mono_assembly_get_image(s_ScriptEngineData->MonoAssembly);
        MonoClass* testClass = mono_class_from_name(assemblyImage, "TRE", "Main");

		MonoObject* Instance = mono_object_new(s_ScriptEngineData->AppDomain, testClass);
		mono_runtime_object_init(Instance);

		MonoMethod* method = mono_class_get_method_from_name(testClass, "Test", 0);
		mono_runtime_invoke(method, Instance, nullptr, nullptr);

    }

    void ScriptEngine::TestUpdateObject()
    {
    	MonoImage* assemblyImage = mono_assembly_get_image(s_ScriptEngineData->MonoAssembly);
		MonoClass* testClass = mono_class_from_name(assemblyImage, "TRE", "Main");
        MonoMethod* method = mono_class_get_method_from_name(testClass, "Update", 0);
        mono_runtime_invoke(method, s_ScriptEngineData->DemoObject, nullptr, nullptr);
    }

#pragma endregion

    
}
