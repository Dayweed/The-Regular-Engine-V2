#include "pch.h"
#include "ScriptEngine.h"
#include "EventSystem/EventHandler/EventHandler.h"

#include "Core/ECS.h"
#include "Demo/Demo.h"
#include "Core/Transform.h"


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
		mono_add_internal_call("TRE.ECSManager::AddComponent", BindAddComponent);
		mono_add_internal_call("TRE.ECSManager::RemoveComponent", BindRemoveComponent);
        mono_add_internal_call("TRE.Demo::SpawnObject", BindTestFunction);
        mono_add_internal_call("TRE.Main::GetTestGUID", BindGetTestGUID);

        // Tranform Bindings
        mono_add_internal_call("TRE.TransformSystem::SetPosition", BindSetPosition);
        mono_add_internal_call("TRE.TransformSystem::SetRotation", BindSetRotation);
        mono_add_internal_call("TRE.TransformSystem::GetPosition", BindGetPosition);
        mono_add_internal_call("TRE.TransformSystem::GetRotation", BindGetRotation);

        // Camera Bindings
        mono_add_internal_call("TRE.CameraSystem::SetPosition", BindCamSetPosition);
        mono_add_internal_call("TRE.CameraSystem::SetRotation", BindCamSetRotation);
        mono_add_internal_call("TRE.CameraSystem::SetViewportSize", BindCamSetViewportSize);
        mono_add_internal_call("TRE.CameraSystem::SetFocalPoint", BindCamSetFocalPoint);
        mono_add_internal_call("TRE.CameraSystem::SetFocalLength", BindCamSetFocalLength);
        mono_add_internal_call("TRE.CameraSystem::SetPitch", BindCamSetPitch);
        mono_add_internal_call("TRE.CameraSystem::SetYaw", BindCamSetYaw);
        mono_add_internal_call("TRE.CameraSystem::SetRoll", BindCamSetRoll);
        mono_add_internal_call("TRE.CameraSystem::SetFOV", BindCamSetFOV);
        mono_add_internal_call("TRE.CameraSystem::SetNear", BindCamSetNear);
        mono_add_internal_call("TRE.CameraSystem::SetFar", BindCamSetFar);
        mono_add_internal_call("TRE.CameraSystem::SetLeft", BindCamSetLeft);
        mono_add_internal_call("TRE.CameraSystem::SetRight", BindCamSetRight);
        mono_add_internal_call("TRE.CameraSystem::SetBottom", BindCamSetBottom);
        mono_add_internal_call("TRE.CameraSystem::SetTop", BindCamSetTop);
        mono_add_internal_call("TRE.CameraSystem::SetAspectRatio", BindCamSetAspectRatio);
        mono_add_internal_call("TRE.CameraSystem::SetIsPerspective", BindCamSetIsPerspective);
        mono_add_internal_call("TRE.CameraSystem::SetIsMainCamera", BindCamSetIsMainCamera);

        mono_add_internal_call("TRE.CameraSystem::GetPosition", BindCamGetPosition);
        mono_add_internal_call("TRE.CameraSystem::GetRotation", BindCamGetRotation);
        mono_add_internal_call("TRE.CameraSystem::GetViewMatrix", BindCamGetViewMatrix);
        mono_add_internal_call("TRE.CameraSystem::GetProjectionMatrix", BindCamGetProjectionMatrix);
        mono_add_internal_call("TRE.CameraSystem::GetInverseViewMatrix", BindCamGetInverseViewMatrix);
        mono_add_internal_call("TRE.CameraSystem::GetInverseProjectionMatrix", BindCamGetInverseProjectionMatrix);
        mono_add_internal_call("TRE.CameraSystem::GetInverseViewProjectionMatrix", BindCamGetInverseViewProjectionMatrix);
        mono_add_internal_call("TRE.CameraSystem::GetViewportSize", BindCamGetViewportSize);
        mono_add_internal_call("TRE.CameraSystem::GetFocalPoint", BindCamGetFocalPoint);
        mono_add_internal_call("TRE.CameraSystem::GetFocalLength", BindCamGetFocalLength);
        mono_add_internal_call("TRE.CameraSystem::GetPitch", BindCamGetPitch);
        mono_add_internal_call("TRE.CameraSystem::GetYaw", BindCamGetYaw);
        mono_add_internal_call("TRE.CameraSystem::GetRoll", BindCamGetRoll);
        mono_add_internal_call("TRE.CameraSystem::GetFov", BindCamGetFOV);
        mono_add_internal_call("TRE.CameraSystem::GetNear", BindCamGetNear);
        mono_add_internal_call("TRE.CameraSystem::GetFar", BindCamGetFar);
        mono_add_internal_call("TRE.CameraSystem::GetLeft", BindCamGetLeft);
        mono_add_internal_call("TRE.CameraSystem::GetRight", BindCamGetRight);
        mono_add_internal_call("TRE.CameraSystem::GetBottom", BindCamGetBottom);
        mono_add_internal_call("TRE.CameraSystem::GetTop", BindCamGetTop);
        mono_add_internal_call("TRE.CameraSystem::GetAspectRatio", BindCamGetAspectRatio);
        mono_add_internal_call("TRE.CameraSystem::IsPerspective", BindCamIsPerspective);
        mono_add_internal_call("TRE.CameraSystem::IsMainCamera", BindCamIsMainCamera);

        // Physics Bindings
        mono_add_internal_call("TRE.PhysicsSystem::ConstructSphereCollider", BindConstructSphereCollider);
        mono_add_internal_call("TRE.PhysicsSystem::ResizeSphereCollider", BindResizeSphereCollider);
        mono_add_internal_call("TRE.PhysicsSystem::ConstructBoxCollider", BindConstructBoxCollider);
        mono_add_internal_call("TRE.PhysicsSystem::ResizeBoxCollider", BindResizeBoxCollider);

    }

    void ScriptEngine::UpdateScriptingEngine()
    {
        MonoImage* assemblyImage = mono_assembly_get_image(s_MonoAssembly);
        MonoClass* testClass = mono_class_from_name(assemblyImage, "TRE", "Main");

        MonoObject * instance = mono_object_new(s_AppDomain, testClass);
        MonoMethod* method = mono_class_get_method_from_name(testClass, "Update", 0);
        mono_runtime_invoke(method, instance, nullptr, nullptr);
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

    void ScriptEngine::TestSpawnObject()
    {
		MonoImage* assemblyImage = mono_assembly_get_image(s_MonoAssembly);
		MonoClass* testClass = mono_class_from_name(assemblyImage, "TRE", "Demo");
        MonoObject* instance = mono_object_new(s_AppDomain, testClass);
        mono_runtime_object_init(instance);
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

    void ScriptEngine::BindTestFunction()
    {
        Demo::SpawnObject();
	}

    void ScriptEngine::BindSetPosition(MonoString* id, glm::vec3 newPos)
    {
        std::string ID = mono_string_to_utf8(id);
        // find the entity
        Entity Temp = ECSManager::Instance().FindEntity(ID);
        Transform& transform = Temp->GetComponent<Transform>();
        transform.m_Position = newPos;
        transform.m_IsDirty = true;
    }

    void ScriptEngine::BindSetRotation(MonoString* id, glm::vec3 newRot)
    {
        std::string ID = mono_string_to_utf8(id);
        // find the entity 
        Entity Temp = ECSManager::Instance().FindEntity(ID);
        Transform& transform = Temp->GetComponent<Transform>();
        transform.m_Rotation = newRot;
        transform.m_IsDirty = true;
	}

    void ScriptEngine::BindGetPosition(MonoString* id, glm::vec3* output)
    {
        std::string ID = mono_string_to_utf8(id);
        // find the entity
        Entity Temp = ECSManager::Instance().FindEntity(ID);
        Transform& transform = Temp->GetComponent<Transform>();
        *output = transform.m_Position;
    }

    void ScriptEngine::BindGetRotation(MonoString* id, glm::vec3* output)
    {
        std::string ID = mono_string_to_utf8(id);
        // find the entity
        Entity Temp = ECSManager::Instance().FindEntity(ID);    
        // Get the rotation
        *output = Temp->GetComponent<Transform>().m_Rotation;
	}

    MonoString* ScriptEngine::BindGetTestGUID()
    {
		MonoString* GUID = mono_string_new(s_AppDomain, TestGUID.c_str());
		return GUID;
	}


#pragma region CameraBindings
    void ScriptEngine::BindCamSetPosition(MonoString* id, glm::vec3 newPos)
    {
        std::string ID = mono_string_to_utf8(id);
        // find the entity
        Entity Temp = ECSManager::Instance().FindEntity(ID);
        ECSSystemManager::Instance().GetSystem<CameraSystem>()->SetPosition(Temp, newPos);
        
    }

    void ScriptEngine::BindCamSetRotation(MonoString* id, glm::vec3 newRot)
    {
    	std::string ID = mono_string_to_utf8(id);
		// find the entity
		Entity Temp = ECSManager::Instance().FindEntity(ID);
		ECSSystemManager::Instance().GetSystem<CameraSystem>()->SetRotation(Temp, newRot);
	}

    void ScriptEngine::BindCamSetViewportSize(MonoString* id, glm::vec2 newSize)
    {
    	std::string ID = mono_string_to_utf8(id);
        // find the entity
        Entity Temp = ECSManager::Instance().FindEntity(ID);
        ECSSystemManager::Instance().GetSystem<CameraSystem>()->SetViewportSize(Temp, newSize);
    }

    void ScriptEngine::BindCamSetFocalPoint(MonoString* id, glm::vec3 focalpoint)
    {
		std::string ID = mono_string_to_utf8(id);
		// find the entity
		Entity Temp = ECSManager::Instance().FindEntity(ID);
		ECSSystemManager::Instance().GetSystem<CameraSystem>()->SetFocalPoint(Temp, focalpoint);
	}

    void ScriptEngine::BindCamSetFocalLength(MonoString* id, float focallength)
    {
    	std::string ID = mono_string_to_utf8(id);
        //find the entity
        Entity Temp = ECSManager::Instance().FindEntity(ID);
        ECSSystemManager::Instance().GetSystem<CameraSystem>()->SetFocalLength(Temp, focallength);
    }

    void ScriptEngine::BindCamSetPitch(MonoString* id, float pitch)
    {
    	std::string ID = mono_string_to_utf8(id);
		//find the entity
		Entity Temp = ECSManager::Instance().FindEntity(ID);
		ECSSystemManager::Instance().GetSystem<CameraSystem>()->SetPitch(Temp, pitch);
	}

    void ScriptEngine::BindCamSetYaw(MonoString* id, float yaw)
    {
    	std::string ID = mono_string_to_utf8(id);
        //find the entity
        Entity Temp = ECSManager::Instance().FindEntity(ID);
        ECSSystemManager::Instance().GetSystem<CameraSystem>()->SetYaw(Temp, yaw);
    }

    void ScriptEngine::BindCamSetRoll(MonoString* id, float roll)
    {
    	std::string ID = mono_string_to_utf8(id);
		//find the entity
		Entity Temp = ECSManager::Instance().FindEntity(ID);
		ECSSystemManager::Instance().GetSystem<CameraSystem>()->SetRoll(Temp, roll);
    }

    void ScriptEngine::BindCamSetFOV(MonoString* id, float fov)
    {
	    std::string ID = mono_string_to_utf8(id);
		//find the entity
		Entity Temp = ECSManager::Instance().FindEntity(ID);
		ECSSystemManager::Instance().GetSystem<CameraSystem>()->SetFov(Temp, fov);
    }

	void ScriptEngine::BindCamSetNear(MonoString* id, float n)
    {
    	std::string ID = mono_string_to_utf8(id);
		//find the entity
		Entity Temp = ECSManager::Instance().FindEntity(ID);
		ECSSystemManager::Instance().GetSystem<CameraSystem>()->SetNear(Temp, n);
	}

    void ScriptEngine::BindCamSetFar(MonoString* id, float f)
    {
    	std::string ID = mono_string_to_utf8(id);
        //find the entity
        Entity Temp = ECSManager::Instance().FindEntity(ID);
        ECSSystemManager::Instance().GetSystem<CameraSystem>()->SetFar(Temp, f);
    }

    void ScriptEngine::BindCamSetLeft(MonoString* id, float left)
    {
    	std::string ID = mono_string_to_utf8(id);
        //find the entity
        Entity Temp = ECSManager::Instance().FindEntity(ID);
        ECSSystemManager::Instance().GetSystem<CameraSystem>()->SetLeft(Temp, left);
    }

    void ScriptEngine::BindCamSetRight(MonoString* id, float right)
    {
    	std::string ID = mono_string_to_utf8(id);
        //find the entity
        Entity Temp = ECSManager::Instance().FindEntity(ID);
        ECSSystemManager::Instance().GetSystem<CameraSystem>()->SetRight(Temp, right);
    }

    void ScriptEngine::BindCamSetTop(MonoString* id, float top)
    {
    	std::string ID = mono_string_to_utf8(id);
        //find the entity
        Entity Temp = ECSManager::Instance().FindEntity(ID);
        ECSSystemManager::Instance().GetSystem<CameraSystem>()->SetTop(Temp, top);
    }

	void ScriptEngine::BindCamSetBottom(MonoString* id, float bottom)
    {
    	std::string ID = mono_string_to_utf8(id);
		//find the entity
		Entity Temp = ECSManager::Instance().FindEntity(ID);
		ECSSystemManager::Instance().GetSystem<CameraSystem>()->SetBottom(Temp, bottom);
	}

    void ScriptEngine::BindCamSetAspectRatio(MonoString* id, float aspectRatio)
    {
    	std::string ID = mono_string_to_utf8(id);
    	//find the entity
		Entity Temp = ECSManager::Instance().FindEntity(ID);
		ECSSystemManager::Instance().GetSystem<CameraSystem>()->SetAspectRatio(Temp, aspectRatio);
    }

    void ScriptEngine::BindCamSetIsPerspective(MonoString* id, bool isPerspective)
    {
    	std::string ID = mono_string_to_utf8(id);
        //find the entity
        Entity Temp = ECSManager::Instance().FindEntity(ID);
        ECSSystemManager::Instance().GetSystem<CameraSystem>()->SetIsPerspective(Temp, isPerspective);
    }

    void ScriptEngine::BindCamSetIsMainCamera(MonoString* id, bool isMainCamera)
    {
    	std::string ID = mono_string_to_utf8(id);
        // find the entity
        Entity Temp = ECSManager::Instance().FindEntity(ID);
        ECSSystemManager::Instance().GetSystem<CameraSystem>()->SetIsMainCamera(Temp, isMainCamera);
    }

    // Getters

    void ScriptEngine::BindCamGetPosition(MonoString* id, glm::vec3* result)
    {
    	std::string ID = mono_string_to_utf8(id);
		// find the entity
        Entity Temp = ECSManager::Instance().FindEntity(ID);
		*result = ECSSystemManager::Instance().GetSystem<CameraSystem>()->GetPosition(Temp);
	}

    void ScriptEngine::BindCamGetRotation(MonoString* id, glm::vec3* result)
    {
    	std::string ID = mono_string_to_utf8(id);
        // find the entity
        Entity Temp = ECSManager::Instance().FindEntity(ID);
        *result = ECSSystemManager::Instance().GetSystem<CameraSystem>()->GetRotation(Temp);
    }

    void ScriptEngine::BindCamGetViewMatrix(MonoString* id, glm::mat4* result)
    {
    	std::string ID = mono_string_to_utf8(id);
        //find the entity
        Entity Temp = ECSManager::Instance().FindEntity(ID);
        *result = ECSSystemManager::Instance().GetSystem<CameraSystem>()->GetViewMatrix(Temp);
    }

    void ScriptEngine::BindCamGetProjectionMatrix(MonoString* id, glm::mat4* result)
    {
    	std::string ID = mono_string_to_utf8(id);
		//find the entity
		Entity Temp = ECSManager::Instance().FindEntity(ID);
		*result = ECSSystemManager::Instance().GetSystem<CameraSystem>()->GetProjectionMatrix(Temp);
	}

    void ScriptEngine::BindCamGetInverseViewMatrix(MonoString* id, glm::mat4* result)
    {
    	std::string ID = mono_string_to_utf8(id);
        //find the entity
        Entity Temp = ECSManager::Instance().FindEntity(ID);
        *result = ECSSystemManager::Instance().GetSystem<CameraSystem>()->GetInverseViewMatrix(Temp);
    }

    void ScriptEngine::BindCamGetInverseProjectionMatrix(MonoString* id, glm::mat4* result)
    {
        std::string ID = mono_string_to_utf8(id);
        //find the entity
        Entity Temp = ECSManager::Instance().FindEntity(ID);
        *result = ECSSystemManager::Instance().GetSystem<CameraSystem>()->GetInverseProjectionMatrix(Temp);
    }

    void ScriptEngine::BindCamGetInverseViewProjectionMatrix(MonoString* id, glm::mat4* result)
    {
    	std::string ID = mono_string_to_utf8(id);
		//find the entity
		Entity Temp = ECSManager::Instance().FindEntity(ID);
		*result = ECSSystemManager::Instance().GetSystem<CameraSystem>()->GetInverseViewProjectionMatrix(Temp);
	}

    void ScriptEngine::BindCamGetViewportSize(MonoString* id, glm::vec2* result)
    {
    	std::string ID = mono_string_to_utf8(id);
		//find the entity
		Entity Temp = ECSManager::Instance().FindEntity(ID);
		*result = ECSSystemManager::Instance().GetSystem<CameraSystem>()->GetViewportSize(Temp);
	}

    void ScriptEngine::BindCamGetFocalPoint(MonoString* id, glm::vec3* result)
    {
    	std::string ID = mono_string_to_utf8(id);
		//find the entity
		Entity Temp = ECSManager::Instance().FindEntity(ID);
		*result = ECSSystemManager::Instance().GetSystem<CameraSystem>()->GetFocalPoint(Temp);
	}

    void ScriptEngine::BindCamGetFocalLength(MonoString* id, float* result)
    {
    	std::string ID = mono_string_to_utf8(id);
		//find the entity
		Entity Temp = ECSManager::Instance().FindEntity(ID);
		*result = ECSSystemManager::Instance().GetSystem<CameraSystem>()->GetFocalLength(Temp);
	}

    void ScriptEngine::BindCamGetPitch(MonoString* id, float* result)
    {
    	std::string ID = mono_string_to_utf8(id);
		//find the entity
		Entity Temp = ECSManager::Instance().FindEntity(ID);
		*result = ECSSystemManager::Instance().GetSystem<CameraSystem>()->GetPitch(Temp);
	}

    void ScriptEngine::BindCamGetYaw(MonoString* id, float* result)
    {
    	std::string ID = mono_string_to_utf8(id);
		//find the entity
		Entity Temp = ECSManager::Instance().FindEntity(ID);
		*result = ECSSystemManager::Instance().GetSystem<CameraSystem>()->GetYaw(Temp);
	}

    void ScriptEngine::BindCamGetRoll(MonoString* id, float* result)
    {
    	std::string ID = mono_string_to_utf8(id);
		//find the entity
        Entity Temp = ECSManager::Instance().FindEntity(ID);
        *result = ECSSystemManager::Instance().GetSystem<CameraSystem>()->GetRoll(Temp);
    }

    void ScriptEngine::BindCamGetFOV(MonoString* id, float* result)
    {
    	std::string ID = mono_string_to_utf8(id);
        //find the entity
        Entity Temp = ECSManager::Instance().FindEntity(ID);
        *result = ECSSystemManager::Instance().GetSystem<CameraSystem>()->GetFov(Temp);
    }

    void ScriptEngine::BindCamGetNear(MonoString* id, float* result)
    {
    	std::string ID = mono_string_to_utf8(id);
		//find the entity
		Entity Temp = ECSManager::Instance().FindEntity(ID);
		*result = ECSSystemManager::Instance().GetSystem<CameraSystem>()->GetNear(Temp);
	}

    void ScriptEngine::BindCamGetFar(MonoString* id, float* result)
    {
    	std::string ID = mono_string_to_utf8(id);
		//find the entity
		Entity Temp = ECSManager::Instance().FindEntity(ID);
		*result = ECSSystemManager::Instance().GetSystem<CameraSystem>()->GetFar(Temp);
    }

    void ScriptEngine::BindCamGetLeft(MonoString* id, float* result)
    {
    	std::string ID = mono_string_to_utf8(id);
		//find the entity
		Entity Temp = ECSManager::Instance().FindEntity(ID);
		*result = ECSSystemManager::Instance().GetSystem<CameraSystem>()->GetLeft(Temp);
	}

    void ScriptEngine::BindCamGetRight(MonoString* id, float* result)
    {
    	std::string ID = mono_string_to_utf8(id);
		//find the entity
		Entity Temp = ECSManager::Instance().FindEntity(ID);
		*result = ECSSystemManager::Instance().GetSystem<CameraSystem>()->GetRight(Temp);
	}

    void ScriptEngine ::BindCamGetTop(MonoString* id, float* result)
    {
    	std::string ID = mono_string_to_utf8(id);
		//find the entity
		Entity Temp = ECSManager::Instance().FindEntity(ID);
		*result = ECSSystemManager::Instance().GetSystem<CameraSystem>()->GetTop(Temp);
	}

	void ScriptEngine::BindCamGetBottom(MonoString* id, float* result)
    {
    	std::string ID = mono_string_to_utf8(id);
		//find the entity
		Entity Temp = ECSManager::Instance().FindEntity(ID);
		*result = ECSSystemManager::Instance().GetSystem<CameraSystem>()->GetBottom(Temp);
	}

	void ScriptEngine::BindCamGetAspectRatio(MonoString* id, float* result)
    {
    	std::string ID = mono_string_to_utf8(id);
		//find the entity
		Entity Temp = ECSManager::Instance().FindEntity(ID);
		*result = ECSSystemManager::Instance().GetSystem<CameraSystem>()->GetAspectRatio(Temp);
	}

	void ScriptEngine::BindCamIsPerspective(MonoString* id, bool* result)
    {
    	std::string ID = mono_string_to_utf8(id);
		//find the entity
		Entity Temp = ECSManager::Instance().FindEntity(ID);
		*result = ECSSystemManager::Instance().GetSystem<CameraSystem>()->IsPerspective(Temp);
	}

	void ScriptEngine::BindCamIsMainCamera(MonoString* id, bool* result)
    {
    	std::string ID = mono_string_to_utf8(id);
		//find the entity
		Entity Temp = ECSManager::Instance().FindEntity(ID);
		*result = ECSSystemManager::Instance().GetSystem<CameraSystem>()->IsMainCamera(Temp);
	}

#pragma endregion

#pragma region InputBindings

    ScriptInputHandler::ScriptInputHandler()
    {
        EventHandler::getEventHandlerInstance().subscribe(this, &ScriptInputHandler::GetKeyPressed);
	}

    void ScriptInputHandler::GetKeyPressed(const InputEvent& event)
    {
        _key = event._key;
        _state = event._state;
    }

    bool ScriptEngine::BindGetKeyPressed(int key)
    {
        if(m_ScriptInputHandler.GetKey() == key )
        {
	        return true;
        }
    }

#pragma endregion

#pragma region Logging
	void ScriptEngine::SendMessageToConsole(MonoString* message)
    {
    	char* messageString = mono_string_to_utf8(message);
		std::string str(messageString);
		mono_free(messageString);

		std::cout << str << std::endl;
	}

#pragma endregion

#pragma region Physics

    void ScriptEngine::BindResizeSphereCollider(MonoString* id, float s)
    {
    	std::string ID = mono_string_to_utf8(id);
		// find the entity
		Entity Temp = ECSManager::Instance().FindEntity(ID);
		ECSSystemManager::Instance().GetSystem<PhysicsSystem>()->ResizeSphereCollider(Temp, s);
	}

    void ScriptEngine::BindResizeBoxCollider(MonoString* id, glm::vec3 s)
    {
    	std::string ID = mono_string_to_utf8(id);
		// find the entity
		Entity Temp = ECSManager::Instance().FindEntity(ID);
		ECSSystemManager::Instance().GetSystem<PhysicsSystem>()->ResizeBoxCollider(Temp, s);
    }

    void ScriptEngine::BindAddForce(MonoString* id , glm::vec3 force)
    {
    	std::string ID = mono_string_to_utf8(id);
		// find the entity
		Entity Temp = ECSManager::Instance().FindEntity(ID);
		ECSSystemManager::Instance().GetSystem<PhysicsSystem>()->AddForce(Temp, force);
    }

#pragma endregion

#pragma endregion
}
