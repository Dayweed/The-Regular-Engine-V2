#include "pch.h"
#include "ScriptEngine.h"

//Everything should be remove. This is just to test the calling of the runtime works.
#include <fstream>

#include "ScriptBind.h"

namespace TRE
{

    struct ScriptEngineData
    {
    	MonoDomain* RootDomain = nullptr;

		MonoDomain* AppDomain= nullptr;
		MonoAssembly* MonoAssembly= nullptr;
        MonoImage* AssemblyImage = nullptr;

        MonoObject* DemoInstance = nullptr;

		ScriptInputHandler* ScriptInputHandler = nullptr;
		
	};

#pragma region ScriptEngine
    static ScriptEngineData* s_ScriptEngineData = nullptr;
    std::string ScriptEngine::TestGUID = "";
    bool ScriptEngine::CreatedScriptObject = false;

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

    void ScriptEngine::Init()
    {
    	s_ScriptEngineData = new ScriptEngineData();
		
		InitMono();
        ScriptBind::RegisterFunctions();
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
        s_ScriptEngineData-> RootDomain = rootDomain;

        s_ScriptEngineData->AppDomain = mono_domain_create_appdomain(const_cast<char*>("TREScriptRuntime"), nullptr);
        mono_domain_set(s_ScriptEngineData->AppDomain, true);

        s_ScriptEngineData->MonoAssembly = LoadCSharpAssembly("../Resources/Scripts/TRE-ScriptCore.dll");
        PrintAssemblyTypes(s_ScriptEngineData->MonoAssembly);

        // Get the image of the assembly
        s_ScriptEngineData->AssemblyImage = mono_assembly_get_image(s_ScriptEngineData->MonoAssembly);

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
		MonoObject* instance = mono_object_new(s_ScriptEngineData->AppDomain, testClass);
        // Run constructor of the object class
    	mono_runtime_object_init(instance);

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

    void ScriptEngine::TestSpawnObject()
    {
		/*MonoImage* assemblyImage = mono_assembly_get_image(s_ScriptEngineData->MonoAssembly);
		MonoClass* testClass = mono_class_from_name(assemblyImage, "TRE", "Demo");
        MonoObject* instance = mono_object_new(s_ScriptEngineData->AppDomain, testClass);
        mono_runtime_object_init(instance);*/
    }


    void ScriptEngine::DemoInit()
    {
	    MonoClass* testClass = mono_class_from_name(s_ScriptEngineData->AssemblyImage, "TRE", "Demo");
        s_ScriptEngineData->DemoInstance = mono_object_new(s_ScriptEngineData->AppDomain, testClass);
        mono_runtime_object_init(s_ScriptEngineData->DemoInstance);
    }

    void ScriptEngine::DemoUpdate()
    {
    	MonoClass* testClass = mono_class_from_name(s_ScriptEngineData->AssemblyImage, "TRE", "Demo");
		MonoMethod* method = mono_class_get_method_from_name(testClass, "Update", 0);
		mono_runtime_invoke(method, s_ScriptEngineData->DemoInstance, nullptr, nullptr);
	}




#pragma endregion

#pragma region FuntionBindings

    
}
