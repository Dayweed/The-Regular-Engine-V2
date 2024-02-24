#include "pch.h"
#include "Scripting/ScriptBind.h"

#include"Scripting/ScriptEngine.h"
#include "Core/ECS.h"
#include "ECS/Components/Transform.h"
#include "Core/GameLoop.h"
#include "Resource/Resource.h"

#include "Audio/AudioSystem.h"
#include "Graphics/Camera.h"
#include "ECS/Components/MeshRenderer.h"
#include "Graphics/Renderer.h"
#include "ECS/Components/Particle.h"
#include "EventSystem/EventHandler/EventHandler.h"
#include "EventSystem/Events/EditorEvent.h"

#include "InputHandler/InputHandler.h"

#include "mono/metadata/object.h"
#include "mono/metadata/reflection.h"

#define VALIDATEENTITY(ID) ValidateEntityID(ID, __FUNCTION__)

#define PUBLISHERROR(msg) PublishError(msg, __FUNCTION__)
#define PUBLISHWARN(msg) PublishWarning(msg, __FUNCTION__)
#define PUBLISHLOG(msg) PublishLog(msg, __FUNCTION__)

namespace TRE
{
	enum class ComponentsID
	{
		MeshRenderer = 0,
		Camera,
		Audio,
		Rigidbody,
		SphereCollider,
		BoxCollider,
		CapsuleCollider,
		CylinderCollider,
		AudioListener,
		Script,
		Transform,
		SpriteRenderer,
		Parenting,
		Animation,
		DirectPathfinding,
		Text,
		Particle,
		None
	};
	std::unordered_map<std::string, ComponentsID> ComponentsMap
	{
		{"TRE.Transform", ComponentsID::Transform},
		{"TRE.MeshRenderer", ComponentsID::MeshRenderer},
		{"TRE.Camera", ComponentsID::Camera},
		{"TRE.Audio", ComponentsID::Audio},
		{"TRE.Rigidbody", ComponentsID::Rigidbody},
		{"TRE.SphereCollider", ComponentsID::SphereCollider},
		{"TRE.BoxCollider", ComponentsID::BoxCollider},
		{"TRE.CapsuleCollider", ComponentsID::CapsuleCollider},
		{"TRE.CyclinderCollider", ComponentsID::CylinderCollider},
		{"TRE.AudioListener", ComponentsID::AudioListener},
		{"TRE.SpriteRenderer", ComponentsID::SpriteRenderer},
		{"TRE.Parenting", ComponentsID::Parenting},
		{"TRE.Script", ComponentsID::Script},
		{"TRE.Animation", ComponentsID::Animation},
		{"TRE.DirectPathfinding", ComponentsID::DirectPathfinding},
		{"TRE.Text", ComponentsID::Text},
		{"TRE.Particle", ComponentsID::Particle}
	};

	namespace Tools
	{
		ComponentsID ConvertComponentNameToID(std::string componentName)
		{
			if (ComponentsMap.find(componentName) != ComponentsMap.end())
			{
				return ComponentsMap[componentName];
			}
			else
			{
				return ComponentsID::None;
			}
		}
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
		// Retrieve the entity from the ID
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

	/*
	static void PublishWarning(std::string message, std::string function)
	{
		std::string str{ CONSOLE_DEBUG_WARN };
		str += "[" + function + "] " + message;
		EventHandler::getEventHandlerInstance().Publish(ConsoleDebugEvent{ str.c_str() });
	}
	*/

	/*
	static void PublishLog(std::string message, std::string function)
	{
		std::string str{ "[" + function + "] " + message };
		EventHandler::getEventHandlerInstance().Publish(ConsoleDebugEvent{ str.c_str() });
	}
	*/

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
		// Retrieve the entity from the ID
		Entity Temp = VALIDATEENTITY(ID);
		if (!Temp) return;
		Temp->GetComponent<Properties>().m_Name = mono_string_to_utf8(name);
	}

	static void BindEntityActive(CSEntityID ID, MonoBoolean isActive)
	{
		// Retrieve the entity from the ID
		Entity Temp = VALIDATEENTITY(ID);
		if (!Temp) return;
		Temp->GetComponent<Properties>().m_IsDirty = (Temp->GetComponent<Properties>().m_Active != (bool)isActive);
		Temp->GetComponent<Properties>().m_Active = (bool)isActive;
		ECSSystemManager::Instance().GetSystem<ParentingSystem>()->UpdateChildActive(Temp);
	}

	static bool BindEntityGetActive(CSEntityID ID)
	{
		// Retrieve the entity from the ID
		Entity Temp = VALIDATEENTITY(ID);
		if (!Temp) return false;
		return Temp->GetComponent<Properties>().m_Active;
	}

	static void BindEntitySetTag(CSEntityID ID, MonoString* tag)
	{
		// Retrieve the entity from the ID
		Entity Temp = VALIDATEENTITY(ID);
		if (!Temp) return;
		Temp->GetComponent<Properties>().m_Tag = mono_string_to_utf8(tag);
	}

	static MonoString* BindEntityGetTag(CSEntityID ID)
	{
		// Retrieve the entity from the ID
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

	static CSEntityID BindCreatePrefabEntity(CSEntityID prefabID/*, glm::vec3 newPos, glm::vec3 newRot*/)
	{
		std::string prefabGUID{ EntityID_CSToEngine(prefabID) };
		Entity prefabInstance{ ECSSystemManager::Instance().GetSystem<PrefabSystem>()->CreatePrefabEntityInstance(prefabGUID) };

		/*Transform& transform = prefabInstance->GetComponent<Transform>();
		transform.m_Position = newPos;
		transform.m_Rotation = newRot;
		transform.m_IsDirty = true;*/

		return EntityID_EngineToCS(prefabInstance->GetGUID());
	}
#pragma endregion

#pragma region ParentBindings
	static void BindParentSetParent(CSEntityID ID, CSEntityID parentID)
	{
		// Retrieve the entity from the ID
		Entity Temp = VALIDATEENTITY(ID);
		Entity parentTemp = VALIDATEENTITY(parentID);
		if (Temp && parentTemp)
		{
			ECSSystemManager::Instance().GetSystem<ParentingSystem>()->SetParent(Temp, parentTemp);
		}
	}

	static void BindParentRemoveParent(CSEntityID ID)
	{
		// Retrieve the entity from the ID
		Entity Temp = VALIDATEENTITY(ID);
		if (!Temp) return;
		ECSSystemManager::Instance().GetSystem<ParentingSystem>()->RemoveParent(Temp);
	}

	static void BindParentAddChild(CSEntityID ID, CSEntityID childID)
	{
		// Retrieve the entity from the ID
		Entity Temp = VALIDATEENTITY(ID);
		Entity childTemp = VALIDATEENTITY(childID);
		if (Temp && childTemp)
		{
			ECSSystemManager::Instance().GetSystem<ParentingSystem>()->AddChild(Temp, childTemp);
		}
	}

	static void BindParentRemoveChild(CSEntityID ID, CSEntityID childID)
	{
		// Retrieve the entity from the ID
		Entity Temp = VALIDATEENTITY(ID);
		Entity childTemp = VALIDATEENTITY(childID);
		if (Temp && childTemp)
		{
			ECSSystemManager::Instance().GetSystem<ParentingSystem>()->AbandonChild(Temp, childTemp);
		}
	}

	static CSEntityID BindParentGetChildFromIndex(CSEntityID ID, int index)
	{
		// Retrieve the entity from the ID
		Entity Temp = VALIDATEENTITY(ID);
		if (index >= Temp->GetComponent<Parenting>().m_Children.size())
		{
			PUBLISHERROR("Index (" + std::to_string(index) + ") >= " + Temp->GetName() + "'s Children size (" + std::to_string(Temp->GetComponent<Parenting>().m_Children.size()) + ")!");
			return {};
		}

		return EntityID_EngineToCS(Temp->GetComponent<Parenting>().m_Children[index]);
	}

	static CSEntityID BindParentGetChildFromName(CSEntityID ID, MonoString* name)
	{
		// Retrieve the entity from the ID
		Entity Temp = VALIDATEENTITY(ID);
		std::string searchID = mono_string_to_utf8(name);

		for (std::string childID : Temp->GetComponent<Parenting>().m_Children)
		{
			std::string childName{ ECSManager::Instance().FindEntity(childID)->GetName() };
			if (childName == searchID) return EntityID_EngineToCS(childID);
		}

		PUBLISHERROR("ID (" + searchID + ") is not found in " + Temp->GetName() + "!");
		return {};
	}

	static int BindParentGetTotalChildren(CSEntityID ID)
	{
		// Retrieve the entity from the ID
		Entity Temp = VALIDATEENTITY(ID);

		return Temp->GetComponent<Parenting>().m_Children.size();
	}

	static bool BindEntityCompareTag(CSEntityID ID, MonoString* tag)
	{
		// Retrieve the entity from the ID
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
		Temp->GetComponent<Transform>().m_DirtyFlags = TransformDirtyFlags::TRE_DIRTY_ALL;

		if (Temp->HasComponent<ScriptComponent>())
		{
			ScriptEngine::CreateCSEntityData(Temp);
		}

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
		Temp->GetComponent<Transform>().m_DirtyFlags = TransformDirtyFlags::TRE_DIRTY_ALL;

		if (Temp->HasComponent<ScriptComponent>())
		{
			ScriptEngine::CreateCSEntityData(Temp);
		}

		std::cout << "Cloned Entity from C#: " << Existing->GetName() << std::endl;
		return EntityID_EngineToCS(Temp->GetGUID());
	}

	static bool BindIsValidEntity(CSEntityID ID)
	{
		Entity Temp = ECSManager::Instance().FindEntity(EntityID_CSToEngine(ID));
		return (Temp != nullptr);
	}

	static void BindAddComponent(CSEntityID ID, MonoReflectionType* componenttype)
	{
		// Retrieve the entity from the ID
		Entity Temp = VALIDATEENTITY(ID);
		if (!Temp) return;

		MonoType* type = mono_reflection_type_get_type(componenttype);
		ComponentsID componentID = Tools::ConvertComponentNameToID(mono_type_get_name(type));

		// Use a switch case to determine which component to add
		switch (componentID)
		{
		case ComponentsID::MeshRenderer: // mesh
			Temp->AddComponent<MeshRenderer>();
			TRE_INFO("MeshRenderer added to {0}({1})", Temp->GetName(), Temp->GetGUID());
			break;
		case ComponentsID::Camera:
			Temp->AddComponent<Camera>();
			TRE_INFO("Camera added to {0}({1})", Temp->GetName(), Temp->GetGUID());
			break;
		case ComponentsID::Audio:
			Temp->AddComponent<Audio>();
			TRE_INFO("Audio added to {0}({1})", Temp->GetName(), Temp->GetGUID());
			break;
		case ComponentsID::Rigidbody:
			Temp->AddComponent<Rigidbody>().m_IsDirty = true;
			ECSSystemManager::Instance().GetSystem<PhysicsSystem>()->ConstructRigidbody(Temp);
			TRE_INFO("Rigidbody added to {0}({1})", Temp->GetName(), Temp->GetGUID());
			break;
		case ComponentsID::BoxCollider:
			Temp->AddComponent<BoxCollider>().m_IsDirty = true;
			ECSSystemManager::Instance().GetSystem<PhysicsSystem>()->ConstructBoxCollider(Temp);
			TRE_INFO("Box Collider added to {0}({1})", Temp->GetName(), Temp->GetGUID());
			break;
		case ComponentsID::SphereCollider:
			Temp->AddComponent<SphereCollider>().m_IsDirty = true;
			ECSSystemManager::Instance().GetSystem<PhysicsSystem>()->ConstructSphereCollider(Temp);
			TRE_INFO("Sphere Collider added to {0}({1})", Temp->GetName(), Temp->GetGUID());
			break;
		case ComponentsID::CapsuleCollider:
			Temp->AddComponent<CapsuleCollider>().m_IsDirty = true;
			ECSSystemManager::Instance().GetSystem<PhysicsSystem>()->ConstructCapsuleCollider(Temp);
			TRE_INFO("Capsule Collider added to {0}({1})", Temp->GetName(), Temp->GetGUID());
			break;
		case ComponentsID::CylinderCollider:
			Temp->AddComponent<CylinderCollider>().m_IsDirty = true;
			ECSSystemManager::Instance().GetSystem<PhysicsSystem>()->ConstructCylinderCollider(Temp);
			TRE_INFO("Cylinder Collider added to {0}({1})", Temp->GetName(), Temp->GetGUID());
			break;
		case ComponentsID::SpriteRenderer:
			Temp->AddComponent<UIComponent>();
			TRE_INFO("Sprite Renderer (UI Component) added to {0}({1})", Temp->GetName(), Temp->GetGUID());
			break;
		case ComponentsID::Parenting:
			Temp->AddComponent<Parenting>();
			TRE_INFO("Parenting added to {0}({1})", Temp->GetName(), Temp->GetGUID());
			break;
		case ComponentsID::Animation:
			Temp->AddComponent<AnimationComponent>();
			TRE_INFO("Animation added to {0}({1})", Temp->GetName(), Temp->GetGUID());
			break;
		case ComponentsID::DirectPathfinding:
			Temp->AddComponent<DirectPathfinding>();
			TRE_INFO("DirectPathfinding added to {0}({1})", Temp->GetName(), Temp->GetGUID());
			break;
		case ComponentsID::Text:
			Temp->AddComponent<TextComponent>();
			TRE_INFO("TextComponent added to {0}({1})", Temp->GetName(), Temp->GetGUID());
			break;
		case ComponentsID::Particle:
			Temp->AddComponent<ParticleComponent>();
			TRE_INFO("Particle Component added to {0}({1})", Temp->GetName(), Temp->GetGUID());
			break;
		default:
			std::cout << "The component does not exist!" << std::endl;
			break;
		}

	}

	static void BindRemoveComponent(CSEntityID ID, MonoReflectionType* componenttype)
	{
		Entity Temp = VALIDATEENTITY(ID);
		if (!Temp) return;

		MonoType* componentType = mono_reflection_type_get_type(componenttype);
		ComponentsID componentID = Tools::ConvertComponentNameToID(mono_type_get_name(componentType));

		switch (componentID)
		{
		case ComponentsID::MeshRenderer: // mesh
			Temp->RemoveComponent<MeshRenderer>();
			TRE_INFO("MeshRenderer Removed From {0}({1})", Temp->GetName(), Temp->GetGUID());
			break;
		case ComponentsID::Camera:
			Temp->RemoveComponent<Camera>();
			TRE_INFO("Camera Removed From {0}({1})", Temp->GetName(), Temp->GetGUID());
			break;
		case ComponentsID::Audio:
			Temp->RemoveComponent<Audio>();
			TRE_INFO("Audio Removed From {0}({1})", Temp->GetName(), Temp->GetGUID());
			break;
		case ComponentsID::Rigidbody:
			Temp->RemoveComponent<Rigidbody>();
			ECSSystemManager::Instance().GetSystem<PhysicsSystem>()->DestructRigidbody(Temp);
			TRE_INFO("Rigid Body Removed From {0}({1})", Temp->GetName(), Temp->GetGUID());
			break;
		case ComponentsID::BoxCollider:
			Temp->RemoveComponent<BoxCollider>();
			ECSSystemManager::Instance().GetSystem<PhysicsSystem>()->DestructBoxCollider(Temp);
			TRE_INFO("Box Collider Removed From {0}({1})", Temp->GetName(), Temp->GetGUID());
			break;
		case ComponentsID::SphereCollider:
			Temp->RemoveComponent<SphereCollider>();
			ECSSystemManager::Instance().GetSystem<PhysicsSystem>()->DestructSphereCollider(Temp);
			TRE_INFO("Sphere Collider Removed From {0}({1})", Temp->GetName(), Temp->GetGUID());
			break;
		case ComponentsID::CapsuleCollider:
			Temp->RemoveComponent<CapsuleCollider>();
			ECSSystemManager::Instance().GetSystem<PhysicsSystem>()->DestructCapsuleCollider(Temp);
			TRE_INFO("Capsule Collider Removed From {0}({1})", Temp->GetName(), Temp->GetGUID());
			break;
		case ComponentsID::CylinderCollider:
			Temp->RemoveComponent<CylinderCollider>();
			ECSSystemManager::Instance().GetSystem<PhysicsSystem>()->DestructCylinderCollider(Temp);
			TRE_INFO("Cylinder Collider Removed From {0}({1})", Temp->GetName(), Temp->GetGUID());
			break;
		case ComponentsID::SpriteRenderer:
			Temp->RemoveComponent<UIComponent>();
			TRE_INFO("Sprite Renderer (UI Component) Removed From {0}({1})", Temp->GetName(), Temp->GetGUID());
			break;
		case ComponentsID::Parenting:
			Temp->RemoveComponent<Parenting>();
			TRE_INFO("Parenting (UI Component) Removed From {0}({1})", Temp->GetName(), Temp->GetGUID());
			break;
		case ComponentsID::Animation:
			Temp->RemoveComponent<AnimationComponent>();
			TRE_INFO("Animation Removed From {0}({1})", Temp->GetName(), Temp->GetGUID());
			break;
		case ComponentsID::DirectPathfinding:
			Temp->RemoveComponent<DirectPathfinding>();
			TRE_INFO("DirectPathfinding Removed From {0}({1})", Temp->GetName(), Temp->GetGUID());
			break;
		case ComponentsID::Text:
			Temp->RemoveComponent<TextComponent>();
			TRE_INFO("TextComponent Removed From {0}({1})", Temp->GetName(), Temp->GetGUID());
			break;
		case ComponentsID::Particle:
			Temp->RemoveComponent<ParticleComponent>();
			TRE_INFO("ParticleComponent Removed From {0}({1})", Temp->GetName(), Temp->GetGUID());
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
		ComponentsID componentID = Tools::ConvertComponentNameToID(mono_type_get_name(monoType));

		switch (componentID)
		{
		case ComponentsID::Transform:
			return entity->HasComponent<Transform>();
		case ComponentsID::MeshRenderer:
			return entity->HasComponent<MeshRenderer>();
		case ComponentsID::Camera:
			return entity->HasComponent<Camera>();
		case ComponentsID::Rigidbody:
			return entity->HasComponent<Rigidbody>();
		case ComponentsID::BoxCollider:
			return entity->HasComponent<BoxCollider>();
		case ComponentsID::SphereCollider:
			return entity->HasComponent<SphereCollider>();
		case ComponentsID::CapsuleCollider:
			return entity->HasComponent<CapsuleCollider>();
		case ComponentsID::CylinderCollider:
			return entity->HasComponent<CylinderCollider>();
		case ComponentsID::Audio:
			return entity->HasComponent<Audio>();
		case ComponentsID::SpriteRenderer:
			return entity->HasComponent<UIComponent>();
		case ComponentsID::Parenting:
			return entity->HasComponent<Parenting>();
		case ComponentsID::Animation:
			return entity->HasComponent<AnimationComponent>();
		case ComponentsID::DirectPathfinding:
			return entity->HasComponent<DirectPathfinding>();
		case ComponentsID::Text:
			return entity->HasComponent<TextComponent>();
		case ComponentsID::Particle:
			return entity->HasComponent<ParticleComponent>();
		default:
			TRE_ERROR("Component does not exist!");
			return false;
		}
	}

	static std::unordered_map<std::string, std::string> GetAllSceneObjects()
	{
		std::unordered_map<std::string, std::string > tempMap;
		std::vector temp = ECSManager::Instance().GetAllEntities(true);

		for (auto& entity : temp)
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
		if (sceneObjects.find(temp) != sceneObjects.end())
		{
			// Found the name
			std::string ID = sceneObjects[temp];
			TRE_CORE_INFO("{0} : {1}", temp, ID);
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

	static CSEntityID FindParentIDFromID(CSEntityID ID)
	{
		Entity Temp = VALIDATEENTITY(ID);
		if (!Temp) return CSEntityID();

		return  EntityID_EngineToCS(Temp->GetComponent<Parenting>().m_Parent);
	}

#pragma region TransformBindings

	static glm::vec3 BindRotateVector(glm::vec3 vector, glm::vec3 rotation)
	{
		return ECSSystemManager::Instance().GetSystem<TransformSystem>()->RotateMatrix(vector, rotation);
	}

	static void BindSetPosition(CSEntityID ID, glm::vec3 newPos)
	{
		Entity Temp = VALIDATEENTITY(ID);
		if (!Temp) return;

		Transform& transform = Temp->GetComponent<Transform>();
		transform.m_Position = newPos;
		transform.m_IsDirty = true;
		transform.m_DirtyFlags = TransformDirtyFlags::TRE_DIRTY_POSITION;
	}

	static void BindSetRotation(CSEntityID ID, glm::vec3 newRot)
	{
		Entity Temp = VALIDATEENTITY(ID);
		if (!Temp) return;

		Transform& transform = Temp->GetComponent<Transform>();
		transform.m_Rotation = newRot;
		transform.m_IsDirty = true;
		transform.m_DirtyFlags = TransformDirtyFlags::TRE_DIRTY_ROTATION;
	}

	static void BindSetScaling(CSEntityID ID, glm::vec3 newSca)
	{
		Entity Temp = VALIDATEENTITY(ID);
		if (!Temp) return;

		Transform& transform = Temp->GetComponent<Transform>();
		transform.m_Scale = newSca;
		transform.m_IsDirty = true;
		transform.m_DirtyFlags = TransformDirtyFlags::TRE_DIRTY_SCALE;
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

#pragma region MeshRendererBindings
	static void BindSetMaterialInstance(CSEntityID ID, MonoString* materialInstanceName)
	{
		Entity Temp = VALIDATEENTITY(ID);
		if (!Temp) return;

		if (!Temp->HasComponent<MeshRenderer>())
		{
			PUBLISHERROR("Entity " + Temp->GetName() + " does not have MeshRenderer!");
			return;
		}

		std::string str = MonoStringToString(materialInstanceName);

		MeshRenderer& mr = Temp->GetComponent<MeshRenderer>();
		mr.m_MaterialInstance = ResourceManager::Instance().GetResource<Material>(str);
		mr.m_IsDirty = true;

		if (mr.m_MaterialInstance == nullptr)
			PUBLISHERROR("Unable to find material " + str);
	}

	/*static MonoString* BindGetMaterialInstance(CSEntityID ID)
	{
		Entity Temp = VALIDATEENTITY(ID);
		if (!Temp) return mono_string_new(mono_domain_get(), "");
		if (!Temp->HasComponent<MeshRenderer>())
		{
			PUBLISHERROR("Entity " + Temp->GetName() + " does not have MeshRenderer!");
			return mono_string_new(mono_domain_get(), "");
		}
		MeshRenderer& mr = Temp->GetComponent<MeshRenderer>();
		if (mr.m_MaterialInstance == nullptr)
			return mono_string_new(mono_domain_get(), "");
		return mono_string_new(mono_domain_get(), mr.m_MaterialInstance->GetHandleHex().c_str());
	}*/

	static void BindSetAnimMaterialInstance(CSEntityID ID, MonoString* animMaterialInstanceName)
	{
		Entity Temp = VALIDATEENTITY(ID);
		if (!Temp) return;

		if (!Temp->HasComponent<MeshRenderer>())
		{
			PUBLISHERROR("Entity " + Temp->GetName() + " does not have MeshRenderer!");
			return;
		}

		std::string str = MonoStringToString(animMaterialInstanceName);

		MeshRenderer& mr = Temp->GetComponent<MeshRenderer>();
		mr.m_AnimationMaterialInstance = ResourceManager::Instance().GetResource<Material>(str);
		mr.m_IsDirty = true;

		if (mr.m_AnimationMaterialInstance == nullptr)
			PUBLISHERROR("Unable to find animation material " + str);
	}

	/*static MonoString* BindGetAnimMaterialInstance(CSEntityID ID)
	{
		Entity Temp = VALIDATEENTITY(ID);
		if (!Temp) return mono_string_new(mono_domain_get(), "");
		if (!Temp->HasComponent<MeshRenderer>())
		{
			PUBLISHERROR("Entity " + Temp->GetName() + " does not have MeshRenderer!");
			return mono_string_new(mono_domain_get(), "");
		}
		MeshRenderer& mr = Temp->GetComponent<MeshRenderer>();
		if (mr.m_AnimationMaterialInstance == nullptr)
			return mono_string_new(mono_domain_get(), "");
		return mono_string_new(mono_domain_get(), mr.m_AnimationMaterialInstance->GetHandleHex().c_str());
	}*/

	static void BindSetMeshVisibility(CSEntityID ID, bool isVisible)
	{
		Entity Temp = VALIDATEENTITY(ID);
		if (!Temp) return;

		if (!Temp->HasComponent<MeshRenderer>())
		{
			PUBLISHERROR("Entity " + Temp->GetName() + " does not have MeshRenderer!");
			return;
		}

		MeshRenderer& render = Temp->GetComponent<MeshRenderer>();
		// Ignores if the same value
		if (render.m_IsVisible == isVisible) return;

		render.m_IsVisible = isVisible;
		render.m_IsDirty = true;
	}

	static bool BindGetMeshVisibility(CSEntityID ID)
	{
		Entity Temp = VALIDATEENTITY(ID);
		if (!Temp) return false;

		if (!Temp->HasComponent<MeshRenderer>())
		{
			PUBLISHERROR("Entity " + Temp->GetName() + " does not have MeshRenderer!");
			return false;
		}

		return Temp->GetComponent<MeshRenderer>().m_IsVisible;
	}

	static void BindSetMesh(CSEntityID ID, MonoString* meshName)
	{
		Entity Temp = VALIDATEENTITY(ID);
		if (!Temp) return;

		if (!Temp->HasComponent<MeshRenderer>())
		{
			PUBLISHERROR("Entity " + Temp->GetName() + " does not have MeshRenderer!");
			return;
		}

		std::string str = MonoStringToString(meshName);

		MeshRenderer& mr = Temp->GetComponent<MeshRenderer>();
		mr.m_RenderObject = ResourceManager::Instance().GetResource<RenderObject>(str);
		mr.m_IsDirty = true;

		if (mr.m_RenderObject->IsRigged())
		{
			if (Temp->HasComponent<AnimationComponent>() == false)
			{
				Temp->AddComponent<AnimationComponent>();
			}
		}
		else
		{
			if (Temp->HasComponent<AnimationComponent>())
			{
				Temp->RemoveComponent<AnimationComponent>();
			}
		}

		if (mr.m_RenderObject == nullptr)
			PUBLISHERROR("Unable to find mesh " + str);
	}

	/*static MonoString* BindGetMesh(CSEntityID ID)
	{
		Entity Temp = VALIDATEENTITY(ID);
		if (!Temp) return mono_string_new(mono_domain_get(), "");
		if (!Temp->HasComponent<MeshRenderer>())
		{
			PUBLISHERROR("Entity " + Temp->GetName() + " does not have MeshRenderer!");
			return mono_string_new(mono_domain_get(), "");
		}
		MeshRenderer& mr = Temp->GetComponent<MeshRenderer>();
		if (mr.m_RenderObject == nullptr)
			return mono_string_new(mono_domain_get(), "");
		return mono_string_new(mono_domain_get(), mr.m_RenderObject->GetHandleHex().c_str());
	}*/

	static bool BindIsMesh(CSEntityID ID, MonoString* meshName)
	{
		Entity Temp = VALIDATEENTITY(ID);
		if (!Temp) return false;
		if (!Temp->HasComponent<MeshRenderer>())
		{
			PUBLISHERROR("Entity " + Temp->GetName() + " does not have MeshRenderer!");
			return false;
		}

		const ResourceHandle handle = Resource::GenerateGUID(MonoStringToString(meshName));

		MeshRenderer& mr = Temp->GetComponent<MeshRenderer>();
		if (mr.m_RenderObject == nullptr)
			return false;
		return mr.m_RenderObject->GetHandle() == handle;
	}

	static bool BindIsMaterialInstance(CSEntityID ID, MonoString* materialInstanceName)
	{
		Entity Temp = VALIDATEENTITY(ID);
		if (!Temp) return false;
		if (!Temp->HasComponent<MeshRenderer>())
		{
			PUBLISHERROR("Entity " + Temp->GetName() + " does not have MeshRenderer!");
			return false;
		}

		const ResourceHandle handle = Resource::GenerateGUID(MonoStringToString(materialInstanceName));

		MeshRenderer& mr = Temp->GetComponent<MeshRenderer>();
		if (mr.m_MaterialInstance == nullptr)
			return false;
		return mr.m_MaterialInstance->GetHandle() == handle;
	}

	static bool BindIsAnimMaterialInstance(CSEntityID ID, MonoString* animMaterialInstanceName)
	{
		Entity Temp = VALIDATEENTITY(ID);
		if (!Temp) return false;
		if (!Temp->HasComponent<MeshRenderer>())
		{
			PUBLISHERROR("Entity " + Temp->GetName() + " does not have MeshRenderer!");
			return false;
		}

		const ResourceHandle handle = Resource::GenerateGUID(MonoStringToString(animMaterialInstanceName));

		MeshRenderer& mr = Temp->GetComponent<MeshRenderer>();
		if (mr.m_AnimationMaterialInstance == nullptr)
			return false;
		return mr.m_AnimationMaterialInstance->GetHandle() == handle;
	}

#pragma endregion

#pragma region Animation

	static void BindSetAnimationSpeed(CSEntityID ID, float animationSpeed)
	{
		Entity Temp = VALIDATEENTITY(ID);
		if (!Temp) return;

		if (!Temp->HasComponent<AnimationComponent>())
		{
			PUBLISHERROR("Entity " + Temp->GetName() + " does not have Animation Component!");
			return;
		}

		Temp->GetComponent<AnimationComponent>().m_AnimationSpeed = animationSpeed;
	}

	static float BindGetAnimationSpeed(CSEntityID ID)
	{
		Entity Temp = VALIDATEENTITY(ID);
		if (!Temp) return 0.0f;
		if (!Temp->HasComponent<AnimationComponent>())
		{
			PUBLISHERROR("Entity " + Temp->GetName() + " does not have Animation Component!");
			return 0.0f;
		}
		return Temp->GetComponent<AnimationComponent>().m_AnimationSpeed;
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

	static void BindCamMainSetLookAt(glm::vec3* target)
	{
		ECSSystemManager::Instance().GetSystem<CameraSystem>()->MainCameraLookAt(*target);
	}

	static void BindCamMainSetFollow(glm::vec3* target, float distance)
	{
		ECSSystemManager::Instance().GetSystem<CameraSystem>()->MainCameraFollow(*target, distance);
	}

	static void BindTransitionMainCamera(glm::vec3* targetPosition, glm::vec3* targetRotation, float duration)
	{
		ECSSystemManager::Instance().GetSystem<CameraSystem>()->TransitionCamera(*targetPosition, *targetRotation, duration);
	}

	static void BindTransitionMainCameraPosition(glm::vec3* targetPosition, float duration)
	{
		ECSSystemManager::Instance().GetSystem<CameraSystem>()->TransitionCameraPosition(*targetPosition, duration);
	}

	static void BindTransitionMainCameraRotation(glm::vec3* targetRotation, float duration)
	{
		ECSSystemManager::Instance().GetSystem<CameraSystem>()->TransitionCameraRotation(*targetRotation, duration);
	}

	static Vector3 BindGetMainCameraPosition()
	{
		//return ECSSystemManager::Instance().GetSystem<CameraSystem>()->GetMainCamera()->GetComponent<Transform>().m_Position;
		const BaseCamera& camera = ECSSystemManager::Instance().GetSystem<CameraSystem>()->GetMainCamera()->GetComponent<Camera>().m_BaseCamera;
		return camera.m_FocalPoint - camera.GetViewDirection() * camera.m_FocalLength;
	}

	static Vector3 BindCameraForwardVector()
	{
		glm::vec3 fwd = ECSSystemManager::Instance().GetSystem<CameraSystem>()->GetMainCamera()->GetComponent<Camera>().m_BaseCamera.GetViewDirection();
		return Vector3(fwd.x, fwd.y, fwd.z);
	}

	static Vector3 BindCameraRightVector()
	{
		glm::vec3 right = ECSSystemManager::Instance().GetSystem<CameraSystem>()->GetMainCamera()->GetComponent<Camera>().m_BaseCamera.GetRightVec();
		return Vector3(right.x, right.y, right.z);
	}

	static Vector3 BindCameraRotation()
	{
		glm::vec3 rotation = ECSSystemManager::Instance().GetSystem<CameraSystem>()->GetMainCamera()->GetComponent<Transform>().m_Rotation;
		return Vector3(rotation.x, rotation.y, rotation.z);
	}

	static void BindSetMainCameraPosition(glm::vec3* position)
	{
		ECSSystemManager::Instance().GetSystem<CameraSystem>()->GetMainCamera()->GetComponent<Transform>().m_Position = *position;
	}

#pragma endregion

#pragma region InputBindings

	ScriptInputHandler& ScriptInputHandler::Instance()
	{
		static ScriptInputHandler instance;
		return instance;
	}

	void ScriptInputHandler::GetKeyPressed(const InputEvent& event)
	{
		_key = event._key;
		_state = event._state;
	}

	static bool GetKeyHold(int key)
	{
		return InputHandler::GetKeyHold(key);
	}

	static bool GetKeyPress(int key)
	{
		return InputHandler::GetKeyPress(key);
	}

	static bool GetKeyRelease(int key)
	{
		return InputHandler::GetKeyRelease(key);
	}

#pragma endregion

#pragma region Logging
	static void SendMessageToConsole(MonoString* message)
	{
		std::string str = MonoStringToString(message);

		EventHandler::getEventHandlerInstance().Publish(ConsoleDebugEvent{ str.c_str() });
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

	static float BindGetSphereColliderRadius(CSEntityID ID)
	{
		const Entity& entity = VALIDATEENTITY(ID);
		if (!entity) return {};

		if (!entity->HasComponent<SphereCollider>())
		{
			PUBLISHERROR("There is no SphereCollider in " + entity->GetName() + "!");
			return {};
		}

		return entity->GetComponent<SphereCollider>().m_Radius;
	}

	static void Engine_SetSphereTrigger(CSEntityID ID, bool isTrigger)
	{
		const Entity& entity = VALIDATEENTITY(ID);
		if (!entity) return;

		if (!entity->HasComponent<SphereCollider>())
		{
			PUBLISHERROR("There is no SphereCollider in " + entity->GetName() + "!");
			return;
		}

		ECSSystemManager::Instance().GetSystem<PhysicsSystem>()->SetSphereColliderTrigger(entity, isTrigger);
	}

	static bool Engine_GetSphereTrigger(CSEntityID ID)
	{
		const Entity& entity = VALIDATEENTITY(ID);
		if (!entity) return false;

		if (!entity->HasComponent<SphereCollider>())
		{
			PUBLISHERROR("There is no SphereCollider in " + entity->GetName() + "!");
			return false;
		}

		return entity->GetComponent<SphereCollider>().m_IsTrigger;
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

	static glm::vec3 BindGetBoxColliderHalfExtents(CSEntityID ID)
	{
		const Entity& entity = VALIDATEENTITY(ID);
		if (!entity) return {};

		if (!entity->HasComponent<BoxCollider>())
		{
			PUBLISHERROR("There is no BoxCollider in " + entity->GetName() + "!");
			return {};
		}

		return entity->GetComponent<BoxCollider>().m_HalfExtents;
	}

	static void Engine_SetBoxTrigger(CSEntityID ID, bool isTrigger)
	{
		const Entity& entity = VALIDATEENTITY(ID);
		if (!entity) return;

		if (!entity->HasComponent<BoxCollider>())
		{
			PUBLISHERROR("There is no BoxCollider in " + entity->GetName() + "!");
			return;
		}

		ECSSystemManager::Instance().GetSystem<PhysicsSystem>()->SetBoxColliderTrigger(entity, isTrigger);
	}

	static bool Engine_GetBoxTrigger(CSEntityID ID)
	{
		const Entity& entity = VALIDATEENTITY(ID);
		if (!entity) return false;

		if (!entity->HasComponent<BoxCollider>())
		{
			PUBLISHERROR("There is no BoxCollider in " + entity->GetName() + "!");
			return false;
		}

		return entity->GetComponent<BoxCollider>().m_IsTrigger;
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

	static float BindGetCapsuleColliderRadius(CSEntityID ID)
	{
		const Entity& entity = VALIDATEENTITY(ID);
		if (!entity) return {};

		if (!entity->HasComponent<CapsuleCollider>())
		{
			PUBLISHERROR("There is no CapsuleCollider in " + entity->GetName() + "!");
			return {};
		}

		return entity->GetComponent<CapsuleCollider>().m_Radius;
	}

	static float BindGetCapsuleColliderHalfHeight(CSEntityID ID)
	{
		const Entity& entity = VALIDATEENTITY(ID);
		if (!entity) return {};

		if (!entity->HasComponent<CapsuleCollider>())
		{
			PUBLISHERROR("There is no CapsuleCollider in " + entity->GetName() + "!");
			return {};
		}

		return entity->GetComponent<CapsuleCollider>().m_HalfHeight;
	}

	static void Engine_SetCapsuleTrigger(CSEntityID ID, bool isTrigger)
	{
		const Entity& entity = VALIDATEENTITY(ID);
		if (!entity) return;

		if (!entity->HasComponent<CapsuleCollider>())
		{
			PUBLISHERROR("There is no CapsuleCollider in " + entity->GetName() + "!");
			return;
		}

		ECSSystemManager::Instance().GetSystem<PhysicsSystem>()->SetCapsuleColliderTrigger(entity, isTrigger);
	}

	static bool Engine_GetCapsuleTrigger(CSEntityID ID)
	{
		const Entity& entity = VALIDATEENTITY(ID);
		if (!entity) return false;

		if (!entity->HasComponent<CapsuleCollider>())
		{
			PUBLISHERROR("There is no CapsuleCollider in " + entity->GetName() + "!");
			return false;
		}

		return entity->GetComponent<CapsuleCollider>().m_IsTrigger;
	}

	static void BindResizeCylinderCollider(CSEntityID ID, float radius, float height)
	{
		const Entity& entity = VALIDATEENTITY(ID);
		if (!entity) return;

		if (!entity->HasComponent<CylinderCollider>())
		{
			PUBLISHERROR("There is no CylinderCollider in " + entity->GetName() + "!");
			return;
		}

		ECSSystemManager::Instance().GetSystem<PhysicsSystem>()->ResizeCylinderCollider(entity, radius, height);
	}

	static float BindGetCylinderColliderRadius(CSEntityID ID)
	{
		const Entity& entity = VALIDATEENTITY(ID);
		if (!entity) return {};

		if (!entity->HasComponent<CylinderCollider>())
		{
			PUBLISHERROR("There is no CylinderCollider in " + entity->GetName() + "!");
			return {};
		}

		return entity->GetComponent<CylinderCollider>().m_Radius;
	}

	static float BindGetCylinderColliderHeight(CSEntityID ID)
	{
		const Entity& entity = VALIDATEENTITY(ID);
		if (!entity) return {};

		if (!entity->HasComponent<CylinderCollider>())
		{
			PUBLISHERROR("There is no CylinderCollider in " + entity->GetName() + "!");
			return {};
		}

		return entity->GetComponent<CylinderCollider>().m_Height;
	}

	static void Engine_SetCylinderTrigger(CSEntityID ID, bool isTrigger)
	{
		const Entity& entity = VALIDATEENTITY(ID);
		if (!entity) return;

		if (!entity->HasComponent<CylinderCollider>())
		{
			PUBLISHERROR("There is no CylinderCollider in " + entity->GetName() + "!");
			return;
		}

		ECSSystemManager::Instance().GetSystem<PhysicsSystem>()->SetCylinderColliderTrigger(entity, isTrigger);
	}

	static bool Engine_GetCylinderTrigger(CSEntityID ID)
	{
		const Entity& entity = VALIDATEENTITY(ID);
		if (!entity) return false;

		if (!entity->HasComponent<CylinderCollider>())
		{
			PUBLISHERROR("There is no CylinderCollider in " + entity->GetName() + "!");
			return false;
		}

		return entity->GetComponent<CylinderCollider>().m_IsTrigger;
	}

	static void BindOffsetCollider(CSEntityID ID, Vector3 offset)
	{
		const Entity& entity = VALIDATEENTITY(ID);
		if (!entity) return;

		bool hasCollider = entity->HasComponent<BoxCollider>() || entity->HasComponent<SphereCollider>() ||
			entity->HasComponent<CapsuleCollider>() || entity->HasComponent<CylinderCollider>();
		if (!hasCollider)
		{
			PUBLISHERROR("There is no Collider in " + entity->GetName() + "!");
			return;
		}

		ECSSystemManager::Instance().GetSystem<PhysicsSystem>()->UpdateColliderData(entity, offset);
	}

	static Vector3 BindGetOffsetCollider(CSEntityID ID)
	{
		const Entity& entity = VALIDATEENTITY(ID);
		if (!entity) return {};

		bool hasCollider = entity->HasComponent<BoxCollider>() || entity->HasComponent<SphereCollider>() ||
			entity->HasComponent<CapsuleCollider>() || entity->HasComponent<CylinderCollider>();
		if (!hasCollider)
		{
			PUBLISHERROR("There is no Collider in " + entity->GetName() + "!");
			return {};
		}

		if (entity->HasComponent<CapsuleCollider>()) return entity->GetComponent<CapsuleCollider>().m_Offset;
		if (entity->HasComponent<CylinderCollider>()) return entity->GetComponent<CylinderCollider>().m_Offset;
		if (entity->HasComponent<SphereCollider>()) return entity->GetComponent<SphereCollider>().m_Offset;
		return entity->GetComponent<BoxCollider>().m_Offset;
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

	static void BindConstrainRotationX(CSEntityID ID, bool state)
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

	static void BindConstrainRotationY(CSEntityID ID, bool state)
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

	static void BindConstrainRotationZ(CSEntityID ID, bool state)
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

	static void BindSetIsActive(CSEntityID ID, bool isActive)
	{
		const Entity& entity = VALIDATEENTITY(ID);
		if (!entity) return;

		// ensure that there is an existing collider on this entity
		bool hasCollider = entity->HasComponent<BoxCollider>() || entity->HasComponent<SphereCollider>() ||
			entity->HasComponent<CapsuleCollider>() || entity->HasComponent<CylinderCollider>();
		if (!hasCollider)
		{
			PUBLISHERROR("There is no collider component in " + entity->GetName() + "!");
			return;
		}

		ECSSystemManager::Instance().GetSystem<PhysicsSystem>()->SetIsActive(entity, isActive);
	}

	static bool BindGetIsActive(CSEntityID ID)
	{
		const Entity& entity = VALIDATEENTITY(ID);
		if (!entity) return false;

		// ensure that there is an existing collider on this entity
		bool hasCollider = entity->HasComponent<BoxCollider>() || entity->HasComponent<SphereCollider>() ||
			entity->HasComponent<CapsuleCollider>() || entity->HasComponent<CylinderCollider>();
		if (!hasCollider)
		{
			PUBLISHERROR("There is no collider component in " + entity->GetName() + "!");
			return false;
		}

		return ECSSystemManager::Instance().GetSystem<PhysicsSystem>()->GetIsActive(entity);
	}

	static void BindSetPauseState(bool state)
	{
		ECSSystemManager::Instance().GetSystem<PhysicsSystem>()->SetPauseState(state);
	}

	static bool BindGetPauseState()
	{
		return ECSSystemManager::Instance().GetSystem<PhysicsSystem>()->GetPauseState();
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

	static bool BindGetKinematic(CSEntityID ID)
	{
		Entity Temp = VALIDATEENTITY(ID);
		if (!Temp) return false;

		return Temp->GetComponent<Rigidbody>().m_IsKinematic;
	}

	static void BindSetGravity(CSEntityID ID, bool enable)
	{
		Entity Temp = VALIDATEENTITY(ID);
		if (!Temp) return;

		Temp->GetComponent<Rigidbody>().m_UseGravity = enable;
		if (!enable) BindSetLinearVelocity(ID, {});
		Temp->GetComponent<Rigidbody>().m_IsDirty = true;
	}

	static bool BindGetGravity(CSEntityID ID)
	{
		Entity Temp = VALIDATEENTITY(ID);
		if (!Temp) return false;

		return Temp->GetComponent<Rigidbody>().m_UseGravity;
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

	static float BindLerp(float a, float b, float t)
	{
		return Mathf::LerpUnclamped(a, b, t);
	}

	static Vector3 BindLerpVec3(Vector3 a, Vector3 b, float t)
	{
		return Vector3(Mathf::LerpUnclamped(a.x, b.x, t), Mathf::LerpUnclamped(a.y, b.y, t), Mathf::LerpUnclamped(a.z, b.z, t));
	}

	static float BindSin(float value)
	{
		return Mathf::Sin(value);
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

	static void BindSetPlaySound(CSEntityID ID)
	{
		Entity entity = VALIDATEENTITY(ID);
		if (!entity) return;

		if (!entity->HasComponent<Audio>())
		{
			PUBLISHERROR("Entity " + entity->GetName() + " has no Audio Component!");
			return;
		}

		ECSSystemManager::Instance().GetSystem<AudioSystem>()->Play(entity, true);
		entity->GetComponent<Audio>().m_Play = true;
	}

	static void BindTogglePauseSound(CSEntityID ID, bool paused)
	{
		(void)paused;
		Entity entity = VALIDATEENTITY(ID);
		if (!entity) return;

		if (!entity->HasComponent<Audio>())
		{
			PUBLISHERROR("Entity " + entity->GetName() + " has no Audio Component!");
			return;
		}

		ECSSystemManager::Instance().GetSystem<AudioSystem>()->TogglePause(entity);
	}

	static void BindSetStop(CSEntityID ID)
	{
		Entity entity = VALIDATEENTITY(ID);
		if (!entity) return;

		if (!entity->HasComponent<Audio>())
		{
			PUBLISHERROR("Entity " + entity->GetName() + " has no Audio Component!");
			return;
		}

		ECSSystemManager::Instance().GetSystem<AudioSystem>()->Stop(entity);
	}

	static void BindSetFileName(CSEntityID ID, MonoString* fileName)
	{
		Entity entity = VALIDATEENTITY(ID);
		if (!entity) return;

		if (!entity->HasComponent<Audio>())
		{
			PUBLISHERROR("Entity " + entity->GetName() + " has no Audio Component!");
			return;
		}

		std::string str = MonoStringToString(fileName);

		ECSSystemManager::Instance().GetSystem<AudioSystem>()->SetFileName(entity, str);
	}

	static MonoString* BindGetFileName(CSEntityID ID)
	{
		Entity entity = VALIDATEENTITY(ID);
		if (!entity) return mono_string_new(mono_domain_get(), "");

		if (!entity->HasComponent<Audio>())
		{
			PUBLISHERROR("Entity " + entity->GetName() + " has no Audio Component!");
			return mono_string_new(mono_domain_get(), "");
		}

		return mono_string_new(mono_domain_get(), ECSSystemManager::Instance().GetSystem<AudioSystem>()->GetFileName(entity).c_str());
	}

	static bool BindIsPlaying(CSEntityID ID)
	{
		Entity entity = VALIDATEENTITY(ID);
		if (!entity) return false;

		if (!entity->HasComponent<Audio>())
		{
			PUBLISHERROR("Entity " + entity->GetName() + " has no Audio Component!");
			return false;
		}

		return ECSSystemManager::Instance().GetSystem<AudioSystem>()->GetIsPlaying(entity);
	}

#pragma endregion

#pragma region SceneBindings
	static void BindLoadScene(MonoString* id)
	{
		std::string sceneName = MonoStringToString(id);
		std::string scenePath = GETFOLDER(FILESYS_SCENE) + sceneName + GETFILE(FILESYS_SCENE);
		SceneManager::Instance().LoadScene(scenePath);
	}

	static void BindTransitionScene(MonoString* id, float totalDuration)
	{
		std::string sceneName = MonoStringToString(id);
		std::string scenePath = GETFOLDER(FILESYS_SCENE) + sceneName + GETFILE(FILESYS_SCENE);
		ECSSystemManager::Instance().GetSystem<ScenePostEffectsSystem>()->TransitionToScene(scenePath, totalDuration);
	}

	static MonoString* BindGetSceneName()
	{
		std::string sceneName = SceneManager::Instance().GetCurrentSceneName();
		return mono_string_new(mono_domain_get(), sceneName.c_str());
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

	// redo this code to allow for the searching of the script instance
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

		for (auto i : Temp->GetComponent<ScriptComponent>().m_RegisteredScripts)
		{
			if (i == classNameStr)
			{
				TRE_CORE_INFO("Found script with name {0} in entity {1}", classNameStr, Temp->GetName());
				return true;
			}
		}

		return false;

	}

	static MonoObject* BindGetScript(CSEntityID ID, MonoString* className)
	{
		if (!BindHaveScript(ID, className))
		{
			std::string error{  };
			error = "ID " + std::to_string(ID) + " does not have script " + MonoStringToString(className);
			PUBLISHERROR(error.c_str());
			return NULL;
		}

		std::string IDStr{ EntityID_CSToEngine(ID) };
		std::string classNameStr{ MonoStringToString(className) };

		auto instances = ScriptEngine::GetAllEntityScripts(IDStr);

		for (auto i : instances)
		{
			std::string temp = "TRE." + i->GetScriptClass()->GetScriptClassName();
			if (temp == classNameStr)
			{
				//TRE_CORE_INFO("Found script with name {0} in entity {1}", classNameStr, IDStr);
				return i->GetScriptObject();
			}
		}

		// Script is not found
		TRE_CORE_WARN("Script with name {0} does not exist in entity {1}", classNameStr, IDStr);
	}
#pragma endregion

#pragma region SpriteBindings
	static void BindSpriteSetVisible(CSEntityID ID, bool isVisible)
	{
		Entity Temp{ VALIDATEENTITY(ID) };
		if (!Temp) return;
		if (!Temp->HasComponent<UIComponent>()) return;
		Temp->GetComponent<UIComponent>().m_IsVisible = isVisible;
	}

	static bool BindSpriteGetVisible(CSEntityID ID)
	{
		Entity Temp{ VALIDATEENTITY(ID) };
		if (!Temp) return false;
		if (!Temp->HasComponent<UIComponent>()) return false;
		return Temp->GetComponent<UIComponent>().m_IsVisible;
	}

	static void BindSpriteSetWidth(CSEntityID ID, int width)
	{
		Entity Temp{ VALIDATEENTITY(ID) };
		if (!Temp) return;
		if (!Temp->HasComponent<UIComponent>()) return;
		Temp->GetComponent<UIComponent>().m_Width = width;
	}

	static int BindSpriteGetWidth(CSEntityID ID)
	{
		Entity Temp{ VALIDATEENTITY(ID) };
		if (!Temp) return 0;
		if (!Temp->HasComponent<UIComponent>()) return 0;
		return Temp->GetComponent<UIComponent>().m_Width;
	}

	static void BindSpriteSetHeight(CSEntityID ID, int height)
	{
		Entity Temp{ VALIDATEENTITY(ID) };
		if (!Temp) return;
		if (!Temp->HasComponent<UIComponent>()) return;
		Temp->GetComponent<UIComponent>().m_Height = height;
	}

	static int BindSpriteGetHeight(CSEntityID ID)
	{
		Entity Temp{ VALIDATEENTITY(ID) };
		if (!Temp) return 0;
		if (!Temp->HasComponent<UIComponent>()) return 0;
		return Temp->GetComponent<UIComponent>().m_Height;
	}

	static void BindSpriteSetColor(CSEntityID ID, glm::vec4 color)
	{
		Entity Temp{ VALIDATEENTITY(ID) };
		if (!Temp) return;
		if (!Temp->HasComponent<UIComponent>()) return;
		Temp->GetComponent<UIComponent>().m_Color = color;
	}

	static glm::vec4 BindSpriteGetColor(CSEntityID ID)
	{
		Entity Temp{ VALIDATEENTITY(ID) };
		if (!Temp) return {};
		if (!Temp->HasComponent<UIComponent>()) return {};
		return Temp->GetComponent<UIComponent>().m_Color;
	}

	static void BindSpriteSetTexture(CSEntityID ID, MonoString* texture)
	{
		Entity Temp{ VALIDATEENTITY(ID) };
		if (!Temp) return;
		if (!Temp->HasComponent<UIComponent>()) return;
		UIComponent& uiComp = Temp->GetComponent<UIComponent>();
		std::string texturestr = MonoStringToString(texture);

		if (texturestr != "")
		{
			if (auto Texture = ResourceManager::Instance().GetResource<VulkanTexture>(texturestr); Texture)
			{
				uiComp.m_Texture = Texture;
			}
			else
			{
				std::string hexCode = Resource::GetGUIDHex(Resource::GenerateGUID(texturestr));
				uiComp.m_Texture = VulkanTexture::Deserialize(hexCode);

				if (uiComp.m_Texture == nullptr)
					PUBLISHERROR("Texture (" + texturestr + ") failed to load in UI Component");
			}
		}
		else
		{
			uiComp.m_Texture = nullptr;
		}
	}

	static MonoString* BindSpriteGetTexture(CSEntityID ID)
	{
		Entity Temp{ VALIDATEENTITY(ID) };
		if (!Temp) return mono_string_new(mono_domain_get(), "");
		if (!Temp->HasComponent<UIComponent>()) return mono_string_new(mono_domain_get(), "");
		return mono_string_new(mono_domain_get(), Temp->GetComponent<UIComponent>().m_Texture->GetHandleHex().c_str());
	}
#pragma endregion

#pragma region GameBindings
	static void BindCloseGame()
	{
		if (Engine::GetInstance().GetEngineInfo().EnableEditor)
		{
			GameLoop::Instance().ResetScene();
		}
		else
		{
			Engine::GetInstance().TellToShutdown();
		}
	}
#pragma endregion

#pragma region PersistenBindings
	static MonoString* BindGetPersistentVarVal(MonoString* VariableName)
	{
		return mono_string_new(mono_domain_get(), PersistentManager::Instance().GetVariableValue(MonoStringToString(VariableName)).c_str());
	}

	static void BindSetPersistentVarVal(MonoString* VariableName, MonoString* VariableValue)
	{
		PersistentManager::Instance().SetVariableValue(MonoStringToString(VariableName), MonoStringToString(VariableValue));
	}
#pragma endregion

#pragma region UIBindings

	static void Engine_SetVisible(CSEntityID id, bool visible)
	{
		Entity entity = VALIDATEENTITY(id);
		if (!entity) return;

		if (!entity->HasComponent<UIComponent>())
		{
			PUBLISHERROR("There is no UIComponent in " + entity->GetName() + "!");
			return;
		}

		entity->GetComponent<UIComponent>().m_IsVisible = visible;
	}

	static bool Engine_GetVisible(CSEntityID id)
	{
		Entity entity = VALIDATEENTITY(id);
		if (!entity) return false;

		if (!entity->HasComponent<UIComponent>())
		{
			PUBLISHERROR("There is no UIComponent in " + entity->GetName() + "!");
			return false;
		}

		return entity->GetComponent<UIComponent>().m_IsVisible;

	}
#pragma endregion

#pragma region TextBindings
	static void Engine_SetTextVisible(CSEntityID id, bool visible)
	{
		Entity entity = VALIDATEENTITY(id);
		if (!entity) return;

		if (!entity->HasComponent<TextComponent>())
		{
			PUBLISHERROR("There is no TextComponent in " + entity->GetName() + "!");
			return;
		}

		entity->GetComponent<TextComponent>().m_IsVisible = visible;
	}

	static bool Engine_GetTextVisible(CSEntityID id)
	{
		Entity entity = VALIDATEENTITY(id);
		if (!entity) return false;

		if (!entity->HasComponent<TextComponent>())
		{
			PUBLISHERROR("There is no TextComponent in " + entity->GetName() + "!");
			return false;
		}

		return entity->GetComponent<TextComponent>().m_IsVisible;
	}

	static void Engine_SetTextMessage(CSEntityID id, MonoString* TextMessage)
	{
		Entity entity = VALIDATEENTITY(id);
		if (!entity) return;

		if (!entity->HasComponent<TextComponent>())
		{
			PUBLISHERROR("There is no TextComponent in " + entity->GetName() + "!");
			return;
		}

		entity->GetComponent<TextComponent>().m_TextContent.Text = MonoStringToString(TextMessage);
	}

	static MonoString* Engine_GetTextMessage(CSEntityID id)
	{
		Entity entity = VALIDATEENTITY(id);
		if (!entity) return nullptr;

		if (!entity->HasComponent<TextComponent>())
		{
			PUBLISHERROR("There is no TextComponent in " + entity->GetName() + "!");
			return nullptr;
		}
		MonoString* monotext = mono_string_new(mono_domain_get(), entity->GetComponent<TextComponent>().m_TextContent.Text.c_str());

		return monotext;
	}

	static void Engine_StartDialogue(CSEntityID id)
	{
		Entity entity = VALIDATEENTITY(id);
		if (!entity) return;

		if (!entity->HasComponent<TextComponent>())
		{
			PUBLISHERROR("There is no TextComponent in " + entity->GetName() + "!");
		}
		
		entity->GetComponent<TextComponent>().m_IsDialogue = true;
	}

	static void Engine_ResetDialogue(CSEntityID id)
	{
		Entity entity = VALIDATEENTITY(id);
		if (!entity) return;

		if (!entity->HasComponent<TextComponent>())
		{
			PUBLISHERROR("There is no TextComponent in " + entity->GetName() + "!");
		}

		entity->GetComponent<TextComponent>().m_IsDialogue = false;
	}

#pragma endregion

#pragma region DirectPathfindingBinding
	static bool Engine_GetPathfindingRunning(CSEntityID id)
	{
		Entity entity = VALIDATEENTITY(id);
		if (!entity) return false;

		if (!entity->HasComponent<DirectPathfinding>())
		{
			PUBLISHERROR("There is no DirectPathfinding in " + entity->GetName() + "!");
			return false;
		}

		return entity->GetComponent<DirectPathfinding>().m_IsRunning;
	}

	static void Engine_StartPathfinding(CSEntityID id)
	{
		Entity entity = VALIDATEENTITY(id);
		if (!entity) return;

		if (!entity->HasComponent<DirectPathfinding>())
		{
			PUBLISHERROR("There is no DirectPathfinding in " + entity->GetName() + "!");
			return;
		}

		ECSSystemManager::Instance().GetSystem<DirectPathfindingSystem>()->StartPathfinding(entity);
	}

	static void Engine_PausePathfinding(CSEntityID id)
	{
		Entity entity = VALIDATEENTITY(id);
		if (!entity) return;

		if (!entity->HasComponent<DirectPathfinding>())
		{
			PUBLISHERROR("There is no DirectPathfinding in " + entity->GetName() + "!");
			return;
		}

		// Dont do anything if it is already paused
		if (!entity->GetComponent<DirectPathfinding>().m_IsRunning) return;

		ECSSystemManager::Instance().GetSystem<DirectPathfindingSystem>()->TogglePausePathfinding(entity);
	}

	static void Engine_ResumePathfinding(CSEntityID id)
	{
		Entity entity = VALIDATEENTITY(id);
		if (!entity) return;

		if (!entity->HasComponent<DirectPathfinding>())
		{
			PUBLISHERROR("There is no DirectPathfinding in " + entity->GetName() + "!");
			return;
		}

		// Dont do anything if it is already running
		if (entity->GetComponent<DirectPathfinding>().m_IsRunning) return;

		ECSSystemManager::Instance().GetSystem<DirectPathfindingSystem>()->TogglePausePathfinding(entity);
	}

	static void Engine_ResetPathfinding(CSEntityID id)
	{
		Entity entity = VALIDATEENTITY(id);
		if (!entity) return;

		if (!entity->HasComponent<DirectPathfinding>())
		{
			PUBLISHERROR("There is no DirectPathfinding in " + entity->GetName() + "!");
			return;
		}

		ECSSystemManager::Instance().GetSystem<DirectPathfindingSystem>()->ResetPathfinding(entity);
	}
#pragma endregion

#pragma region PostProcessing
	static void Engine_ShrinkVignette(float duration)
	{
		ECSSystemManager::Instance().GetSystem<ScenePostEffectsSystem>()->VignetteShrink(duration);
	}

	static bool Engine_GetVignetteStateIn()
	{
		return ECSSystemManager::Instance().GetSystem<ScenePostEffectsSystem>()->GetTransitionState(ScenePostEffectsSystem::TransitionTypeIndex::TYPE_VIGNETTE)
			== ScenePostEffectsSystem::STATE_IN;
	}

	static bool Engine_GetVignetteStateOut()
	{
		return ECSSystemManager::Instance().GetSystem<ScenePostEffectsSystem>()->GetTransitionState(ScenePostEffectsSystem::TransitionTypeIndex::TYPE_VIGNETTE)
			== ScenePostEffectsSystem::STATE_OUT;
	}
#pragma endregion

#pragma region Renderer
	static void Engine_SetSkyboxEnvironment(MonoString* Texture0, MonoString* Texture1, MonoString* Texture2, MonoString* Texture3, MonoString* Texture4, MonoString* Texture5)
	{
		std::string cpptexture0 = MonoStringToString(Texture0);
		std::string cpptexture1 = MonoStringToString(Texture1);
		std::string cpptexture2 = MonoStringToString(Texture2);
		std::string cpptexture3 = MonoStringToString(Texture3);
		std::string cpptexture4 = MonoStringToString(Texture4);
		std::string cpptexture5 = MonoStringToString(Texture5);

		Renderer::SetSkyboxEnvironment(cpptexture0, cpptexture1, cpptexture2, cpptexture3, cpptexture4, cpptexture5);
	}
#pragma endregion

#pragma region Particle
	static void Engine_SetParticleEmitterActive(CSEntityID id, bool active)
	{
		Entity entity = VALIDATEENTITY(id);
		if (!entity) return;
		if (!entity->HasComponent<ParticleComponent>())
		{
			PUBLISHERROR("There is no Particle Component in " + entity->GetName() + "!");
			return;
		}
		entity->GetComponent<ParticleComponent>().ResetParticles(entity->GetComponent<Transform>().m_Position);
		entity->GetComponent<ParticleComponent>().m_Running = active;
	}

	static bool Engine_GetParticleEmitterActive(CSEntityID id)
	{
		Entity entity = VALIDATEENTITY(id);
		if (!entity) return false;
		if (!entity->HasComponent<ParticleComponent>())
		{
			PUBLISHERROR("There is no Particle Component in " + entity->GetName() + "!");
			return false;
		}
		return entity->GetComponent<ParticleComponent>().m_Running;
	}

	void ScriptBind::RegisterFunctions()
	{
		// ECS Bindings
		{
			mono_add_internal_call("TRE.ECSManager::Engine_CreateEntity", BindCreateEntity);
			mono_add_internal_call("TRE.ECSManager::Engine_CloneEntity", BindCloneEntity);
			mono_add_internal_call("TRE.ECSManager::Engine_IsValidEntity", BindIsValidEntity);
			mono_add_internal_call("TRE.ECSManager::Engine_AddComponent", BindAddComponent);
			mono_add_internal_call("TRE.ECSManager::Engine_RemoveComponent", BindRemoveComponent);
			mono_add_internal_call("TRE.ECSManager::Engine_DestroyEntity", BindDestroyEntity);
			mono_add_internal_call("TRE.ECSManager::Engine_FindIDFromName", FindIDFromName);
			mono_add_internal_call("TRE.ECSManager::Engine_FindNameFromID", FindNameFromID);
			mono_add_internal_call("TRE.ECSManager::Engine_FindParentIDFromID", FindParentIDFromID);
			mono_add_internal_call("TRE.ECSManager::Engine_HasComponent", BindHasComponent);
		}

		// Entity Bindings
		{
			mono_add_internal_call("TRE.Entity::Engine_Rename", BindEntityRename);
			mono_add_internal_call("TRE.Entity::Engine_SetActive", BindEntityActive);
			mono_add_internal_call("TRE.Entity::Engine_GetActive", BindEntityGetActive);
			mono_add_internal_call("TRE.Entity::Engine_SetTag", BindEntitySetTag);
			mono_add_internal_call("TRE.Entity::Engine_GetTag", BindEntityGetTag);
			mono_add_internal_call("TRE.Entity::Engine_CompareTag", BindEntityCompareTag);
		}

		// Prefab Bindings
		{
			mono_add_internal_call("TRE.Prefab::Engine_IsPrefabResource", BindCheckIsPrefabResource);
			mono_add_internal_call("TRE.Prefab::Engine_CreatePrefabEntity", BindCreatePrefabEntity);
		}

		// Parent Bindings
		{
			mono_add_internal_call("TRE.ParentingSystem::Engine_ParentSetParent", BindParentSetParent);
			mono_add_internal_call("TRE.ParentingSystem::Engine_ParentRemoveParent", BindParentRemoveParent);
			mono_add_internal_call("TRE.ParentingSystem::Engine_ParentAddChild", BindParentAddChild);
			mono_add_internal_call("TRE.ParentingSystem::Engine_ParentRemoveChild", BindParentRemoveChild);
			mono_add_internal_call("TRE.ParentingSystem::Engine_GetChildID", BindParentGetChildFromIndex);
			mono_add_internal_call("TRE.ParentingSystem::Engine_GetChildIDFromName", BindParentGetChildFromName);
			mono_add_internal_call("TRE.ParentingSystem::Engine_GetTotalChildren", BindParentGetTotalChildren);
		}

		// Transform Bindings
		{
			mono_add_internal_call("TRE.TransformSystem::Engine_RotateVector", BindRotateVector);
			mono_add_internal_call("TRE.TransformSystem::Engine_SetPosition", BindSetPosition);
			mono_add_internal_call("TRE.TransformSystem::Engine_SetRotation", BindSetRotation);
			mono_add_internal_call("TRE.TransformSystem::Engine_SetScaling", BindSetScaling);
			mono_add_internal_call("TRE.TransformSystem::Engine_GetPosition", BindGetPosition);
			mono_add_internal_call("TRE.TransformSystem::Engine_GetRotation", BindGetRotation);
			mono_add_internal_call("TRE.TransformSystem::Engine_GetScaling", BindGetScaling);
		}

		// Mesh Renderer Bindings
		{
			mono_add_internal_call("TRE.MeshRendererSystem::Engine_SetMaterialInstance", BindSetMaterialInstance);
			//mono_add_internal_call("TRE.MeshRendererSystem::Engine_GetMaterialInstance", BindGetMaterialInstance);
			mono_add_internal_call("TRE.MeshRendererSystem::Engine_SetAnimMaterialInstance", BindSetAnimMaterialInstance);
			//mono_add_internal_call("TRE.MeshRendererSystem::Engine_GetAnimMaterialInstance", BindGetAnimMaterialInstance);
			mono_add_internal_call("TRE.MeshRendererSystem::Engine_SetMesh", BindSetMesh);
			//mono_add_internal_call("TRE.MeshRendererSystem::Engine_SetMeshName", BindSetMeshName);
			//mono_add_internal_call("TRE.MeshRendererSystem::Engine_GetMesh", BindGetMesh);
			mono_add_internal_call("TRE.MeshRendererSystem::Engine_IsCurrentMesh", BindIsMesh);
			mono_add_internal_call("TRE.MeshRendererSystem::Engine_IsCurrentMaterial", BindIsMaterialInstance);
			mono_add_internal_call("TRE.MeshRendererSystem::Engine_IsCurrentAnimMaterial", BindIsAnimMaterialInstance);
			mono_add_internal_call("TRE.MeshRendererSystem::Engine_SetMeshVisibility", BindSetMeshVisibility);
			mono_add_internal_call("TRE.MeshRendererSystem::Engine_GetMeshVisibility", BindGetMeshVisibility);
		}

		//Animation Bindings
		{
			mono_add_internal_call("TRE.AnimationSystem::Engine_SetAnimationSpeed", BindSetAnimationSpeed);
			mono_add_internal_call("TRE.AnimationSystem::Engine_GetAnimationSpeed", BindGetAnimationSpeed);
		}

		// Camera Bindings
		{
			mono_add_internal_call("TRE.CameraSystem::Engine_SetViewportSize", BindCamSetViewportSize);
			mono_add_internal_call("TRE.CameraSystem::Engine_SetFocalPoint", BindCamSetFocalPoint);
			mono_add_internal_call("TRE.CameraSystem::Engine_SetFocalLength", BindCamSetFocalLength);
			mono_add_internal_call("TRE.CameraSystem::Engine_SetFOV", BindCamSetFOV);
			mono_add_internal_call("TRE.CameraSystem::Engine_SetNear", BindCamSetNear);
			mono_add_internal_call("TRE.CameraSystem::Engine_SetFar", BindCamSetFar);
			mono_add_internal_call("TRE.CameraSystem::Engine_SetLeft", BindCamSetLeft);
			mono_add_internal_call("TRE.CameraSystem::Engine_SetRight", BindCamSetRight);
			mono_add_internal_call("TRE.CameraSystem::Engine_SetBottom", BindCamSetBottom);
			mono_add_internal_call("TRE.CameraSystem::Engine_SetTop", BindCamSetTop);
			mono_add_internal_call("TRE.CameraSystem::Engine_SetAspectRatio", BindCamSetAspectRatio);
			mono_add_internal_call("TRE.CameraSystem::Engine_SetIsPerspective", BindCamSetIsPerspective);
			mono_add_internal_call("TRE.CameraSystem::Engine_SetIsMainCamera", BindCamSetIsMainCamera);

			mono_add_internal_call("TRE.CameraSystem::Engine_GetViewMatrix", BindCamGetViewMatrix);
			mono_add_internal_call("TRE.CameraSystem::Engine_GetProjectionMatrix", BindCamGetProjectionMatrix);
			mono_add_internal_call("TRE.CameraSystem::Engine_GetInverseViewMatrix", BindCamGetInverseViewMatrix);
			mono_add_internal_call("TRE.CameraSystem::Engine_GetInverseProjectionMatrix", BindCamGetInverseProjectionMatrix);
			mono_add_internal_call("TRE.CameraSystem::Engine_GetInverseViewProjectionMatrix", BindCamGetInverseViewProjectionMatrix);
			mono_add_internal_call("TRE.CameraSystem::Engine_GetViewportSize", BindCamGetViewportSize);
			mono_add_internal_call("TRE.CameraSystem::Engine_GetFOV", BindCamGetFOV);
			mono_add_internal_call("TRE.CameraSystem::Engine_GetNear", BindCamGetNear);
			mono_add_internal_call("TRE.CameraSystem::Engine_GetFar", BindCamGetFar);
			mono_add_internal_call("TRE.CameraSystem::Engine_GetLeft", BindCamGetLeft);
			mono_add_internal_call("TRE.CameraSystem::Engine_GetRight", BindCamGetRight);
			mono_add_internal_call("TRE.CameraSystem::Engine_GetBottom", BindCamGetBottom);
			mono_add_internal_call("TRE.CameraSystem::Engine_GetTop", BindCamGetTop);
			mono_add_internal_call("TRE.CameraSystem::Engine_GetAspectRatio", BindCamGetAspectRatio);
			mono_add_internal_call("TRE.CameraSystem::Engine_IsPerspective", BindCamIsPerspective);
			mono_add_internal_call("TRE.CameraSystem::Engine_IsMainCamera", BindCamIsMainCamera);
			mono_add_internal_call("TRE.CameraSystem::Engine_SetMainCameraLookAt", BindCamMainSetLookAt);
			mono_add_internal_call("TRE.CameraSystem::Engine_SetMainCameraFollow", BindCamMainSetFollow);
			mono_add_internal_call("TRE.CameraSystem::Engine_TransitionMainCamera", BindTransitionMainCamera);
			mono_add_internal_call("TRE.CameraSystem::Engine_TransitionMainCameraPosition", BindTransitionMainCameraPosition);
			mono_add_internal_call("TRE.CameraSystem::Engine_TransitionMainCameraRotation", BindTransitionMainCameraRotation);

			mono_add_internal_call("TRE.CameraSystem::Engine_GetMainCameraPosition", BindGetMainCameraPosition);
			mono_add_internal_call("TRE.CameraSystem::Engine_GetMainCameraForwardVec", BindCameraForwardVector);
			mono_add_internal_call("TRE.CameraSystem::Engine_GetMainCameraRightVec", BindCameraRightVector);
			mono_add_internal_call("TRE.CameraSystem::Engine_GetMainCameraRotation", BindCameraRotation);

			mono_add_internal_call("TRE.CameraSystem::Engine_SetMainCameraPosition", BindSetMainCameraPosition);
		}

		// Physics Bindings
		{
			mono_add_internal_call("TRE.PhysicsSystem::Engine_ResizeSphereCollider", BindResizeSphereCollider);
			mono_add_internal_call("TRE.PhysicsSystem::Engine_GetSphereColliderRadius", BindGetSphereColliderRadius);
			mono_add_internal_call("TRE.PhysicsSystem::Engine_SetSphereTrigger", Engine_SetSphereTrigger);
			mono_add_internal_call("TRE.PhysicsSystem::Engine_GetSphereTrigger", Engine_GetSphereTrigger);

			mono_add_internal_call("TRE.PhysicsSystem::Engine_ResizeBoxCollider", BindResizeBoxCollider);
			mono_add_internal_call("TRE.PhysicsSystem::Engine_GetBoxColliderHalfExtents", BindGetBoxColliderHalfExtents);
			mono_add_internal_call("TRE.PhysicsSystem::Engine_SetBoxTrigger", Engine_SetBoxTrigger);
			mono_add_internal_call("TRE.PhysicsSystem::Engine_GetBoxTrigger", Engine_GetBoxTrigger);

			mono_add_internal_call("TRE.PhysicsSystem::Engine_ResizeCapsuleCollider", BindResizeCapsuleCollider);
			mono_add_internal_call("TRE.PhysicsSystem::Engine_GetCapsuleColliderRadius", BindGetCapsuleColliderRadius);
			mono_add_internal_call("TRE.PhysicsSystem::Engine_GetCapsuleColliderHalfHeight", BindGetCapsuleColliderHalfHeight);
			mono_add_internal_call("TRE.PhysicsSystem::Engine_SetCapsuleTrigger", Engine_SetCapsuleTrigger);
			mono_add_internal_call("TRE.PhysicsSystem::Engine_GetCapsuleTrigger", Engine_GetCapsuleTrigger);

			mono_add_internal_call("TRE.PhysicsSystem::Engine_ResizeCylinderCollider", BindResizeCylinderCollider);
			mono_add_internal_call("TRE.PhysicsSystem::Engine_GetCylinderColliderRadius", BindGetCylinderColliderRadius);
			mono_add_internal_call("TRE.PhysicsSystem::Engine_GetCylinderColliderHeight", BindGetCylinderColliderHeight);
			mono_add_internal_call("TRE.PhysicsSystem::Engine_SetCylinderTrigger", Engine_SetCylinderTrigger);
			mono_add_internal_call("TRE.PhysicsSystem::Engine_GetCylinderTrigger", Engine_GetCylinderTrigger);

			mono_add_internal_call("TRE.PhysicsSystem::Engine_UpdateColliderOffset", BindOffsetCollider);
			mono_add_internal_call("TRE.PhysicsSystem::Engine_GetColliderOffset", BindGetOffsetCollider);
			mono_add_internal_call("TRE.PhysicsSystem::Engine_AddForce", BindAddForce);
			mono_add_internal_call("TRE.PhysicsSystem::Engine_ConstrainRotationX", BindConstrainRotationX);
			mono_add_internal_call("TRE.PhysicsSystem::Engine_ConstrainRotationY", BindConstrainRotationY);
			mono_add_internal_call("TRE.PhysicsSystem::Engine_ConstrainRotationZ", BindConstrainRotationZ);
			mono_add_internal_call("TRE.PhysicsSystem::Engine_GetLinearVelocity", BindGetLinearVelocity);
			mono_add_internal_call("TRE.PhysicsSystem::Engine_SetLinearVelocity", BindSetLinearVelocity);

			mono_add_internal_call("TRE.PhysicsSystem::Engine_IsCollisionEnter", BindIsCollisionEnter);
			mono_add_internal_call("TRE.PhysicsSystem::Engine_IsCollisionStay", BindIsCollisionStay);
			mono_add_internal_call("TRE.PhysicsSystem::Engine_IsCollisionExit", BindIsCollisionExit);
			mono_add_internal_call("TRE.PhysicsSystem::Engine_IsTriggerEnter", BindIsTriggerEnter);
			mono_add_internal_call("TRE.PhysicsSystem::Engine_IsTriggerStay", BindIsTriggerStay);
			mono_add_internal_call("TRE.PhysicsSystem::Engine_IsTriggerExit", BindIsTriggerExit);

			mono_add_internal_call("TRE.PhysicsSystem::Engine_SetIsActive", BindSetIsActive);
			mono_add_internal_call("TRE.PhysicsSystem::Engine_GetIsActive", BindGetIsActive);

			mono_add_internal_call("TRE.PhysicsSystem::Engine_SetPauseState", BindSetPauseState);
			mono_add_internal_call("TRE.PhysicsSystem::Engine_GetPauseState", BindGetPauseState);
		}

		// RigidBody Binding
		{
			mono_add_internal_call("TRE.RigidBodySystem::Engine_SetKinematic", BindSetKinematic);
			mono_add_internal_call("TRE.RigidBodySystem::Engine_GetKinematic", BindGetKinematic);
			mono_add_internal_call("TRE.RigidBodySystem::Engine_SetGravity", BindSetGravity);
			mono_add_internal_call("TRE.RigidBodySystem::Engine_GetGravity", BindGetGravity);
		}

		// Input Binding
		{
			mono_add_internal_call("TRE.InputSystem::Engine_GetKeyHold", GetKeyHold);
			mono_add_internal_call("TRE.InputSystem::Engine_GetKeyPress", GetKeyPress);
			mono_add_internal_call("TRE.InputSystem::Engine_GetKeyRelease", GetKeyRelease);
		}

		// Logging
		{
			mono_add_internal_call("TRE.Debug::Engine_Log", SendMessageToConsole);
			mono_add_internal_call("TRE.Debug::Engine_LogWarning", SendWarningToConsole);
			mono_add_internal_call("TRE.Debug::Engine_LogError", SendErrorToConsole);
			mono_add_internal_call("TRE.Debug::Engine_LogCritical", SendCriticalToConsole);
		}

		// Math
		{
			mono_add_internal_call("TRE.MathF::Engine_Sqrt", BindSqrt);
			mono_add_internal_call("TRE.MathF::Engine_Lerp", BindLerp);
			mono_add_internal_call("TRE.MathF::Engine_Vec3Lerp", BindLerpVec3);
			mono_add_internal_call("TRE.MathF::Engine_Sin", BindSin);
		}

		// Random
		{
			mono_add_internal_call("TRE.Random::Engine_IntRange", BindIntRandRange);
			mono_add_internal_call("TRE.Random::Engine_FloatRange", BindFloatRandRange);
		}

		// Time
		{
			mono_add_internal_call("TRE.Time::Engine_GetDeltaTime", BindGetDeltaTime);
		}

		// Audio
		{
			mono_add_internal_call("TRE.AudioSystem::Engine_Play", BindSetPlaySound);
			mono_add_internal_call("TRE.AudioSystem::Engine_PlayOnce", BindSetPlaySound);
			mono_add_internal_call("TRE.AudioSystem::Engine_TogglePause", BindTogglePauseSound);
			mono_add_internal_call("TRE.AudioSystem::Engine_Stop", BindSetStop);
			mono_add_internal_call("TRE.AudioSystem::Engine_SetFileName", BindSetFileName);
			mono_add_internal_call("TRE.AudioSystem::Engine_GetFileName", BindGetFileName);
			mono_add_internal_call("TRE.AudioSystem::Engine_GetIsPlaying", BindIsPlaying);
		}

		// Scene
		{
			mono_add_internal_call("TRE.Scene::Engine_ChangeScene", BindLoadScene);
			mono_add_internal_call("TRE.Scene::Engine_TransitionScene", BindTransitionScene);
			mono_add_internal_call("TRE.Scene::Engine_GetSceneName", BindGetSceneName);
		}

		// Scripting
		{
			mono_add_internal_call("TRE.Script::Engine_IsScript", BindIsScript);
			mono_add_internal_call("TRE.Script::Engine_HaveScript", BindHaveScript);
			mono_add_internal_call("TRE.Script::Engine_GetScript", BindGetScript);
		}

		// SpriteRenderer
		{
			mono_add_internal_call("TRE.SpriteSystem::Engine_SetVisible", BindSpriteSetVisible);
			mono_add_internal_call("TRE.SpriteSystem::Engine_GetVisible", BindSpriteGetVisible);
			mono_add_internal_call("TRE.SpriteSystem::Engine_SetWidth", BindSpriteSetWidth);
			mono_add_internal_call("TRE.SpriteSystem::Engine_GetWidth", BindSpriteGetWidth);
			mono_add_internal_call("TRE.SpriteSystem::Engine_SetHeight", BindSpriteSetHeight);
			mono_add_internal_call("TRE.SpriteSystem::Engine_GetHeight", BindSpriteGetHeight);
			mono_add_internal_call("TRE.SpriteSystem::Engine_SetColor", BindSpriteSetColor);
			mono_add_internal_call("TRE.SpriteSystem::Engine_GetColor", BindSpriteGetColor);
			mono_add_internal_call("TRE.SpriteSystem::Engine_SetTexture", BindSpriteSetTexture);
			mono_add_internal_call("TRE.SpriteSystem::Engine_GetTexture", BindSpriteGetTexture);
		}

		// Game
		{
			mono_add_internal_call("TRE.Game::Engine_CloseGame", BindCloseGame);
		}

		// PersistentValues
		{
			mono_add_internal_call("TRE.PersistentSystem::Engine_GetPersistentValue", BindGetPersistentVarVal);
			mono_add_internal_call("TRE.PersistentSystem::Engine_SetPersistentValue", BindSetPersistentVarVal);
		}

		// UI
		{
			mono_add_internal_call("TRE.UISystem::Engine_SetVisible", Engine_SetVisible);
			mono_add_internal_call("TRE.UISystem::Engine_GetVisible", Engine_GetVisible);
		}

		// Text
		{
			mono_add_internal_call("TRE.TextSystem::Engine_SetTextVisible", Engine_SetTextVisible);
			mono_add_internal_call("TRE.TextSystem::Engine_GetTextVisible", Engine_GetTextVisible);
			mono_add_internal_call("TRE.TextSystem::Engine_SetTextMessage", Engine_SetTextMessage);
			mono_add_internal_call("TRE.TextSystem::Engine_GetTextMessage", Engine_GetTextMessage); 
			mono_add_internal_call("TRE.TextSystem::Engine_StartDialogue", Engine_StartDialogue);
			mono_add_internal_call("TRE.TextSystem::Engine_ResetDialogue", Engine_ResetDialogue);
		}

		// Direct Pathfinding
		{
			mono_add_internal_call("TRE.DirectPathfindingSystem::Engine_GetPathfindingRunning", Engine_GetPathfindingRunning);
			mono_add_internal_call("TRE.DirectPathfindingSystem::Engine_StartPathfinding", Engine_StartPathfinding);
			mono_add_internal_call("TRE.DirectPathfindingSystem::Engine_PausePathfinding", Engine_PausePathfinding);
			mono_add_internal_call("TRE.DirectPathfindingSystem::Engine_ResumePathfinding", Engine_ResumePathfinding);
			mono_add_internal_call("TRE.DirectPathfindingSystem::Engine_ResetPathfinding", Engine_ResetPathfinding);
		}

		// Post Effects
		{
			mono_add_internal_call("TRE.ScenePostEffectsSystem::Engine_ShrinkVignette", Engine_ShrinkVignette);
			mono_add_internal_call("TRE.ScenePostEffectsSystem::Engine_GetVignetteStateIn", Engine_GetVignetteStateIn);
			mono_add_internal_call("TRE.ScenePostEffectsSystem::Engine_GetVignetteStateOut", Engine_GetVignetteStateOut);
		}

		// Renderer
		{
			mono_add_internal_call("TRE.RendererSystem::Engine_SetSkyboxEnvironment", Engine_SetSkyboxEnvironment);
		}

		//Particle
		{
			mono_add_internal_call("TRE.ParticleSystem::Engine_SetParticleEmitterActive", Engine_SetParticleEmitterActive);
			mono_add_internal_call("TRE.ParticleSystem::Engine_GetParticleEmitterActive", Engine_GetParticleEmitterActive);
		}
	}
}
