#include "pch.h"
#include "Scripting/ScriptBind.h"

#include"Scripting/ScriptEngine.h"
#include "Core/ECS.h"
#include "Demo/Demo.h"
#include "Core/Transform.h"

#include "Audio/AudioSystem.h"
#include "Graphics/Camera.h"
#include "Graphics/MeshRenderer.h"
#include "EventSystem/EventHandler/EventHandler.h"
#include "EventSystem/Events/EditorEvent.h"

#include "mono/metadata/object.h"
#include "mono/metadata/reflection.h"

#define VALIDATEENTITY(ID) ValidateEntityID(ID, __FUNCTION__)

#define PUBLISHERROR(msg) PublishError(msg, __FUNCTION__)
#define PUBLISHWARN(msg) PublishWarning(msg, __FUNCTION__)
#define PUBLISHLOG(msg) PublishLog(msg, __FUNCTION__)

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

    static std::string EntityID_CSToEngine(CSEntityID ID)
    {
        return std::to_string(ID);
    }

    static CSEntityID EntityID_EngineToCS(std::string ID)
    {
        return std::stoull(ID);
    }

    static Entity ValidateEntityID(CSEntityID ID, std::string function)
    {
        // Retrive the entity from the ID
        Entity Temp = ECSManager::Instance().FindEntity(EntityID_CSToEngine(ID));
        if (Temp == nullptr)
        {
            std::string str{ CONSOLE_DEBUG_ERROR };
            str += "[" + function + "] Entity ID (" + EntityID_CSToEngine(ID) + ") does not exist in ECS Entities!";
            EventHandler::getEventHandlerInstance().Publish(ConsoleDebugEvent{ str.c_str() });
            return nullptr;
        }
        return Temp;
    }

    static void PublishError(std::string message, std::string function)
    {
        std::string str{ CONSOLE_DEBUG_ERROR };
        str += "[" + function + "] " + message;
        EventHandler::getEventHandlerInstance().Publish(ConsoleDebugEvent{ str.c_str() });
    }

    static void PublishWarning(std::string message, std::string function)
    {
        std::string str{ CONSOLE_DEBUG_WARN };
        str += "[" + function + "] " + message;
        EventHandler::getEventHandlerInstance().Publish(ConsoleDebugEvent{ str.c_str() });
    }

    static void PublishLog(std::string message, std::string function)
    {
        std::string str{ "[" + function + "] " + message };
        EventHandler::getEventHandlerInstance().Publish(ConsoleDebugEvent{ str.c_str() });
    }

	std::string MonoStringToString(MonoString* monoString)
	{
		char* temp = mono_string_to_utf8(monoString);
        std::string str(temp);
        mono_free(temp);
        return str;
	}

#pragma region PropertyBindings
    static void BindEntityRename(CSEntityID ID, MonoString* name)
    {
        // Retrive the entity from the ID
        Entity Temp = VALIDATEENTITY(ID);
        if (!Temp) return;
        Temp->GetComponent<Properties>().m_Name = mono_string_to_utf8(name);
    }

    static void BindEntityActive(CSEntityID ID, MonoBoolean isActive)
    {
        // Retrive the entity from the ID
        Entity Temp = VALIDATEENTITY(ID);
        if (!Temp) return;
        Temp->GetComponent<Properties>().m_IsDirty = (Temp->GetComponent<Properties>().m_Active != isActive);
        Temp->GetComponent<Properties>().m_Active = isActive;
    }

    static bool BindEntityGetActive(CSEntityID ID)
    {
        // Retrive the entity from the ID
        Entity Temp = VALIDATEENTITY(ID);
        if (!Temp) return false;
        return Temp->GetComponent<Properties>().m_Active;
    }

    static void BindEntitySetTag(CSEntityID ID, MonoString* tag)
    {
        // Retrive the entity from the ID
        Entity Temp = VALIDATEENTITY(ID);
        if (!Temp) return;
        Temp->GetComponent<Properties>().m_Tag = mono_string_to_utf8(tag);
    }
    
    static MonoString* BindEntityGetTag(CSEntityID ID)
    {
        // Retrive the entity from the ID
        Entity Temp = VALIDATEENTITY(ID);
        if (!Temp) return mono_string_new(mono_domain_get(), "");

        return mono_string_new(mono_domain_get(), Temp->GetComponent<Properties>().m_Tag.c_str());
    }
#pragma endregion

#pragma region PrefabBindings
    static bool BindCheckIsPrefabResource(CSEntityID ID)
    {
        std::string entityID{ EntityID_CSToEngine(ID) };
        if (ECSManager::Instance().FindEntity(entityID))
            return false;   // Return false if it is an entity in the Entity Scene
        // Check if entityID is a prefab resource
        return ECSSystemManager::Instance().GetSystem<PrefabSystem>()->IsValidPrefabResource(entityID);
    }

    static CSEntityID BindCreatePrefabEntity(CSEntityID prefabID, glm::vec3 newPos, glm::vec3 newRot)
    {
        std::string prefabGUID{ EntityID_CSToEngine(prefabID) };
        Entity prefabInstance{ ECSSystemManager::Instance().GetSystem<PrefabSystem>()->CreatePrefabEntityInstance(prefabGUID) };

        Transform& transform = prefabInstance->GetComponent<Transform>();
        transform.m_Position = newPos;
        transform.m_Rotation = newRot;
        transform.m_IsDirty = true;

        return EntityID_EngineToCS(prefabInstance->GetGUID());
    }
#pragma endregion
    
#pragma region ParentBindings
    static void BindParentSetParent(CSEntityID ID, CSEntityID parentID)
    {
        // Retrive the entity from the ID
        Entity Temp = VALIDATEENTITY(ID);
        Entity parentTemp = VALIDATEENTITY(parentID);
        if (Temp && parentTemp)
        {
            ECSSystemManager::Instance().GetSystem<ParentingSystem>()->SetParent(Temp, parentTemp);
        }
    }
    
    static void BindParentRemoveParent(CSEntityID ID)
    {
        // Retrive the entity from the ID
        Entity Temp = VALIDATEENTITY(ID);
        if (!Temp) return;
        ECSSystemManager::Instance().GetSystem<ParentingSystem>()->RemoveParent(Temp);
    }
    
    static void BindParentAddChild(CSEntityID ID, CSEntityID childID)
    {
        // Retrive the entity from the ID
        Entity Temp = VALIDATEENTITY(ID);
        Entity childTemp = VALIDATEENTITY(childID);
        if (Temp && childTemp)
        {
            ECSSystemManager::Instance().GetSystem<ParentingSystem>()->AddChild(Temp, childTemp);
        }
    }
    
    static void BindParentRemoveChild(CSEntityID ID, CSEntityID childID)
    {
        // Retrive the entity from the ID
        Entity Temp = VALIDATEENTITY(ID);
        Entity childTemp = VALIDATEENTITY(childID);
        if (Temp && childTemp)
        {
            ECSSystemManager::Instance().GetSystem<ParentingSystem>()->AbandonChild(Temp, childTemp);
        }
    }
    
    static CSEntityID BindParentGetChildFromIndex(CSEntityID ID, int index)
    {
        // Retrive the entity from the ID
        Entity Temp = VALIDATEENTITY(ID);
        if (index >= Temp->GetComponent<Parenting>().m_Children.size())
        {
            PUBLISHERROR("Index (" + std::to_string(index) + ") >= " + Temp->GetName() + "'s Children size (" + std::to_string(Temp->GetComponent<Parenting>().m_Children.size()) + ")!");
            return {};
        }

        return EntityID_EngineToCS(Temp->GetComponent<Parenting>().m_Children[index]);
    }

    static bool BindEntityCompareTag(CSEntityID ID, MonoString* tag)
    {
        // Retrive the entity from the ID
        Entity Temp = VALIDATEENTITY(ID);
        if (!Temp) return false;
        return Temp->GetComponent<Properties>().m_Tag == mono_string_to_utf8(tag);
    }
#pragma endregion

    static CSEntityID BindCreateEntity(MonoString* name, glm::vec3 pos, glm::vec3 rot, glm::vec3 sca)
    {
        char* nameString = mono_string_to_utf8(name);
        std::string str(nameString);
        mono_free(nameString);

        Entity Temp = ECSManager::Instance().CreateEntity(str);
        Temp->GetComponent<Transform>().m_Position = pos;
        Temp->GetComponent<Transform>().m_Rotation = rot;
        Temp->GetComponent<Transform>().m_Scale = sca;
        Temp->GetComponent<Transform>().m_IsDirty = true;
        std::cout << "Created Entity from C#: " << str << std::endl;
        return EntityID_EngineToCS(Temp->GetGUID());
    }

    static CSEntityID BindCloneEntity(CSEntityID ID, glm::vec3 pos, glm::vec3 rot, glm::vec3 sca)
    {
        Entity Existing{ VALIDATEENTITY(ID) };

        if (!Existing) return CSEntityID();

        Entity Temp = ECSManager::Instance().CloneEntity(Existing);
        Temp->GetComponent<Transform>().m_Position = pos;
        Temp->GetComponent<Transform>().m_Rotation = rot;
        Temp->GetComponent<Transform>().m_Scale = sca;
        Temp->GetComponent<Transform>().m_IsDirty = true;
        std::cout << "Cloned Entity from C#: " << Existing->GetName() << std::endl;
        return EntityID_EngineToCS(Temp->GetGUID());
    }

    static bool BindIsValidEntity(CSEntityID ID)
    {
        Entity Temp = ECSManager::Instance().FindEntity(EntityID_CSToEngine(ID));
        return (Temp != nullptr);
    }

	static void BindAddComponent(CSEntityID ID, int componenttype)
    {
        // Retrive the entity from the ID
        Entity Temp = VALIDATEENTITY(ID);
        if (!Temp) return;

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

    static void BindRemoveComponent(CSEntityID ID, int componenttype)
    {
        Entity Temp = VALIDATEENTITY(ID);
        if (!Temp) return;

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

    static void BindDestroyEntity(CSEntityID ID)
    {
        // find the entity
        Entity Temp = VALIDATEENTITY(ID);
        if (!Temp) return;

        ECSManager::Instance().MarkForDeletion(Temp);
    }

    static bool BindHasComponent(CSEntityID ID, MonoReflectionType* type)
    {
	    Entity entity = VALIDATEENTITY(ID);

        MonoType* monoType = mono_reflection_type_get_type(type);
        std::string ComponentName = mono_type_get_name(monoType);

        // for now the name of the type will be used to differentiate between components
        /*switch(ComponentName)
        {
        case"Transform":
            return entity->HasComponent<Transform>();
        case "MeshRenderer":
            return entity->HasComponent<MeshRenderer>();
        case"Camera":
            return entity->HasComponent<Camera>();
        case"Rigidbody":
            return entity->HasComponent<Rigidbody>();
        case"SphereCollider":
            return entity->HasComponent<SphereCollider>();
        case"BoxCollider":
            return entity->HasComponent<BoxCollider>();
        case"CapsuleCollider":
            return entity->HasComponent<CapsuleCollider>();
        case"Audio":
            return entity->HasComponent<Audio>();
        case"AudioListener":
            return entity->HasComponent<AudioListener>();
        case"Script":
            return entity->HasComponent<ScriptComponent>();
        default:
            TRE_ERROR("Component does not exist!");
            return false;
        }*/
        return false;
    }

    static void BindTestFunction()
    {
        Demo::SpawnObject();
    }

    static std::unordered_map<std::string, std::string> GetAllSceneObjects()
	{
        std::unordered_map<std::string, std::string > tempMap;
		std::vector temp = ECSManager::Instance().GetAllEntities(true);

        for(auto& entity : temp)
        {
            std::string temp1 = entity->GetName();
            std::string temp2 = entity->GetGUID();
            tempMap.insert(std::make_pair(temp1, temp2));
		}

		return tempMap;
	}

    static CSEntityID FindIDFromName(MonoString* name)
	{
        if (name == nullptr)
        {
            // Did not find the name
            PUBLISHERROR("Name is empty!");
            return CSEntityID();
        }

		std::string temp = MonoStringToString(name);
        std::unordered_map<std::string, std::string> sceneObjects = GetAllSceneObjects();
        // Search for the name in the map
        if(sceneObjects.find(temp) != sceneObjects.end())
        {
        	// Found the name
			std::string ID = sceneObjects[temp];
			return EntityID_EngineToCS(ID);
		}
		else
		{
			// Did not find the name
            PUBLISHERROR("Could not find name (" + MonoStringToString(name) + ") in ECS Entities!");
			return CSEntityID();
		}
	}

    static MonoString* FindNameFromID(CSEntityID ID)
	{
        Entity Temp = VALIDATEENTITY(ID);
        if (!Temp) return mono_string_new(mono_domain_get(), "");

        return  mono_string_new(mono_domain_get(), Temp->GetName().c_str());
	}

    static MonoString* FindParentIDFromID(CSEntityID ID)
	{
        Entity Temp = VALIDATEENTITY(ID);
        if (!Temp) return mono_string_new(mono_domain_get(), "");

        return  mono_string_new(mono_domain_get(), Temp->GetComponent<Parenting>().m_Parent.c_str());
	}

#pragma region TransformBindings

    static void BindSetPosition(CSEntityID ID, glm::vec3 newPos)
    {
        Entity Temp = VALIDATEENTITY(ID);
        if (!Temp) return;

        Transform& transform = Temp->GetComponent<Transform>();
        transform.m_Position = newPos;
        transform.m_IsDirty = true;
    }

    static void BindSetRotation(CSEntityID ID, glm::vec3 newRot)
    {
        Entity Temp = VALIDATEENTITY(ID);
        if (!Temp) return;

        Transform& transform = Temp->GetComponent<Transform>();
        transform.m_Rotation = newRot;
        transform.m_IsDirty = true;
    }

    static void BindSetScaling(CSEntityID ID, glm::vec3 newSca)
    {
        Entity Temp = VALIDATEENTITY(ID);
        if (!Temp) return;

        Transform& transform = Temp->GetComponent<Transform>();
        transform.m_Scale = newSca;
        transform.m_IsDirty = true;
    }

    static void BindGetPosition(CSEntityID ID, glm::vec3* output)
    {
        Entity Temp = VALIDATEENTITY(ID);
        if (!Temp) return;

        Transform& transform = Temp->GetComponent<Transform>();
        *output = transform.m_Position;
    }

    static void BindGetRotation(CSEntityID ID, glm::vec3* output)
    {
        Entity Temp = VALIDATEENTITY(ID);
        if (!Temp) return;

        // Get the rotation
        *output = Temp->GetComponent<Transform>().m_Rotation;
    }

    static void BindGetScaling(CSEntityID ID, glm::vec3* output)
    {
        Entity Temp = VALIDATEENTITY(ID);
        if (!Temp) return;

        // Get the rotation
        *output = Temp->GetComponent<Transform>().m_Scale;
    }

#pragma endregion

#pragma region CameraBindings
    static void BindCamSetViewportSize(CSEntityID ID, glm::vec2 newSize)
    {
        Entity Temp = VALIDATEENTITY(ID);
        if (!Temp) return;

        ECSSystemManager::Instance().GetSystem<CameraSystem>()->SetViewportSize(Temp, newSize);
    }

    static void BindCamSetFocalPoint(CSEntityID ID, glm::vec3 focalpoint)
    {
        Entity Temp = VALIDATEENTITY(ID);
        if (!Temp) return;

        ECSSystemManager::Instance().GetSystem<CameraSystem>()->SetFocalPoint(Temp, focalpoint);
    }

    static void BindCamSetFocalLength(CSEntityID ID, float focallength)
    {
        Entity Temp = VALIDATEENTITY(ID);
        if (!Temp) return;

        ECSSystemManager::Instance().GetSystem<CameraSystem>()->SetFocalLength(Temp, focallength);
    }

    static void BindCamSetFOV(CSEntityID ID, float fov)
    {
        Entity Temp = VALIDATEENTITY(ID);
        if (!Temp) return;

        ECSSystemManager::Instance().GetSystem<CameraSystem>()->SetFov(Temp, fov);
    }

    static void BindCamSetNear(CSEntityID ID, float n)
    {
        Entity Temp = VALIDATEENTITY(ID);
        if (!Temp) return;

        ECSSystemManager::Instance().GetSystem<CameraSystem>()->SetNear(Temp, n);
    }

    static void BindCamSetFar(CSEntityID ID, float f)
    {
        Entity Temp = VALIDATEENTITY(ID);
        if (!Temp) return;

        ECSSystemManager::Instance().GetSystem<CameraSystem>()->SetFar(Temp, f);
    }

    static void BindCamSetLeft(CSEntityID ID, float left)
    {
        Entity Temp = VALIDATEENTITY(ID);
        if (!Temp) return;

        ECSSystemManager::Instance().GetSystem<CameraSystem>()->SetLeft(Temp, left);
    }

    static void BindCamSetRight(CSEntityID ID, float right)
    {
        Entity Temp = VALIDATEENTITY(ID);
        if (!Temp) return;

        ECSSystemManager::Instance().GetSystem<CameraSystem>()->SetRight(Temp, right);
    }

    static void BindCamSetTop(CSEntityID ID, float top)
    {
        Entity Temp = VALIDATEENTITY(ID);
        if (!Temp) return;

        ECSSystemManager::Instance().GetSystem<CameraSystem>()->SetTop(Temp, top);
    }

    static void BindCamSetBottom(CSEntityID ID, float bottom)
    {
        Entity Temp = VALIDATEENTITY(ID);
        if (!Temp) return;

        ECSSystemManager::Instance().GetSystem<CameraSystem>()->SetBottom(Temp, bottom);
    }

    static void BindCamSetAspectRatio(CSEntityID ID, float aspectRatio)
    {
        Entity Temp = VALIDATEENTITY(ID);
        if (!Temp) return;

        ECSSystemManager::Instance().GetSystem<CameraSystem>()->SetAspectRatio(Temp, aspectRatio);
    }

    static void BindCamSetIsPerspective(CSEntityID ID, bool isPerspective)
    {
        Entity Temp = VALIDATEENTITY(ID);
        if (!Temp) return;

        ECSSystemManager::Instance().GetSystem<CameraSystem>()->SetIsPerspective(Temp, isPerspective);
    }

    static void BindCamSetIsMainCamera(CSEntityID ID, bool isMainCamera)
    {
        Entity Temp = VALIDATEENTITY(ID);
        if (!Temp) return;

        ECSSystemManager::Instance().GetSystem<CameraSystem>()->SetIsMainCamera(Temp, isMainCamera);
    }

    // Getters
    static void BindCamGetViewMatrix(CSEntityID ID, glm::mat4* result)
    {
        Entity Temp = VALIDATEENTITY(ID);
        if (!Temp) return;

        *result = ECSSystemManager::Instance().GetSystem<CameraSystem>()->GetViewMatrix(Temp);
    }

    static void BindCamGetProjectionMatrix(CSEntityID ID, glm::mat4* result)
    {
        Entity Temp = VALIDATEENTITY(ID);
        if (!Temp) return;

        *result = ECSSystemManager::Instance().GetSystem<CameraSystem>()->GetProjectionMatrix(Temp);
    }

    static void BindCamGetInverseViewMatrix(CSEntityID ID, glm::mat4* result)
    {
        Entity Temp = VALIDATEENTITY(ID);
        if (!Temp) return;

        *result = ECSSystemManager::Instance().GetSystem<CameraSystem>()->GetInverseViewMatrix(Temp);
    }

    static void BindCamGetInverseProjectionMatrix(CSEntityID ID, glm::mat4* result)
    {
        Entity Temp = VALIDATEENTITY(ID);
        if (!Temp) return;

        *result = ECSSystemManager::Instance().GetSystem<CameraSystem>()->GetInverseProjectionMatrix(Temp);
    }

    static void BindCamGetInverseViewProjectionMatrix(CSEntityID ID, glm::mat4* result)
    {
        Entity Temp = VALIDATEENTITY(ID);
        if (!Temp) return;

        *result = ECSSystemManager::Instance().GetSystem<CameraSystem>()->GetInverseViewProjectionMatrix(Temp);
    }

    static void BindCamGetViewportSize(CSEntityID ID, glm::vec2* result)
    {
        Entity Temp = VALIDATEENTITY(ID);
        if (!Temp) return;

        *result = ECSSystemManager::Instance().GetSystem<CameraSystem>()->GetViewportSize(Temp);
    }

    static void BindCamGetFOV(CSEntityID ID, float* result)
    {
        Entity Temp = VALIDATEENTITY(ID);
        if (!Temp) return;

        *result = ECSSystemManager::Instance().GetSystem<CameraSystem>()->GetFov(Temp);
    }

    static void BindCamGetNear(CSEntityID ID, float* result)
    {
        Entity Temp = VALIDATEENTITY(ID);
        if (!Temp) return;

        *result = ECSSystemManager::Instance().GetSystem<CameraSystem>()->GetNear(Temp);
    }

    static void BindCamGetFar(CSEntityID ID, float* result)
    {
        Entity Temp = VALIDATEENTITY(ID);
        if (!Temp) return;

        *result = ECSSystemManager::Instance().GetSystem<CameraSystem>()->GetFar(Temp);
    }

    static void BindCamGetLeft(CSEntityID ID, float* result)
    {
        Entity Temp = VALIDATEENTITY(ID);
        if (!Temp) return;

        *result = ECSSystemManager::Instance().GetSystem<CameraSystem>()->GetLeft(Temp);
    }

    static void BindCamGetRight(CSEntityID ID, float* result)
    {
        Entity Temp = VALIDATEENTITY(ID);
        if (!Temp) return;

        *result = ECSSystemManager::Instance().GetSystem<CameraSystem>()->GetRight(Temp);
    }

    static void BindCamGetTop(CSEntityID ID, float* result)
    {
        Entity Temp = VALIDATEENTITY(ID);
        if (!Temp) return;

        *result = ECSSystemManager::Instance().GetSystem<CameraSystem>()->GetTop(Temp);
    }

    static void BindCamGetBottom(CSEntityID ID, float* result)
    {
        Entity Temp = VALIDATEENTITY(ID);
        if (!Temp) return;

        *result = ECSSystemManager::Instance().GetSystem<CameraSystem>()->GetBottom(Temp);
    }

    static void BindCamGetAspectRatio(CSEntityID ID, float* result)
    {
        Entity Temp = VALIDATEENTITY(ID);
        if (!Temp) return;

        *result = ECSSystemManager::Instance().GetSystem<CameraSystem>()->GetAspectRatio(Temp);
    }

    static void BindCamIsPerspective(CSEntityID ID, bool* result)
    {
        Entity Temp = VALIDATEENTITY(ID);
        if (!Temp) return;

        *result = ECSSystemManager::Instance().GetSystem<CameraSystem>()->IsPerspective(Temp);
    }

    static void BindCamIsMainCamera(CSEntityID ID, bool* result)
    {
        Entity Temp = VALIDATEENTITY(ID);
        if (!Temp) return;

        *result = ECSSystemManager::Instance().GetSystem<CameraSystem>()->IsMainCamera(Temp);
    }

    static void BindCamMainSetLookAt(glm::vec3* target, float distance)
    {
        ECSSystemManager::Instance().GetSystem<CameraSystem>()->MainCameraLookAt(*target, distance);
    }

    static void BindTransitionMainCamera(glm::vec3* targetPosition, glm::vec3* targetRotation, float speed)
    {
        ECSSystemManager::Instance().GetSystem<CameraSystem>()->TransitionCamera(*targetPosition, *targetRotation, speed);
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

        EventHandler::getEventHandlerInstance().Publish(ConsoleDebugEvent{ str.c_str()});
        //TRE_INFO(str);
    }

    static void SendWarningToConsole(MonoString* message)
    {
        std::string str = CONSOLE_DEBUG_WARN + MonoStringToString(message);

        EventHandler::getEventHandlerInstance().Publish(ConsoleDebugEvent{ str.c_str() });
        //TRE_WARN(str);
    }

    static void SendErrorToConsole(MonoString* message)
    {
        std::string str = CONSOLE_DEBUG_ERROR + MonoStringToString(message);

        EventHandler::getEventHandlerInstance().Publish(ConsoleDebugEvent{ str.c_str() });
        //TRE_ERROR(str);
    }

    static void SendCriticalToConsole(MonoString* message)
    {
        std::string str = CONSOLE_DEBUG_ERROR + MonoStringToString(message);

        EventHandler::getEventHandlerInstance().Publish(ConsoleDebugEvent{ str.c_str() });
        //TRE_CRITICAL(str);
    }

#pragma endregion

#pragma region Physics
    static void BindResizeSphereCollider(CSEntityID ID, float radius)
    {
        const Entity& entity = VALIDATEENTITY(ID);
        if (!entity) return;

        if (!entity->HasComponent<SphereCollider>())
        {
            PUBLISHERROR("There is no SphereCollider in " + entity->GetName() + "!");
            return;
        }

        ECSSystemManager::Instance().GetSystem<PhysicsSystem>()->ResizeSphereCollider(entity, radius);
    }

    static void BindResizeBoxCollider(CSEntityID ID, glm::vec3 halfExtents)
    {
        const Entity& entity = VALIDATEENTITY(ID);
        if (!entity) return;

        if (!entity->HasComponent<BoxCollider>())
        {
            PUBLISHERROR("There is no BoxCollider in " + entity->GetName() + "!");
            return;
        }

        ECSSystemManager::Instance().GetSystem<PhysicsSystem>()->ResizeBoxCollider(entity, halfExtents);
    }

    static void BindResizeCapsuleCollider(CSEntityID ID, float radius, float halfHeight)
    {
        const Entity& entity = VALIDATEENTITY(ID);
        if (!entity) return;

        if (!entity->HasComponent<CapsuleCollider>())
        {
            PUBLISHERROR("There is no CapsuleCollider in " + entity->GetName() + "!");
            return;
        }

        ECSSystemManager::Instance().GetSystem<PhysicsSystem>()->ResizeCapsuleCollider(entity, radius, halfHeight);
    }

    static void BindAddForce(CSEntityID ID, glm::vec3 force, ForceMode::Enum mode)
    {
        const Entity& entity = VALIDATEENTITY(ID);
        if (!entity) return;

        if (!entity->HasComponent<Rigidbody>())
        {
            PUBLISHERROR("There is no Rigidbody in " + entity->GetName() + "!");
            return;
        }

        ECSSystemManager::Instance().GetSystem<PhysicsSystem>()->AddForce(entity, force, mode);
    }

    void BindConstrainRotationX(CSEntityID ID, bool state)
    {
        const Entity& entity = VALIDATEENTITY(ID);
        if (!entity) return;

        if (!entity->HasComponent<Rigidbody>())
        {
            PUBLISHERROR("There is no Rigidbody in " + entity->GetName() + "!");
            return;
        }

        ECSSystemManager::Instance().GetSystem<PhysicsSystem>()->ConstrainRotationX(entity, state);
    }

    void BindConstrainRotationY(CSEntityID ID, bool state)
    {
        const Entity& entity = VALIDATEENTITY(ID);
        if (!entity) return;

        if (!entity->HasComponent<Rigidbody>())
        {
            PUBLISHERROR("There is no Rigidbody in " + entity->GetName() + "!");
            return;
        }

        ECSSystemManager::Instance().GetSystem<PhysicsSystem>()->ConstrainRotationY(entity, state);
    }

    void BindConstrainRotationZ(CSEntityID ID, bool state)
    {
        const Entity& entity = VALIDATEENTITY(ID);
        if (!entity) return;

        if (!entity->HasComponent<Rigidbody>())
        {
            PUBLISHERROR("There is no Rigidbody in " + entity->GetName() + "!");
            return;
        }

        ECSSystemManager::Instance().GetSystem<PhysicsSystem>()->ConstrainRotationZ(entity, state);
    }

    static void BindGetLinearVelocity(CSEntityID ID, glm::vec3* output)
    {
        const Entity& entity = VALIDATEENTITY(ID);
        if (!entity) return;

        if (!entity->HasComponent<Rigidbody>())
        {
            PUBLISHERROR("There is no Rigidbody in " + entity->GetName() + "!");
            return;
        }

        *output = ECSSystemManager::Instance().GetSystem<PhysicsSystem>()->GetLinearVelocity(entity);
    }

    static void BindSetLinearVelocity(CSEntityID ID, glm::vec3 velocity)
    {
        const Entity& entity = VALIDATEENTITY(ID);
        if (!entity) return;

        if (!entity->HasComponent<Rigidbody>())
        {
            PUBLISHERROR("There is no Rigidbody in " + entity->GetName() + "!");
            return;
        }

        ECSSystemManager::Instance().GetSystem<PhysicsSystem>()->SetLinearVelocity(entity, velocity);
    }

    static bool BindIsCollisionEnter(CSEntityID id1, CSEntityID id2)
    {
        Entity entity1 = VALIDATEENTITY(id1);
        Entity entity2 = VALIDATEENTITY(id2);
        if (!entity1 || !entity2) return false;

        return ECSSystemManager::Instance().GetSystem<PhysicsSystem>()->IsCollisionEnter(entity1, entity2);
    }

    static bool BindIsCollisionStay(CSEntityID id1, CSEntityID id2)
    {
        Entity entity1 = VALIDATEENTITY(id1);
        Entity entity2 = VALIDATEENTITY(id2);
        if (!entity1 || !entity2) return false;

        return ECSSystemManager::Instance().GetSystem<PhysicsSystem>()->IsCollisionStay(entity1, entity2);
    }

    static bool BindIsCollisionExit(CSEntityID id1, CSEntityID id2)
    {
        Entity entity1 = VALIDATEENTITY(id1);
        Entity entity2 = VALIDATEENTITY(id2);
        if (!entity1 || !entity2) return false;

        return ECSSystemManager::Instance().GetSystem<PhysicsSystem>()->IsCollisionExit(entity1, entity2);
    }

    static bool BindIsTriggerEnter(CSEntityID id1, CSEntityID id2)
    {
        Entity entity1 = VALIDATEENTITY(id1);
        Entity entity2 = VALIDATEENTITY(id2);
        if (!entity1 || !entity2) return false;

        return ECSSystemManager::Instance().GetSystem<PhysicsSystem>()->IsTriggerEnter(entity1, entity2);
    }

    static bool BindIsTriggerStay(CSEntityID id1, CSEntityID id2)
    {
        Entity entity1 = VALIDATEENTITY(id1);
        Entity entity2 = VALIDATEENTITY(id2);
        if (!entity1 || !entity2) return false;

        return ECSSystemManager::Instance().GetSystem<PhysicsSystem>()->IsTriggerStay(entity1, entity2);
    }

    static bool BindIsTriggerExit(CSEntityID id1, CSEntityID id2)
    {
        Entity entity1 = VALIDATEENTITY(id1);
        Entity entity2 = VALIDATEENTITY(id2);
        if (!entity1 || !entity2) return false;

        return ECSSystemManager::Instance().GetSystem<PhysicsSystem>()->IsTriggerExit(entity1, entity2);
    }
#pragma endregion

#pragma region RigidBodyBindings
    static void BindSetKinematic(CSEntityID ID, bool enable)
    {
        Entity Temp = VALIDATEENTITY(ID);
        if (!Temp) return;

        Temp->GetComponent<Rigidbody>().m_IsKinematic = enable;
        Temp->GetComponent<Rigidbody>().m_IsDirty = true;
    }

    static void BindSetGravity(CSEntityID ID, bool enable)
    {
        Entity Temp = VALIDATEENTITY(ID);
        if (!Temp) return;

        Temp->GetComponent<Rigidbody>().m_UseGravity = enable;
        Temp->GetComponent<Rigidbody>().m_IsDirty = true;
    }
#pragma endregion

#pragma region TimeBindings
    static float BindGetDeltaTime()
    {
        return Engine::GetInstance().GetWindow()->GetDeltaTime();
    }
#pragma endregion

#pragma region MathfBindings
    static float BindSqrt(float value)
    {
        return Mathf::Sqrt(value);
    }
#pragma endregion

#pragma region RandomBindings
    static int BindIntRandRange(int min_incl, int max_excl)
    {
        return Random::RangeInt(min_incl, max_excl);
    }

    static float BindFloatRandRange(float min_incl, float max_incl)
    {
        return Random::RangeFloat(min_incl, max_incl);
    }

#pragma endregion

#pragma region Audio

    static void BindSetPlaySound(MonoString* id)
    {
        Entity entity = ECSManager::Instance().FindEntity(MonoStringToString(id));
        return ECSSystemManager::Instance().GetSystem<AudioSystem>()->SetPlay(entity, true);
    }

    static void BindTogglePauseSound(MonoString* id, bool paused)
    {
        Entity entity = ECSManager::Instance().FindEntity(MonoStringToString(id));
        return ECSSystemManager::Instance().GetSystem<AudioSystem>()->SetPause(entity, paused);
    }

    static void BindSetStopSound(MonoString* id)
    {
        Entity entity = ECSManager::Instance().FindEntity(MonoStringToString(id));
        ECSSystemManager::Instance().GetSystem<AudioSystem>()->SetPlay(entity, false);
        return ECSSystemManager::Instance().GetSystem<AudioSystem>()->StopAudio(entity);
    }

    static bool BindIsPlaying(MonoString* id)
    {
        Entity entity = ECSManager::Instance().FindEntity(MonoStringToString(id));
        return ECSSystemManager::Instance().GetSystem<AudioSystem>()->GetIsPlaying(entity);
    }

#pragma endregion


#pragma region ScriptBindings
    static bool BindIsScript(MonoString* className)
    {
        if (!className)
        {
            PUBLISHERROR("ClassName is invalid!");
            return false;
        }
        std::string classNameStr{ MonoStringToString(className) };
        auto classes{ ScriptEngine::s_ScriptEngineData->ScriptClasses };

        return classes.find(classNameStr) != classes.end();
    }

    static bool BindHaveScript(CSEntityID ID, MonoString* className)
    {
        Entity Temp{ VALIDATEENTITY(ID) };
        if (!Temp) return false;
        if (!Temp->HasComponent<ScriptComponent>()) return false;

        if (!className)
        {
            PUBLISHERROR("ClassName is invalid!");
            return false;
        }
        std::string classNameStr{ MonoStringToString(className) };

        return Temp->GetComponent<ScriptComponent>().m_StoredClass == classNameStr;
    }

    static MonoObject* BindGetScript(CSEntityID ID, MonoString* className)
    {
        if (!BindHaveScript(ID, className)) return NULL;

        std::string IDStr{ EntityID_CSToEngine(ID) };
        std::string classNameStr{ MonoStringToString(className) };

        auto instances{ ScriptEngine::s_ScriptEngineData->ScriptInstances };

        return instances[IDStr]->GetScriptObject();
    }
#pragma endregion

    void ScriptBind::RegisterFunctions()
    {
        // ECS Bindings
	    {
		    mono_add_internal_call("TRE.ECSManager::CreateEntity", BindCreateEntity);
	    	mono_add_internal_call("TRE.ECSManager::CloneEntity", BindCloneEntity);
	    	mono_add_internal_call("TRE.ECSManager::IsValidEntity", BindIsValidEntity);
	    	mono_add_internal_call("TRE.ECSManager::AddComponent", BindAddComponent);
	    	mono_add_internal_call("TRE.ECSManager::RemoveComponent", BindRemoveComponent);
	    	mono_add_internal_call("TRE.ECSManager::DestroyEntity", BindDestroyEntity);
	    	mono_add_internal_call("TRE.Demo::SpawnObject", BindTestFunction);
	    	mono_add_internal_call("TRE.ECSManager::FindIDFromName", FindIDFromName);
	    	mono_add_internal_call("TRE.ECSManager::FindNameFromID", FindNameFromID);
	    	mono_add_internal_call("TRE.ECSManager::FindParentIDFromID", FindParentIDFromID);
            mono_add_internal_call("TRE.ECSManager::HasComponent",BindHasComponent);
	    }

        // Entity Bindings
	    {
		    mono_add_internal_call("TRE.Entity::EngineRename", BindEntityRename);
	    	mono_add_internal_call("TRE.Entity::EngineSetActive", BindEntityActive);
	    	mono_add_internal_call("TRE.Entity::EngineGetActive", BindEntityGetActive);
	    	mono_add_internal_call("TRE.Entity::EngineSetTag", BindEntitySetTag);
	    	mono_add_internal_call("TRE.Entity::EngineGetTag", BindEntityGetTag);
	    	mono_add_internal_call("TRE.Entity::EngineCompareTag", BindEntityCompareTag);
	    }

        // Prefab Bindings
	    {
		    mono_add_internal_call("TRE.Prefab::EngineIsPrefabResource", BindCheckIsPrefabResource);
	    	mono_add_internal_call("TRE.Prefab::CreatePrefabEntity", BindCreatePrefabEntity);
	    }

        // Parent Bindings
	    {
		    mono_add_internal_call("TRE.Parenting::EngineParentSetParent", BindParentSetParent);
	    	mono_add_internal_call("TRE.Parenting::EngineParentRemoveParent", BindParentRemoveParent);
	    	mono_add_internal_call("TRE.Parenting::EngineParentAddChild", BindParentAddChild);
	    	mono_add_internal_call("TRE.Parenting::EngineParentRemoveChild", BindParentRemoveChild);
	    	mono_add_internal_call("TRE.Parenting::EngineGetChildID", BindParentGetChildFromIndex);
	    }

        // Transform Bindings
	    {
		    mono_add_internal_call("TRE.TransformSystem::SetPosition", BindSetPosition);
	    	mono_add_internal_call("TRE.TransformSystem::SetRotation", BindSetRotation);
	    	mono_add_internal_call("TRE.TransformSystem::SetScaling", BindSetScaling);
	    	mono_add_internal_call("TRE.TransformSystem::GetPosition", BindGetPosition);
	    	mono_add_internal_call("TRE.TransformSystem::GetRotation", BindGetRotation);
	    	mono_add_internal_call("TRE.TransformSystem::GetScaling", BindGetScaling);
	    }

        // Camera Bindings
	    {
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
            mono_add_internal_call("TRE.CameraSystem::SetMainCameraLookAt", BindCamMainSetLookAt);
            mono_add_internal_call("TRE.CameraSystem::TransitionMainCamera", BindTransitionMainCamera);
	    }

        // Physics Bindings
	    {
            mono_add_internal_call("TRE.PhysicsSystem::ResizeSphereCollider", BindResizeSphereCollider);
            mono_add_internal_call("TRE.PhysicsSystem::ResizeBoxCollider", BindResizeBoxCollider);
            mono_add_internal_call("TRE.PhysicsSystem::ResizeCapsuleCollider", BindResizeCapsuleCollider);
            mono_add_internal_call("TRE.PhysicsSystem::AddForce", BindAddForce);
            mono_add_internal_call("TRE.PhysicsSystem::ConstrainRotationX", BindConstrainRotationX);
            mono_add_internal_call("TRE.PhysicsSystem::ConstrainRotationY", BindConstrainRotationY);
            mono_add_internal_call("TRE.PhysicsSystem::ConstrainRotationZ", BindConstrainRotationZ);
            mono_add_internal_call("TRE.PhysicsSystem::GetLinearVelocity", BindGetLinearVelocity);
            mono_add_internal_call("TRE.PhysicsSystem::SetLinearVelocity", BindSetLinearVelocity);
            mono_add_internal_call("TRE.PhysicsSystem::IsCollisionEnter", BindIsCollisionEnter);
            mono_add_internal_call("TRE.PhysicsSystem::IsCollisionStay", BindIsCollisionStay);
            mono_add_internal_call("TRE.PhysicsSystem::IsCollisionExit", BindIsCollisionExit);
            mono_add_internal_call("TRE.PhysicsSystem::IsTriggerEnter", BindIsTriggerEnter);
            mono_add_internal_call("TRE.PhysicsSystem::IsTriggerStay", BindIsTriggerStay);
            mono_add_internal_call("TRE.PhysicsSystem::IsTriggerExit", BindIsTriggerExit);
	    }

        // RigidBody Binding
        {
            mono_add_internal_call("TRE.RigidBodySystem::SetKinematic", BindSetKinematic);
            mono_add_internal_call("TRE.RigidBodySystem::SetGravity", BindSetGravity);
	    }

        // Input Binding
	    {
		    mono_add_internal_call("TRE.InputSystem::GetKeyDown", GetKeyDown);
	    }

        // Logging
	    {
		    mono_add_internal_call("TRE.Debug::Log", SendMessageToConsole);
	    	mono_add_internal_call("TRE.Debug::LogWarning", SendWarningToConsole);
	    	mono_add_internal_call("TRE.Debug::LogError", SendErrorToConsole);
	    	mono_add_internal_call("TRE.Debug::LogCritical", SendCriticalToConsole);
	    }

        // Math
	    {
		    mono_add_internal_call("TRE.MathF::Sqrt", BindSqrt);
	    }

        // Random
	    {
		    mono_add_internal_call("TRE.Random::IntRange", BindIntRandRange);
	    	mono_add_internal_call("TRE.Random::FloatRange", BindFloatRandRange);
	    }

        // Time
	    {
		    mono_add_internal_call("TRE.Time::GetDeltaTime", BindGetDeltaTime);
	    }

        //Audio
        {
            mono_add_internal_call("TRE.AudioSystem::SetPlay", BindSetPlaySound);
            mono_add_internal_call("TRE.AudioSystem::SetPause", BindTogglePauseSound);
            mono_add_internal_call("TRE.AudioSystem::StopAudio", BindSetStopSound);
            mono_add_internal_call("TRE.AudioSystem::GetIsPlaying", BindIsPlaying);
        }

        // Scripting
        {
            mono_add_internal_call("TRE.Script::IsScript", BindIsScript);
            mono_add_internal_call("TRE.Script::HaveScript", BindHaveScript);
            mono_add_internal_call("TRE.Script::GetScript", BindGetScript);
        }
    }
}
