#pragma once
#include "pch.h"
#include "entt.hpp"
#include "System.h"
#include "ComponentManager.h"
#include "Transform.h"
#include <typeindex>
#include "Core/Logger.h"

namespace TRE
{
	// DO NOT USE THIS UNLESS YOU WANT THE Entity TO BE DELETED!
	// Get this component in GetEntities to get Entity that are going to be deleted in this loop
	class Removal
	{
		//static constexpr auto in_place_delete = false;
		bool fake; //This value is to ensure it can compile and be registered
	};
	class Undeployed
	{
		bool fake;
	};

	class Properties
	{
	public:
		std::string m_Name; // To get the name
		bool m_Active;		// To check if it is active
	};

	class Ent;
	typedef std::shared_ptr<Ent> Entity;

	typedef std::uint32_t Entity_ID;

	class Ent : public std::enable_shared_from_this<Ent>
	{
	public:
		/* !
		@function	GetThis
		@author		Isaiah Lim lim.i@digipen.edu

		@brief		Returns this in the form of a shared_ptr (Entity)
					Note! Not using Ent, but using the shared_ptr variant.
					This should replace the variable this
		*//*__________________________________________________________________________*/
		Entity GetThis();

		/* !
		@function	HasComponent
		@author		Isaiah Lim lim.i@digipen.edu

		@brief		Checks if this Entity has a specific component

		Example:

		Entity goVar = ECSManager::Instance().CreateEntity("goVar");
		if (goVar->HasComponent<ComponentStruct>())
		{
			// Use Properties Component
		}
		*//*__________________________________________________________________________*/
		template <typename T>
		bool HasComponent();

		/* !
		@function	AddComponent
		@author		Isaiah Lim lim.i@digipen.edu

		@brief		Adds the component into the Entity
					If it already exist, can be used an alternative GetComponent

		[Warning]	If the typename is not registered in _component_manager, it will
					cause an assert

		Example:

		Entity goVar = ECSManager::Instance().CreateEntity("goVar");
		goVar->AddComponent<ComponentStruct>();

		goVar->AddComponent<ComponentStruct>().Var = 0;
		*//*__________________________________________________________________________*/
		template <typename T>
		T& AddComponent();

		/* !
		@function	GetComponent
		@author		Isaiah Lim lim.i@digipen.edu

		@brief		Get the component from the Entity

		[Warning]	If the typename is not registered in _component_manager,
					or the Entity is deleted or the Entity does not have the component,
					it will	cause an assert

		Example:
		if 
		*//*__________________________________________________________________________*/
		template <typename T>
		T& GetComponent();

		/* !
		@function	RemoveComponent
		@author		Isaiah Lim lim.i@digipen.edu

		@brief		Remove the component from the Entity

		[Warning]	If the typename is not registered in _component_manager, it will
					cause an assert

		Example:

		Entity goVar = ECSManager::Instance().CreateEntity("goVar");
		if (goVar->HasComponent<ComponentStruct>())
		{
			goVar->GetComponent<ComponentStruct>().Val = 0;
		}
		*//*__________________________________________________________________________*/
		template <typename T>
		void RemoveComponent();

		/* !
		@function	SetParent
		@author		Isaiah Lim lim.i@digipen.edu

		@params		Entity parent // Entity for the new parent

		@brief		Abandons the previous parent if it exist
					Set the parent of the Entity
					Automatically add this Entity to the parent's children list

		Example:

		Entity goVar = ECSManager::Instance().CreateEntity("goVar");
		Entity goParentVar = ECSManager::Instance().CreateEntity("goParentVar");
		goVar->SetParent(goParentVar);
		*//*__________________________________________________________________________*/
		void SetParent(Entity parent);

		/* !
		@function	GetParent
		@author		Isaiah Lim lim.i@digipen.edu

		@brief		Returns the parent of the Entity
					Returns nullptr if it doesn't exist

		Example:

		Entity goVar = ECSManager::Instance().CreateEntity("goVar");
		Entity goParentVar = ECSManager::Instance().CreateEntity("goParentVar");
		goVar->SetParent(goParentVar);

		Entity AccessEntityVarParent = goVar->GetParent();
		*//*__________________________________________________________________________*/
		Entity GetParent();

		/* !
		@function	RemoveParent
		@author		Isaiah Lim lim.i@digipen.edu

		@brief		Remove the parent of the Entity
					Automatically Abandons this Entity from the parent's children list

		Example:

		Entity goVar = ECSManager::Instance().CreateEntity("goVar");
		Entity goParentVar = ECSManager::Instance().CreateEntity("goParentVar");
		goVar->SetParent(goParentVar);

		goVar->RemoveParent();
		*//*__________________________________________________________________________*/
		void RemoveParent();

		/* !
		@function	AddChild
		@author		Isaiah Lim lim.i@digipen.edu

		@params		Entity child // Entity for the child

		@brief		Add a child to this Entity children list
					Automatically set this Entity as the parent of the child

		Example:

		Entity goVar = ECSManager::Instance().CreateEntity("goVar");
		Entity goChildVar = ECSManager::Instance().CreateEntity("goChildVar");

		goVar->AddChild(goChildVar);
		*//*__________________________________________________________________________*/
		void AddChild(Entity child);

		/* !
		@function	GetChildren
		@author		Isaiah Lim lim.i@digipen.edu

		@brief		Returns the children of the Entity

		Example:

		Entity goVar = ECSManager::Instance().CreateEntity("goVar");
		Entity goParentVar = ECSManager::Instance().CreateEntity("goParentVar");
		goVar->SetParent(goParentVar);

		std::vector<Entity> goParentVarChildren = goParentVar->GetChildren();
		*//*__________________________________________________________________________*/
		std::vector<Entity> GetChildren();

		/* !
		@function	AbandonChild
		@author		Isaiah Lim lim.i@digipen.edu

		@params		Entity child // Entity for the child

		@brief		Remove child from children list
					Automatically remove parent from the child Entity
					Ignores command if child's parent is not this Entity

		Example:

		Entity goVar = ECSManager::Instance().CreateEntity("goVar");
		Entity goParentVar = ECSManager::Instance().CreateEntity("goParentVar");
		goVar->SetParent(goParentVar);

		goParentVar->AbandonChild(goVar);
		*//*__________________________________________________________________________*/
		void AbandonChild(Entity child);

		/* !
		@function	AbandonChildren
		@author		Isaiah Lim lim.i@digipen.edu

		@brief		Remove all children from children list
					Automatically remove parent from each child Entity

		Example:

		Entity goVar = ECSManager::Instance().CreateEntity("goVar");
		Entity goParentVar = ECSManager::Instance().CreateEntity("goParentVar");
		goVar->SetParent(goParentVar);

		goParentVar->AbandonChildren();
		*//*__________________________________________________________________________*/
		void AbandonChildren();
		
	private:
		friend class ECSManager;
		friend class MemoryManager;

		Entity m_Parent;
		std::vector<Entity> m_Children;

		entt::entity m_Entity;
	};

	class ECSOutputArchive
	{
	public:
		void operator()(entt::entity ent);
		void operator()(std::underlying_type_t<entt::entity> u)
		{
			std::cout << u << ";";
		}
		template <typename T>
		void operator()(const T& t)
		{
			std::cout << "&";
		}
	};

	class ECSInputArchive
	{
	public:
		void operator()(entt::entity& ent)
		{
			std::cout << static_cast<std::underlying_type_t<entt::entity>>(ent) << "\\";
		}
		void operator()(std::underlying_type_t<entt::entity>& u)
		{
			std::cout << u << ":";
		}
		template <typename T>
		void operator()(const T&)
		{
			std::cout << "/";
		}
	};

	// ECS Manager (Entity Manager) THERE CAN ONLY BE ONE! >:o
	//==================================================
	class ECSManager
	{
	public:
		/* !
		@function	Instance
		@author		Isaiah Lim lim.i@digipen.edu

		@brief		Holds the singleton for the ECSManager
					This shouldn't be used, use ECSManager::Instance() instead
		*//*__________________________________________________________________________*/
		static ECSManager& Instance();

		/* !
		@function	GetRegistry
		@author		Isaiah Lim lim.i@digipen.edu

		@brief		Returns entt registry in the ECSManager::Instance() singleton
					This should ideally be used for ECSManager::Instance() only

		Example:
		ECSManager::Instance().GetRegistry();
		*//*__________________________________________________________________________*/
		entt::registry& GetRegistry();

		// Engine Loop
		/* !
		@function	DestroyRemovalEntities
		@author		Isaiah Lim lim.i@digipen.edu

		@brief		Delete all Entity that has the component Removal
					This is only run under Engine.cpp at the end of the frame

		Example:
		ECSManager::Instance().DestroyRemovalEntity();
		*//*__________________________________________________________________________*/
		void DeleteRemovalEntities();

		// Shutdown Functions
		/* !
		@function	DestroyAll
		@author		Isaiah Lim lim.i@digipen.edu

		@brief		Destroys all the Entitys

		Example:
		ECSManager::Instance().DestroyAll();
		*//*__________________________________________________________________________*/
		void DestroyAll();

		// Entity Controller
		/* !
		@function	CreateEntity
		@author		Isaiah Lim lim.i@digipen.edu

		@params		std::string name	// Default name is "Ent", can be renamed
										// name is found in Properties::m_Name

		@brief		Create a new Entity
					Automatically adds the Properties Component

		Example:

		Entity goVar = ECSManager::Instance().CreateEntity("goVar");

		std::cout << goVar->HasComponent<Properties>() << std::endl;
		std::cout << goVar->GetComponent<Properties>().m_Name << std::endl;
		// Output
		// true
		// goVar
		*//*__________________________________________________________________________*/
		Entity CreateEntity(std::string name = "Ent");

		/* !
		@function	MarkForDeletion
		@author		Isaiah Lim lim.i@digipen.edu

		@params		Entity& object			// Entity to be destroyed

		@brief		Create a new Entity

		Example:

		Entity goVar = ECSManager::Instance().CreateEntity("goVar");

		std::cout << goVar << std::endl; // Address of goVar

		ECSManager::Instance().MarkForDeletion(goVar);
		*//*__________________________________________________________________________*/
		void MarkForDeletion(Entity& object);

		/* !
		@function	CloneEntity
		@author		Isaiah Lim lim.i@digipen.edu

		@params		Entity& object			// Entity to clone from
					std::string name	// Name for the cloned Entity

		@brief		Create a new Entity from an existing Entity

		Example:

		Entity goVar = ECSManager::Instance().CreateEntity("goVar");
		Entity clonGoVar = ECSManager::Instance().CloneEntity(goVar, "ClonedEntityName");
		*//*__________________________________________________________________________*/
		Entity CloneEntity(Entity& object, std::string name = "Cloned_GameObject");

		/* !
		@function	EntityHasComponent
		@author		Isaiah Lim lim.i@digipen.edu

		@params		Entity& object			// Entity to check from

		@brief		Check if the Entity have a component

		Example:

		Entity goVar = ECSManager::Instance().CreateEntity("goVar");
		goVar->AddComponent<Transform>();

		std::cout << ECSManager::Instance().EntityHasComponent<Transform>(goVar) << std::endl;
		std::cout << ECSManager::Instance().EntityHasComponent<HUMAN>(goVar) << std::endl;

		// Output
		// true
		// false
		*//*__________________________________________________________________________*/
		template <typename T>
		bool EntityHasComponent(Entity object);

		/* !
		@function	GetEntities
		@author		Isaiah Lim lim.i@digipen.edu

		@brief		Returns a vector of Entity with the components listed

		Example:

		Entity goVar = ECSManager::Instance().CreateEntity("Object1");
		goVar->AddComponent<Transform>();
		goVar->AddComponent<Component>();

		Entity goVar2 = ECSManager::Instance().CreateEntity("Object2");

		Entity goVar3 = ECSManager::Instance().CreateEntity("Object3");
		goVar3->AddComponent<Component>();
		ECSManager::Instance().DestroyEntity(goVar3);

		std::cout << "Entity with Properties Size: " << ECSManager::Instance().GetEntities<Properties>().size() << std::endl;
		for (Entity& obj : ECSManager::Instance().GetEntities<Properties>())
			std::cout << "- " << obj->GetComponent<Properties>().m_Name << std::endl;

		std::cout << "Entity with Transform Size: " << ECSManager::Instance().GetEntities<Transform>().size() << std::endl;
		for (Entity& obj : ECSManager::Instance().GetEntities<Transform>())
			std::cout << "- " << obj->GetComponent<Properties>().m_Name << std::endl;

		std::cout << "Entity with Component Size: " << ECSManager::Instance().GetEntities<Component>().size() << std::endl;
		for (Entity& obj : ECSManager::Instance().GetEntities<Component>())
			std::cout << "- " << obj->GetComponent<Properties>().m_Name << std::endl;

		std::cout << "Entity with Removal Size: " << ECSManager::Instance().GetEntities<Removal>().size() << std::endl;
		for (Entity& obj : ECSManager::Instance().GetEntities<Removal>())
			std::cout << "- " << obj->GetComponent<Properties>().m_Name << std::endl;

		std::cout << "Entity with Transform and Component Size: " << ECSManager::Instance().GetEntities<Transform, Component>().size() << std::endl;
		for (Entity& obj : ECSManager::Instance().GetEntities<Transform, Component>())
			std::cout << "- " << obj->GetComponent<Properties>().m_Name << std::endl;

		// Output
		// Entity with Properties Size: 3
		// - Object1
		// - Object2
		// - Object3
		// Entity with Transform Size: 3
		// - Object1
		// - Object2
		// - Object3
		// Entity with Component Size: 2
		// - Object1
		// - Object3
		// Entity with Removal Size: 1
		// - Object3
		// Entity with Transform and Component and Removal Size: 1
		// - Object3
		*//*__________________________________________________________________________*/
		template <typename Comp, typename... Others>
		std::vector<Entity> GetEntities();

		/* !
		@function	GetAllEntities
		@author		Isaiah Lim lim.i@digipen.edu

		@brief		Returns a vector of all Entities
		*//*__________________________________________________________________________*/
		std::vector<Entity> GetAllEntities();

		// TODELETE
		void TESTRUN()
		{
			std::cout << "\nTEST RUNNING ECS\n====================================\n";

			/*std::cout << "Sizes: " << m_EntityList.size() << "\n";
			for (auto&& storage : GetRegistry().storage())
				std::cout << "- " << storage.first << "|" << storage.second.size() << "\n";
			std::cout << "-------\n";*/

			Entity test = CreateEntity("Test 1");

			/*std::cout << "Sizes: " << m_EntityList.size() << "\n";
			for (auto&& storage : GetRegistry().storage())
				std::cout << "- " << storage.first << "|" << storage.second.size() << "\n";
			std::cout << "-------\n";

			std::cout << "\n\nEntities IDs\n";
			for (auto& pair : m_EntityList)
			{
				std::cout << "> " << static_cast<std::uint32_t>(pair.second->m_Entity) << "\n";
			}
			std::cout << "-- Storage --\n";
			for (auto&& id : GetRegistry().storage().begin()->second)
			{
				std::cout << "> " << static_cast<std::uint32_t>(id) << "\n";
			}
			std::cout << "++++++++++++\n";*/

			test->AddComponent<Transform>().m_Position.x = 19;
			std::cout << "Creating Entity, Adding, Getting and editing a value: " << test->GetComponent<Transform>().m_Position.x << std::endl;
			std::cout << "Removing Editted Component...\n";
			test->RemoveComponent<Transform>();

			/*std::cout << "Sizes: " << m_EntityList.size() << "\n";
			for (auto&& storage : GetRegistry().storage())
				std::cout << "- " << storage.first << "|" << storage.second.size() << "\n";
			std::cout << "-------\n";*/

			//std::cout << "Attempting to get a component it does not have: " << test->GetComponent<Transform>().m_Scale.x << std::endl; // Will call assert in GetComponent!
			std::cout << "Default Parent: " << test->GetParent() << "\n";
			MarkForDeletion(test);
			std::cout << "Destroyed earlier Entity...\n";
			//std::cout << "Attempting to call a deleted/destroyed Entity: " << test->GetComponent<Properties>().m_Name << std::endl; // Will not call assert in GetComponent until next loop!

			/*std::cout << "Sizes: " << m_EntityList.size() << "\n";
			for (auto&& storage : GetRegistry().storage())
				std::cout << "- " << storage.first << "|" << storage.second.size() << "\n";
			std::cout << "-------\n";

			std::cout << "\n\nEntities IDs\n";
			for (auto& pair : m_EntityList)
			{
				std::cout << "> " << static_cast<std::uint32_t>(pair.second->m_Entity) << "\n";
			}
			std::cout << "-- Storage --\n";
			for (auto&& id : GetRegistry().storage().begin()->second)
			{
				std::cout << "> " << static_cast<std::uint32_t>(id) << "\n";
			}
			std::cout << "++++++++++++\n";*/

			std::cout << "Creating Entitys with 1 Entity with only Properties and 2 Entity with Transform and Properties...\n";
			Entity test2 = CreateEntity("test2");

			Entity allobj = CreateEntity("allobj");
			Entity allobj2 = CreateEntity("allobj2");

			std::cout << "Total Entity with Transform: " << GetEntities<Transform>().size() << "\n";
			std::cout << "Total Entity with Properties: " << GetEntities<Properties>().size() << "\n";
			std::cout << "Total Entity with Transform and Properties: " << GetEntities<Transform, Properties>().size() << "\n";

			std::cout << "Testing cloning Entity...\n";
			std::cout << "- Setting Original Entity value to 123...\n";
			Entity oriobj = CreateEntity("oriobj");
			oriobj->AddComponent<Transform>().m_Position.x = 123;
			std::cout << "- Cloning Original Entity\n";
			Entity cloneobj = CloneEntity(oriobj);
			std::cout << "- Cloned Entity value is " << cloneobj->GetComponent<Transform>().m_Position.x << "\n";
			std::cout << "- Setting Original Entity value to 0...\n";
			oriobj->GetComponent<Transform>().m_Position.x = 0;
			std::cout << "- Cloned Entity value is " << cloneobj->GetComponent<Transform>().m_Position.x << "\n";

			std::cout << "\nIterating All Available Component in ComponentManager\n";
			for (auto comp : ComponentManager::Instance().m_Components)
			{
				std::cout << "- " << comp.second << "\n";
			}

			std::cout << "\nTesting iterating through All Entity with Properties\n";
			for (auto go : GetEntities<Properties>())
			{
				go->GetComponent<Properties>().m_Active = true;
			}
			std::cout << "- Testing Complete\n";

			/*std::cout << "Sizes: " << m_EntityList.size() << "\n";
			for (auto&& storage : GetRegistry().storage())
				std::cout << "- " << storage.first << "|" << storage.second.size() << "\n";
			std::cout << "-------\n";*/

			std::cout << "\nTesting setting, getting and removing parent\n";
			Entity parentEntity = CreateEntity();
			Entity childEntity = CreateEntity();
			std::cout << "- Default childEntity parent: " << childEntity->GetParent() << "\n";
			std::cout << "- childEntity address: " << childEntity << "\n";
			std::cout << "- parentEntity address: " << parentEntity << "\n";
			std::cout << "- Setting parentEntity as childEntity parent...\n";
			childEntity->SetParent(parentEntity);
			std::cout << "- New childEntity parent: " << childEntity->GetParent() << "\n";
			std::cout << "- childEntity children size: " << childEntity->GetChildren().size() << "\n";
			std::cout << "- parentEntity children size: " << parentEntity->GetChildren().size() << "\n";
			std::cout << "- Removing childEntity parent...\n";
			childEntity->RemoveParent();
			std::cout << "- Removed childEntity parent: " << childEntity->GetParent() << "\n";
			std::cout << "- childEntity children size: " << childEntity->GetChildren().size() << "\n";
			std::cout << "- parentEntity children size: " << parentEntity->GetChildren().size() << "\n";
			std::cout << "- Setting childEntity as childEntity parent...\n";
			childEntity->SetParent(childEntity);
			std::cout << "- New childEntity parent (Ideally it would set parent as a nullptr): " << childEntity->GetParent() << "\n";
			std::cout << "- childEntity children size: " << childEntity->GetChildren().size() << "\n";
			std::cout << "- parentEntity children size: " << parentEntity->GetChildren().size() << "\n";
			std::cout << "\n- Adding 5 Entitys to parentEntity as children...\n";
			parentEntity->AddChild(test2);
			parentEntity->AddChild(allobj);
			parentEntity->AddChild(allobj2);
			parentEntity->AddChild(cloneobj);
			parentEntity->AddChild(oriobj);
			std::cout << "- parentEntity children size: " << parentEntity->GetChildren().size() << "\n";
			std::cout << "- 1 Entity removing parentEntity...\n";
			test2->RemoveParent();
			std::cout << "- parentEntity children size: " << parentEntity->GetChildren().size() << "\n";
			std::cout << "- parentEntity abandoning 1 children...\n";
			parentEntity->AbandonChild(allobj);
			std::cout << "- parentEntity children size: " << parentEntity->GetChildren().size() << "\n";
			std::cout << "- 1 Entity Setting another parent...\n";
			allobj2->SetParent(childEntity);
			std::cout << "- parentEntity children size: " << parentEntity->GetChildren().size() << "\n";
			std::cout << "- parentEntity abandoning all remaining children...\n";
			parentEntity->AbandonChildren();
			std::cout << "- parentEntity children size: " << parentEntity->GetChildren().size() << "\n";
			std::cout << "- Attempting to remove a non child in parentEntity...\n";
			parentEntity->AbandonChild(test2);
			std::cout << "- Testing Complete\n";

			/*std::cout << "Sizes: " << m_EntityList.size() << "\n";
			for (auto&& storage : GetRegistry().storage())
				std::cout << "- " << storage.first << "|" << storage.second.size() << "\n";
			std::cout << "-------\n";*/

			std::cout << "\nTesting Listener\n";
			registry.on_construct<Transform>().connect<&Transform::Init>();
			registry.on_update<Transform>().connect<&Transform::UpdateValues>();
			registry.on_destroy<Transform>().connect<&Transform::Destroy>();

			Entity listenerEntity = CreateEntity("listenerEntity");
			std::cout << "\nTesting Listening to Adding Component, should call Init\n";
			listenerEntity->AddComponent<Transform>();
			std::cout << "\nTesting Listening to Changing Component Values, should call Update\n";
			listenerEntity->GetComponent<Transform>().m_Position.x = 5; // This wont work
			listenerEntity->GetComponent<Transform>().m_Position.y = 85; // This wont work
			std::cout << "Original Value: " << listenerEntity->GetComponent<Transform>().m_Position.x << ", " << listenerEntity->GetComponent<Transform>().m_Position.y << "\n";
			// replaces the component in-place
			//int newVal = 69;
			//GetRegistry().patch<Transform>(listenerEntity->m_Entity, [&](Transform& pos) { pos.m_Position.x = newVal; });
			//GetRegistry().patch<Transform>(listenerEntity->m_Entity, &Transform::SetPosX);
			std::cout << "New Value: " << listenerEntity->GetComponent<Transform>().m_Position.x << ", " << listenerEntity->GetComponent<Transform>().m_Position.y << "\n";
			std::cout << "\nTesting Listening to Destroying Values, should call Destroy\n";
			listenerEntity->RemoveComponent<Transform>();

			std::cout << "\nDisconnecting Listeners...\n";
			registry.on_construct<Transform>().disconnect<&Transform::Init>();
			registry.on_update<Transform>().disconnect<&Transform::UpdateValues>();
			registry.on_destroy<Transform>().disconnect<&Transform::Destroy>();

			std::cout << "- Testing complete!\n";

			std::cout << "\nTesting observer noticing if any Transform change\n";
			entt::observer existingObserver{ registry, entt::collector.group<Transform>() };
			entt::observer updatedObserver{ registry, entt::collector.update<Transform>() };
			std::cout << "- Adding Entity for observer to observe...\n";
			Entity observerEntity = CreateEntity("ObserverEntity");
			observerEntity->AddComponent<Transform>();

			std::cout << "Existing Transform Observer Size: " << existingObserver.size() << "\n";

			std::cout << "Updated Transform Observer Size: " << updatedObserver.size() << "\n";

			std::cout << "\n- Testing if changing variables manually would affect\n";
			observerEntity->GetComponent<Transform>().m_Position.x = 5;
			std::cout << "Updated Transform Observer Size: " << updatedObserver.size() << "\n";
			std::cout << "-- If size is same, it did not update\n";

			std::cout << "\n- Testing if changing variables using patch in entt would affect\n";
			GetRegistry().patch<Transform>(observerEntity->m_Entity, [&](Transform& pos) { pos.m_Position.x = 5; });
			std::cout << "Updated Transform Observer Size: " << updatedObserver.size() << "\n";
			std::cout << "-- If size is same, it did not update\n";

			std::cout << "\n- Testing if changing variables using patch in entt on the same Entity would cause dups\n";
			GetRegistry().patch<Transform>(observerEntity->m_Entity, [&](Transform& pos) { pos.m_Position.x = 0; });
			std::cout << "Updated Transform Observer Size: " << updatedObserver.size() << "\n";
			std::cout << "-- If size is same, it did not dup\n";

			std::cout << "\n- Clearing observers size... (REMEMBER TO DISCONNECT FROM REGISTRY OR IT WILL CRASH ON SHUTDOWN!)\n";
			existingObserver.clear();
			updatedObserver.clear();
			existingObserver.disconnect();
			updatedObserver.disconnect();
			std::cout << "- Testing complete!\n";

			std::cout << "\nTrying out snapshot for archiving entities\n";
			std::cout << "- Total Objects: " << GetEntities<Properties>().size() << "...\n";

			std::cout << "- Archiving to Output: " << GetEntities<Properties>().size() << "...\n";
			ECSOutputArchive str{};
			entt::exclude_t<Undeployed> u;
			const auto view = registry.view<Undeployed>();
			int i{};
			for (auto& obj : view)
			{
				++i;
			}
			std::cout << ">>>> " << i << "\n";
			// Destroys all undeployed entities
			registry.destroy(view.begin(), view.end());
			i = 0;
			for (auto& obj : registry.view<Undeployed>())
			{
				++i;
			}
			std::cout << ">>>> " << i << "\n";

			entt::snapshot snapshot{ GetRegistry() };
			// Serialize all entities and components
			snapshot.entities(str).component<>(str);
			//snapshot.component<>(str);

			std::cout << "\n\nEntities IDs\n";
			for (auto& pair : m_EntityList)
			{
				std::cout << "> " << static_cast<std::uint32_t>(pair.second->m_Entity) << "\n";
			}
			std::cout << "-- Storage --\n";
			for (auto&& id : GetRegistry().storage().begin()->second)
			{
				std::cout << "> " << static_cast<std::uint32_t>(id) << "\n";
			}
			std::cout << "++++++++++++\n";

			std::cout << "\n- Destroy All...\n";
			DestroyAll();
			std::cout << "- Current: " << GetEntities<Properties>().size() << "...\n";

			/*std::cout << "Sizes: " << m_EntityList.size() << "\n";
			for (auto&& storage : GetRegistry().storage())
				std::cout << "- " << storage.first << "|" << storage.second.size() << "\n";
			std::cout << "-------\n";

			std::cout << "\n\nEntities IDs\n";
			for (auto& pair : m_EntityList)
			{
				std::cout << "> " << static_cast<std::uint32_t>(pair.second->m_Entity) << "\n";
			}
			std::cout << "-- Storage --\n";
			for (auto&& id : GetRegistry().storage().begin()->second)
			{
				std::cout << "> " << static_cast<std::uint32_t>(id) << "\n";
			}
			std::cout << "++++++++++++\n";*/
			/*ECSInputArchive instr{};
			entt::snapshot_loader snapshotLoader{ GetRegistry() };
			snapshotLoader.entities(instr);*/
			//snapshotLoader.component<>(str);

			std::cout << "\nDestroying all " << GetEntities<Properties>().size() << "  test objects...\n";
			DestroyAll();
			std::cout << "- Remaining: " << GetEntities<Properties>().size() << " | Successfully cleared: " << (GetEntities<Properties>().empty() ? "true" : "false") << "\n";

			std::cout << "====================================\n\n";
		}

	private:
		friend class MemoryManager;

		// Delete possible copy ctor and assignment to ensure singleton
		ECSManager() {};
		ECSManager(ECSManager const&) = delete;
		void operator=(ECSManager const&) = delete;
		void* operator new(size_t) = delete;

		// EnTT stuff
		entt::registry registry;

		// Component Types
		std::map<size_t, void*> componentTypes;

		std::unordered_map<Entity_ID, Entity> m_EntityList;
	};


	template <typename Comp, typename... Others>
	std::vector<Entity> ECSManager::GetEntities()
	{
		std::vector<Entity> objects{};
		auto view = registry.view<Comp, Others...>();
		objects.reserve(m_EntityList.size());

		// Get all Entity owning the entities
		for (entt::entity obj : view)
		{
			if (m_EntityList.find(static_cast<Entity_ID>(obj)) != m_EntityList.end())
			{
				objects.emplace_back(m_EntityList[static_cast<Entity_ID>(obj)]);
			}
		}

		return objects;
	}

	template <typename T>
	bool ECSManager::EntityHasComponent(Entity object)
	{
		return registry.any_of<T>(object->m_Entity);
	}

	template <typename T>
	bool Ent::HasComponent()
	{
		if (!ComponentManager::Instance().HasComponent<T>() && !ComponentManager::Instance().HasHiddenComponent<T>())
		{
			std::string funcName{ __FUNCTION__ };
			std::string compName{ typeid(T).name() };
			TRE_CORE_ERROR("[" + funcName + "] Component " + compName + " is not included in _component_manager");
			assert(ComponentManager::Instance().HasComponent<T>() || ComponentManager::Instance().HasHiddenComponent<T>());
		}
		return ECSManager::Instance().EntityHasComponent<T>(shared_from_this());
	}

	template <typename T>
	T& Ent::AddComponent()
	{
		if (!ComponentManager::Instance().HasComponent<T>() && !ComponentManager::Instance().HasHiddenComponent<T>())
		{
			std::string funcName{ __FUNCTION__ };
			std::string compName{ typeid(T).name() };
			TRE_CORE_ERROR("[" + funcName + "] Component " + compName + " is not included in _component_manager");
			assert(ComponentManager::Instance().HasComponent<T>() || ComponentManager::Instance().HasHiddenComponent<T>());
		}

		if (HasComponent<T>())
		{
			std::string funcName{ __FUNCTION__ };
			std::string compName{ typeid(T).name() };
			TRE_CORE_ERROR("[" + funcName + "] Component " + compName + " is already in " + GetComponent<Properties>().m_Name + "...");
			return GetComponent<T>();
		}
		return ECSManager::Instance().GetRegistry().emplace<T>(m_Entity);
	}

	template <typename T>
	T& Ent::GetComponent()
	{
		// Ensure cannot get a component from a freed object and entity
		if (this == nullptr || &m_Entity == nullptr)
		{
			std::string funcName{ __FUNCTION__ };
			TRE_CORE_ERROR("[" + funcName + "] " + GetComponent<Properties>().m_Name + " is no longer valid (this or entity is nullptr)");
			assert(this != nullptr);
			assert(&m_Entity != nullptr);
		}

		if (!ComponentManager::Instance().HasComponent<T>() && !ComponentManager::Instance().HasHiddenComponent<T>())
		{
			std::string funcName{ __FUNCTION__ };
			std::string compName{ typeid(T).name() };
			TRE_CORE_ERROR("[" + funcName + "] Component " + compName + " is not included in _component_manager");
			assert(ComponentManager::Instance().HasComponent<T>() || ComponentManager::Instance().HasHiddenComponent<T>());
		}

		if (!HasComponent<T>())
		{
			std::string funcName{ __FUNCTION__ };
			std::string compName{ typeid(T).name() };
			TRE_CORE_ERROR("[" + funcName + "] " + GetComponent<Properties>().m_Name + " does not have the component " + compName);
			assert(HasComponent<T>());
		}

		return ECSManager::Instance().GetRegistry().get<T>(m_Entity);
	}

	template <typename T>
	void Ent::RemoveComponent()
	{
		// Ensure cannot get a component from a freed object and entity
		if (this == nullptr || &m_Entity == nullptr)
		{
			std::string funcName{ __FUNCTION__ };
			TRE_CORE_ERROR("[" + funcName + "] " + GetComponent<Properties>().m_Name + " is no longer valid (this or entity is nullptr)");
			assert(this != nullptr);
			assert(&m_Entity != nullptr);
		}

		if (!ComponentManager::Instance().HasComponent<T>() && !ComponentManager::Instance().HasHiddenComponent<T>())
		{
			std::string funcName{ __FUNCTION__ };
			std::string compName{ typeid(T).name() };
			TRE_CORE_ERROR("[" + funcName + "] Component " + compName + " is not included in _component_manager");
			assert(ComponentManager::Instance().HasComponent<T>() || ComponentManager::Instance().HasHiddenComponent<T>());
		}

		if (HasComponent<T>())
			ECSManager::Instance().GetRegistry().remove<T>(m_Entity);
	}
}