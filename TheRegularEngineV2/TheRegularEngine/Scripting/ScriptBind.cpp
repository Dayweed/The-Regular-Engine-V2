#include "pch.h"
#include "Scripting/ScriptBind.h"

#include"Scripting/ScriptEngine.h"
#include "Core/ECS.h"
#include "Demo/Demo.h"
#include "Core/Transform.h"
#include "Core/Logger.h"

#include "Audio/AudioSystem.h"
#include "Graphics/Camera.h"
#include "Graphics/MeshRenderer.h"
#include "EventSystem/EventHandler/EventHandler.h"

#include "mono/metadata/object.h"
#include "mono/metadata/reflection.h"

#include <map>

namespace TRE
{
    ScriptInputHandler& ScriptInputHandler::Instance()
	{
		static ScriptInputHandler instance;
		return instance;
	}

    void ScriptInputHandler::GetKeyPressed(const InputEvent& event)
    {
        _key= event._key;
        _state = event._state;
    }

	std::string MonoStringToString(MonoString* monoString)
	{
		char* temp = mono_string_to_utf8(monoString);
        std::string str(temp);
        mono_free(temp);
        return str;
	}

    static void BindEntityRename(MonoString* ID, MonoString* name)
    {
        // Retrive the entity from the ID
        Entity Temp = ECSManager::Instance().FindEntity(mono_string_to_utf8(ID));
        Temp->GetComponent<Properties>().m_Name;
    }

    static void BindEntityActive(MonoString* ID, MonoBoolean isActive)
    {
        // Retrive the entity from the ID
        Entity Temp = ECSManager::Instance().FindEntity(mono_string_to_utf8(ID));
        Temp->GetComponent<Properties>().m_Active = isActive;
    }

    static bool BindEntityGetActive(MonoString* ID)
    {
        // Retrive the entity from the ID
        Entity Temp = ECSManager::Instance().FindEntity(mono_string_to_utf8(ID));
        return Temp->GetComponent<Properties>().m_Active;
    }

    static void BindCreateEntity(MonoString* name, MonoString* output)
    {
        char* nameString = mono_string_to_utf8(name);
        std::string str(nameString);
        mono_free(nameString);

        Entity Temp = ECSManager::Instance().CreateEntity(str);
        std::cout << "Created Entity from C#: " << str << std::endl;
    }

     static void BindAddComponent(MonoString* ID, int componenttype)
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

    static void BindRemoveComponent(MonoString* id, int componenttype)
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

    static void BindDestroyEntity(MonoString* id)
    {
        std::string ID = MonoStringToString(id);
        // find the entity
        Entity Temp = ECSManager::Instance().FindEntity(ID);
        ECSManager::Instance().MarkForDeletion(Temp);
    }

    static void BindTestFunction()
    {
        Demo::SpawnObject();
    }

    static std::unordered_map<std::string, std::string> GetAllSceneObjects()
	{
        std::unordered_map<std::string, std::string > tempMap;
		std::vector temp = ECSManager::Instance().GetAllEntities();

        for(auto& entity : temp)
        {
            std::string temp1 = entity->GetName();
            std::string temp2 = entity->GetGUID();
            tempMap.insert(std::make_pair(temp1, temp2));
		}

		return tempMap;
	}

    static MonoString* FindIDFromName(MonoString* name)
	{
		std::string temp = MonoStringToString(name);
        std::unordered_map<std::string, std::string> sceneObjects = GetAllSceneObjects();
        // Search for the name in the map
        if(sceneObjects.find(temp) != sceneObjects.end())
        {
        	// Found the name
			std::string ID = sceneObjects[temp];
			return mono_string_new(mono_domain_get(), ID.c_str());
		}
		else
		{
			// Did not find the name
			return mono_string_new(mono_domain_get(), "NULL");
		}
	}

#pragma region TransformBindings

    static void BindSetPosition(MonoString* id, glm::vec3 newPos)
    {
        std::string ID = MonoStringToString(id);
        // find the entity
        Entity Temp = ECSManager::Instance().FindEntity(ID);
        Transform& transform = Temp->GetComponent<Transform>();
        transform.m_Position = newPos;
        transform.m_IsDirty = true;
    }

    static void BindSetRotation(MonoString* id, glm::vec3 newRot)
    {
        std::string ID = MonoStringToString(id);
        // find the entity 
        Entity Temp = ECSManager::Instance().FindEntity(ID);
        Transform& transform = Temp->GetComponent<Transform>();
        transform.m_Rotation = newRot;
        transform.m_IsDirty = true;
    }

    static void BindGetPosition(MonoString* id, glm::vec3* output)
    {
        std::string ID = MonoStringToString(id);
        // find the entity
        Entity Temp = ECSManager::Instance().FindEntity(ID);
        Transform& transform = Temp->GetComponent<Transform>();
        *output = transform.m_Position;
    }

    static void BindGetRotation(MonoString* id, glm::vec3* output)
    {
        std::string ID = MonoStringToString(id);
        // find the entity
        Entity Temp = ECSManager::Instance().FindEntity(ID);
        // Get the rotation
        *output = Temp->GetComponent<Transform>().m_Rotation;
    }

#pragma endregion

#pragma region CameraBindings
    static void BindCamSetViewportSize(MonoString* id, glm::vec2 newSize)
    {
        std::string ID = MonoStringToString(id);
        // find the entity
        Entity Temp = ECSManager::Instance().FindEntity(ID);
        ECSSystemManager::Instance().GetSystem<CameraSystem>()->SetViewportSize(Temp, newSize);
    }

    static void BindCamSetFocalPoint(MonoString* id, glm::vec3 focalpoint)
    {
        std::string ID = MonoStringToString(id);
        // find the entity
        Entity Temp = ECSManager::Instance().FindEntity(ID);
        ECSSystemManager::Instance().GetSystem<CameraSystem>()->SetFocalPoint(Temp, focalpoint);
    }

    static void BindCamSetFocalLength(MonoString* id, float focallength)
    {
        std::string ID = MonoStringToString(id);
        //find the entity
        Entity Temp = ECSManager::Instance().FindEntity(ID);
        ECSSystemManager::Instance().GetSystem<CameraSystem>()->SetFocalLength(Temp, focallength);
    }

    static void BindCamSetFOV(MonoString* id, float fov)
    {
        std::string ID = MonoStringToString(id);
        //find the entity
        Entity Temp = ECSManager::Instance().FindEntity(ID);
        ECSSystemManager::Instance().GetSystem<CameraSystem>()->SetFov(Temp, fov);
    }

    static void BindCamSetNear(MonoString* id, float n)
    {
        std::string ID = MonoStringToString(id);
        //find the entity
        Entity Temp = ECSManager::Instance().FindEntity(ID);
        ECSSystemManager::Instance().GetSystem<CameraSystem>()->SetNear(Temp, n);
    }

    static void BindCamSetFar(MonoString* id, float f)
    {
        std::string ID = MonoStringToString(id);
        //find the entity
        Entity Temp = ECSManager::Instance().FindEntity(ID);
        ECSSystemManager::Instance().GetSystem<CameraSystem>()->SetFar(Temp, f);
    }

    static void BindCamSetLeft(MonoString* id, float left)
    {
        std::string ID = MonoStringToString(id);
        //find the entity
        Entity Temp = ECSManager::Instance().FindEntity(ID);
        ECSSystemManager::Instance().GetSystem<CameraSystem>()->SetLeft(Temp, left);
    }

    static void BindCamSetRight(MonoString* id, float right)
    {
        std::string ID = MonoStringToString(id);
        //find the entity
        Entity Temp = ECSManager::Instance().FindEntity(ID);
        ECSSystemManager::Instance().GetSystem<CameraSystem>()->SetRight(Temp, right);
    }

    static void BindCamSetTop(MonoString* id, float top)
    {
        std::string ID = MonoStringToString(id);
        //find the entity
        Entity Temp = ECSManager::Instance().FindEntity(ID);
        ECSSystemManager::Instance().GetSystem<CameraSystem>()->SetTop(Temp, top);
    }

    static void BindCamSetBottom(MonoString* id, float bottom)
    {
        std::string ID = MonoStringToString(id);
        //find the entity
        Entity Temp = ECSManager::Instance().FindEntity(ID);
        ECSSystemManager::Instance().GetSystem<CameraSystem>()->SetBottom(Temp, bottom);
    }

    static void BindCamSetAspectRatio(MonoString* id, float aspectRatio)
    {
        std::string ID = MonoStringToString(id);
        //find the entity
        Entity Temp = ECSManager::Instance().FindEntity(ID);
        ECSSystemManager::Instance().GetSystem<CameraSystem>()->SetAspectRatio(Temp, aspectRatio);
    }

    static void BindCamSetIsPerspective(MonoString* id, bool isPerspective)
    {
        std::string ID = MonoStringToString(id);
        //find the entity
        Entity Temp = ECSManager::Instance().FindEntity(ID);
        ECSSystemManager::Instance().GetSystem<CameraSystem>()->SetIsPerspective(Temp, isPerspective);
    }

    static void BindCamSetIsMainCamera(MonoString* id, bool isMainCamera)
    {
        std::string ID = MonoStringToString(id);
        // find the entity
        Entity Temp = ECSManager::Instance().FindEntity(ID);
        ECSSystemManager::Instance().GetSystem<CameraSystem>()->SetIsMainCamera(Temp, isMainCamera);
    }

    // Getters
    static void BindCamGetViewMatrix(MonoString* id, glm::mat4* result)
    {
        std::string ID = MonoStringToString(id);
        //find the entity
        Entity Temp = ECSManager::Instance().FindEntity(ID);
        *result = ECSSystemManager::Instance().GetSystem<CameraSystem>()->GetViewMatrix(Temp);
    }

    static void BindCamGetProjectionMatrix(MonoString* id, glm::mat4* result)
    {
        std::string ID = MonoStringToString(id);
        //find the entity
        Entity Temp = ECSManager::Instance().FindEntity(ID);
        *result = ECSSystemManager::Instance().GetSystem<CameraSystem>()->GetProjectionMatrix(Temp);
    }

    static void BindCamGetInverseViewMatrix(MonoString* id, glm::mat4* result)
    {
        std::string ID = MonoStringToString(id);
        //find the entity
        Entity Temp = ECSManager::Instance().FindEntity(ID);
        *result = ECSSystemManager::Instance().GetSystem<CameraSystem>()->GetInverseViewMatrix(Temp);
    }

    static void BindCamGetInverseProjectionMatrix(MonoString* id, glm::mat4* result)
    {
        std::string ID = MonoStringToString(id);
        //find the entity
        Entity Temp = ECSManager::Instance().FindEntity(ID);
        *result = ECSSystemManager::Instance().GetSystem<CameraSystem>()->GetInverseProjectionMatrix(Temp);
    }

    static void BindCamGetInverseViewProjectionMatrix(MonoString* id, glm::mat4* result)
    {
        std::string ID = MonoStringToString(id);
        //find the entity
        Entity Temp = ECSManager::Instance().FindEntity(ID);
        *result = ECSSystemManager::Instance().GetSystem<CameraSystem>()->GetInverseViewProjectionMatrix(Temp);
    }

    static void BindCamGetViewportSize(MonoString* id, glm::vec2* result)
    {
        std::string ID = MonoStringToString(id);
        //find the entity
        Entity Temp = ECSManager::Instance().FindEntity(ID);
        *result = ECSSystemManager::Instance().GetSystem<CameraSystem>()->GetViewportSize(Temp);
    }

    static void BindCamGetFOV(MonoString* id, float* result)
    {
        std::string ID = MonoStringToString(id);
        //find the entity
        Entity Temp = ECSManager::Instance().FindEntity(ID);
        *result = ECSSystemManager::Instance().GetSystem<CameraSystem>()->GetFov(Temp);
    }

    static void BindCamGetNear(MonoString* id, float* result)
    {
        std::string ID = MonoStringToString(id);
        //find the entity
        Entity Temp = ECSManager::Instance().FindEntity(ID);
        *result = ECSSystemManager::Instance().GetSystem<CameraSystem>()->GetNear(Temp);
    }

    static void BindCamGetFar(MonoString* id, float* result)
    {
        std::string ID = MonoStringToString(id);
        //find the entity
        Entity Temp = ECSManager::Instance().FindEntity(ID);
        *result = ECSSystemManager::Instance().GetSystem<CameraSystem>()->GetFar(Temp);
    }

    static void BindCamGetLeft(MonoString* id, float* result)
    {
        std::string ID = MonoStringToString(id);
        //find the entity
        Entity Temp = ECSManager::Instance().FindEntity(ID);
        *result = ECSSystemManager::Instance().GetSystem<CameraSystem>()->GetLeft(Temp);
    }

    static void BindCamGetRight(MonoString* id, float* result)
    {
        std::string ID = MonoStringToString(id);
        //find the entity
        Entity Temp = ECSManager::Instance().FindEntity(ID);
        *result = ECSSystemManager::Instance().GetSystem<CameraSystem>()->GetRight(Temp);
    }

    static void BindCamGetTop(MonoString* id, float* result)
    {
        std::string ID = MonoStringToString(id);
        //find the entity
        Entity Temp = ECSManager::Instance().FindEntity(ID);
        *result = ECSSystemManager::Instance().GetSystem<CameraSystem>()->GetTop(Temp);
    }

    static void BindCamGetBottom(MonoString* id, float* result)
    {
        std::string ID = MonoStringToString(id);
        //find the entity
        Entity Temp = ECSManager::Instance().FindEntity(ID);
        *result = ECSSystemManager::Instance().GetSystem<CameraSystem>()->GetBottom(Temp);
    }

    static void BindCamGetAspectRatio(MonoString* id, float* result)
    {
        std::string ID = MonoStringToString(id);
        //find the entity
        Entity Temp = ECSManager::Instance().FindEntity(ID);
        *result = ECSSystemManager::Instance().GetSystem<CameraSystem>()->GetAspectRatio(Temp);
    }

    static void BindCamIsPerspective(MonoString* id, bool* result)
    {
        std::string ID = MonoStringToString(id);
        //find the entity
        Entity Temp = ECSManager::Instance().FindEntity(ID);
        *result = ECSSystemManager::Instance().GetSystem<CameraSystem>()->IsPerspective(Temp);
    }

    static void BindCamIsMainCamera(MonoString* id, bool* result)
    {
        std::string ID = MonoStringToString(id);
        //find the entity
        Entity Temp = ECSManager::Instance().FindEntity(ID);
        *result = ECSSystemManager::Instance().GetSystem<CameraSystem>()->IsMainCamera(Temp);
    }

#pragma endregion

#pragma region InputBindings

    static bool GetKeyDown(int key)
	{
		
        if(key == ScriptInputHandler::Instance().GetKey() )
        {
            ScriptInputHandler::Instance().ResetSystem();
	        return true;
        }
        else 
            return false;
         
	}
    

#pragma endregion

#pragma region Logging
    static void SendMessageToConsole(MonoString* message)
    {
        std::string str = MonoStringToString(message);

        TRE_INFO(str);
    }

    static void SendWarningToConsole(MonoString* message)
    {
        std::string str = MonoStringToString(message);
        TRE_WARN(str);
    }

    static void SendErrorToConsole(MonoString* message)
    {
        std::string str = MonoStringToString(message);
        TRE_ERROR(str);
    }

    static void SendCriticalToConsole(MonoString* message)
    {
        std::string str = MonoStringToString(message);
        TRE_CRITICAL(str);
    }

#pragma endregion

#pragma region Physics

    static void BindResizeSphereCollider(MonoString* id, float s)
    {
        std::string ID = MonoStringToString(id);
        // find the entity
        Entity Temp = ECSManager::Instance().FindEntity(ID);
        ECSSystemManager::Instance().GetSystem<PhysicsSystem>()->ResizeSphereCollider(Temp, s);
    }

    static void BindResizeBoxCollider(MonoString* id, glm::vec3 s)
    {
        std::string ID = MonoStringToString(id);
        // find the entity
        Entity Temp = ECSManager::Instance().FindEntity(ID);
        ECSSystemManager::Instance().GetSystem<PhysicsSystem>()->ResizeBoxCollider(Temp, s);
    }

    static void BindAddForce(MonoString* id, glm::vec3 force)
    {
        std::string ID = MonoStringToString(id);
        // find the entity
        Entity Temp = ECSManager::Instance().FindEntity(ID);
        ECSSystemManager::Instance().GetSystem<PhysicsSystem>()->AddForce(Temp, force);
    }

#pragma endregion

#pragma endregion

    void ScriptBind::RegisterFunctions()
    {
        

        // ECS Bindings
        mono_add_internal_call("TRE.ECSManager::CreateEntity", BindCreateEntity);
        mono_add_internal_call("TRE.ECSManager::AddComponent", BindAddComponent);
        mono_add_internal_call("TRE.ECSManager::RemoveComponent", BindRemoveComponent);
        mono_add_internal_call("TRE.Demo::SpawnObject", BindTestFunction);
        mono_add_internal_call("TRE.ECSManager::FindIDFromName", FindIDFromName);

        // Entity Bindings
        mono_add_internal_call("TRE.Entity::Rename", BindEntityRename);
        mono_add_internal_call("TRE.Entity::SetActive", BindEntityActive);
        mono_add_internal_call("TRE.Entity::GetActive", BindEntityGetActive);

        // Tranform Bindings
        mono_add_internal_call("TRE.TransformSystem::SetPosition", BindSetPosition);
        mono_add_internal_call("TRE.TransformSystem::SetRotation", BindSetRotation);
        mono_add_internal_call("TRE.TransformSystem::GetPosition", BindGetPosition);
        mono_add_internal_call("TRE.TransformSystem::GetRotation", BindGetRotation);

        // Camera Bindings
        mono_add_internal_call("TRE.CameraSystem::SetViewportSize", BindCamSetViewportSize);
        mono_add_internal_call("TRE.CameraSystem::SetFocalPoint", BindCamSetFocalPoint);
        mono_add_internal_call("TRE.CameraSystem::SetFocalLength", BindCamSetFocalLength);
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

        mono_add_internal_call("TRE.CameraSystem::GetViewMatrix", BindCamGetViewMatrix);
        mono_add_internal_call("TRE.CameraSystem::GetProjectionMatrix", BindCamGetProjectionMatrix);
        mono_add_internal_call("TRE.CameraSystem::GetInverseViewMatrix", BindCamGetInverseViewMatrix);
        mono_add_internal_call("TRE.CameraSystem::GetInverseProjectionMatrix", BindCamGetInverseProjectionMatrix);
        mono_add_internal_call("TRE.CameraSystem::GetInverseViewProjectionMatrix", BindCamGetInverseViewProjectionMatrix);
        mono_add_internal_call("TRE.CameraSystem::GetViewportSize", BindCamGetViewportSize);
        mono_add_internal_call("TRE.CameraSystem::GetFOV", BindCamGetFOV);
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
        mono_add_internal_call("TRE.PhysicsSystem::ResizeSphereCollider", BindResizeSphereCollider);
        mono_add_internal_call("TRE.PhysicsSystem::ResizeBoxCollider", BindResizeBoxCollider);
        mono_add_internal_call("TRE.PhysicsSystem::AddForce", BindAddForce);

        // Input Binding
        mono_add_internal_call("TRE.InputSystem::GetKeyDown", GetKeyDown);

        // Logging
        mono_add_internal_call("TRE.Core::Log", SendMessageToConsole);
        mono_add_internal_call("TRE.Core::LogWarning", SendWarningToConsole);
        mono_add_internal_call("TRE.Core::LogError", SendErrorToConsole);
        mono_add_internal_call("TRE.Core::LogCritical", SendCriticalToConsole);

    }
}