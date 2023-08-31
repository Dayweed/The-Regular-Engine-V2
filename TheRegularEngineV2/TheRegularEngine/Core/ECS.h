#pragma once
#include "pch.h"
#include "entt.hpp"
#include "System.h"
#include "ComponentManager.h"
#include "Transform.h"
#include <typeindex>
//#include "Core/Logger.h"

namespace TRE
{
	// DO NOT USE THIS UNLESS YOU WANT THE GO TO BE DELETED!
	// Get this component in GetGO to get GO that are going to be deleted in this loop
	class Removal
	{
		//static constexpr auto in_place_delete = false;
		bool fake; //This value is to ensure it can compile and be registered
	};

	class Properties
	{
	public:
		std::string m_Name; // To get the name
		bool m_Active;		// To check if it is active
	};

	class GameObject;
	typedef std::shared_ptr<GameObject> GO;

	class GameObject : public std::enable_shared_from_this<GameObject>
	{
	public:
		/* !
		@function	GetThis
		@author		Isaiah Lim lim.i@digipen.edu

		@brief		Returns this in the form of a shared_ptr (GO)
					Note! Not using GameObject, but using the shared_ptr variant.
					This should replace the variable this
		*//*__________________________________________________________________________*/
		GO GetThis();

		/* !
		@function	HasComponent
		@author		Isaiah Lim lim.i@digipen.edu

		@brief		Checks if this GO has a specific component

		Example:

		GO goVar = _ecs_manager->CreateGO("goVar");
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

		@brief		Adds the component into the GO
					If it already exist, can be used an alternative GetComponent

		[Warning]	If the typename is not registered in _component_manager, it will
					cause an assert

		Example:

		GO goVar = _ecs_manager->CreateGO("goVar");
		goVar->AddComponent<ComponentStruct>();

		goVar->AddComponent<ComponentStruct>().Var = 0;
		*//*__________________________________________________________________________*/
		template <typename T>
		T& AddComponent();

		/* !
		@function	GetComponent
		@author		Isaiah Lim lim.i@digipen.edu

		@brief		Get the component from the GO

		[Warning]	If the typename is not registered in _component_manager,
					or the GO is deleted or the GO does not have the component,
					it will	cause an assert

		Example:
		if 
		*//*__________________________________________________________________________*/
		template <typename T>
		T& GetComponent();

		/* !
		@function	RemoveComponent
		@author		Isaiah Lim lim.i@digipen.edu

		@brief		Remove the component from the GO

		[Warning]	If the typename is not registered in _component_manager, it will
					cause an assert

		Example:

		GO goVar = _ecs_manager->CreateGO("goVar");
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

		@params		GO parent // GO for the new parent

		@brief		Abandons the previous parent if it exist
					Set the parent of the GO
					Automatically add this GO to the parent's children list

		Example:

		GO goVar = _ecs_manager->CreateGO("goVar");
		GO goParentVar = _ecs_manager->CreateGO("goParentVar");
		goVar->SetParent(goParentVar);
		*//*__________________________________________________________________________*/
		void SetParent(GO parent);

		/* !
		@function	GetParent
		@author		Isaiah Lim lim.i@digipen.edu

		@brief		Returns the parent of the GO
					Returns nullptr if it doesn't exist

		Example:

		GO goVar = _ecs_manager->CreateGO("goVar");
		GO goParentVar = _ecs_manager->CreateGO("goParentVar");
		goVar->SetParent(goParentVar);

		GO AccessGOVarParent = goVar->GetParent();
		*//*__________________________________________________________________________*/
		GO GetParent();

		/* !
		@function	RemoveParent
		@author		Isaiah Lim lim.i@digipen.edu

		@brief		Remove the parent of the GO
					Automatically Abandons this GO from the parent's children list

		Example:

		GO goVar = _ecs_manager->CreateGO("goVar");
		GO goParentVar = _ecs_manager->CreateGO("goParentVar");
		goVar->SetParent(goParentVar);

		goVar->RemoveParent();
		*//*__________________________________________________________________________*/
		void RemoveParent();

		/* !
		@function	AddChild
		@author		Isaiah Lim lim.i@digipen.edu

		@params		GO child // GO for the child

		@brief		Add a child to this GO children list
					Automatically set this GO as the parent of the child

		Example:

		GO goVar = _ecs_manager->CreateGO("goVar");
		GO goChildVar = _ecs_manager->CreateGO("goChildVar");

		goVar->AddChild(goChildVar);
		*//*__________________________________________________________________________*/
		void AddChild(GO child);

		/* !
		@function	GetChildren
		@author		Isaiah Lim lim.i@digipen.edu

		@brief		Returns the children of the GO

		Example:

		GO goVar = _ecs_manager->CreateGO("goVar");
		GO goParentVar = _ecs_manager->CreateGO("goParentVar");
		goVar->SetParent(goParentVar);

		std::vector<GO> goParentVarChildren = goParentVar->GetChildren();
		*//*__________________________________________________________________________*/
		std::vector<GO> GetChildren();

		/* !
		@function	AbandonChild
		@author		Isaiah Lim lim.i@digipen.edu

		@params		GO child // GO for the child

		@brief		Remove child from children list
					Automatically remove parent from the child GO
					Ignores command if child's parent is not this GO

		Example:

		GO goVar = _ecs_manager->CreateGO("goVar");
		GO goParentVar = _ecs_manager->CreateGO("goParentVar");
		goVar->SetParent(goParentVar);

		goParentVar->AbandonChild(goVar);
		*//*__________________________________________________________________________*/
		void AbandonChild(GO child);

		/* !
		@function	AbandonChildren
		@author		Isaiah Lim lim.i@digipen.edu

		@brief		Remove all children from children list
					Automatically remove parent from each child GO

		Example:

		GO goVar = _ecs_manager->CreateGO("goVar");
		GO goParentVar = _ecs_manager->CreateGO("goParentVar");
		goVar->SetParent(goParentVar);

		goParentVar->AbandonChildren();
		*//*__________________________________________________________________________*/
		void AbandonChildren();
		
	private:
		friend class ECSManager;

		GO m_Parent;
		std::vector<GO> m_Children;

		entt::entity m_Entity;
	};

	class ECSOutputArchive
	{
	public:
		void operator()(entt::entity ent)
		{
			std::cout << static_cast<std::underlying_type_t<entt::entity>>(ent) << "|";
		}
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
					This shouldn't be used, use _ecs_manager instead
		*//*__________________________________________________________________________*/
		static ECSManager& Instance();

		/* !
		@function	GetRegistry
		@author		Isaiah Lim lim.i@digipen.edu

		@brief		Returns entt registry in the _ecs_manager singleton
					This should ideally be used for _ecs_manager only

		Example:
		_ecs_manager->GetRegistry();
		*//*__________________________________________________________________________*/
		entt::registry& GetRegistry();

		// Engine Loop
		/* !
		@function	DestroyRemovalGO
		@author		Isaiah Lim lim.i@digipen.edu

		@brief		Remove all GO that has the component Removal
					This is only run under Engine.cpp at the end of the frame

		Example:
		_ecs_manager->DestroyRemovalGO();
		*//*__________________________________________________________________________*/
		void DestroyRemovalGO();

		// Shutdown Functions
		/* !
		@function	DestroyAll
		@author		Isaiah Lim lim.i@digipen.edu

		@brief		Destroys all the GOs

		Example:
		_ecs_manager->DestroyAll();
		*//*__________________________________________________________________________*/
		void DestroyAll();

		// Entity Controller
		/* !
		@function	CreateGO
		@author		Isaiah Lim lim.i@digipen.edu

		@params		std::string name	// Default name is "GameObject", can be renamed
										// name is found in Properties::m_Name

		@brief		Create a new GO
					Automatically adds the Properties Component

		Example:

		GO goVar = _ecs_manager->CreateGO("goVar");

		std::cout << goVar->HasComponent<Properties>() << std::endl;
		std::cout << goVar->GetComponent<Properties>().m_Name << std::endl;
		// Output
		// true
		// goVar
		*//*__________________________________________________________________________*/
		GO CreateGO(std::string name = "GameObject");

		/* !
		@function	DestroyGO
		@author		Isaiah Lim lim.i@digipen.edu

		@params		GO& object			// GO to be destroyed

		@brief		Create a new GO

		Example:

		GO goVar = _ecs_manager->CreateGO("goVar");

		std::cout << goVar << std::endl; // Address of goVar

		_ecs_manager->DestroyGO(goVar);
		*//*__________________________________________________________________________*/
		void DestroyGO(GO& object);

		/* !
		@function	CloneGO
		@author		Isaiah Lim lim.i@digipen.edu

		@params		GO& object			// GO to clone from
					std::string name	// Name for the cloned GO

		@brief		Create a new GO from an existing GO

		Example:

		GO goVar = _ecs_manager->CreateGO("goVar");
		GO clonGoVar = _ecs_manager->CloneGO(goVar, "ClonedGOName");
		*//*__________________________________________________________________________*/
		GO CloneGO(GO& object, std::string name = "Cloned_GameObject");

		/* !
		@function	GOHasComponent
		@author		Isaiah Lim lim.i@digipen.edu

		@params		GO& object			// GO to check from

		@brief		Check if the GO have a component

		Example:

		GO goVar = _ecs_manager->CreateGO("goVar");
		goVar->AddComponent<Transform>();

		std::cout << _ecs_manager->GOHasComponent<Transform>(goVar) << std::endl;
		std::cout << _ecs_manager->GOHasComponent<HUMAN>(goVar) << std::endl;

		// Output
		// true
		// false
		*//*__________________________________________________________________________*/
		template <typename T>
		bool GOHasComponent(GO object);

		/* !
		@function	GetGO
		@author		Isaiah Lim lim.i@digipen.edu

		@brief		Returns a vector of GO with the components listed

		Example:

		GO goVar = _ecs_manager->CreateGO("Object1");
		goVar->AddComponent<Transform>();
		goVar->AddComponent<Component>();

		GO goVar2 = _ecs_manager->CreateGO("Object2");

		GO goVar3 = _ecs_manager->CreateGO("Object3");
		goVar3->AddComponent<Component>();
		_ecs_manager->DestroyGO(goVar3);

		std::cout << "GO with Properties Size: " << _ecs_manager->GetGO<Properties>().size() << std::endl;
		for (GO& obj : _ecs_manager->GetGO<Properties>())
			std::cout << "- " << obj->GetComponent<Properties>().m_Name << std::endl;

		std::cout << "GO with Transform Size: " << _ecs_manager->GetGO<Transform>().size() << std::endl;
		for (GO& obj : _ecs_manager->GetGO<Transform>())
			std::cout << "- " << obj->GetComponent<Properties>().m_Name << std::endl;

		std::cout << "GO with Component Size: " << _ecs_manager->GetGO<Component>().size() << std::endl;
		for (GO& obj : _ecs_manager->GetGO<Component>())
			std::cout << "- " << obj->GetComponent<Properties>().m_Name << std::endl;

		std::cout << "GO with Removal Size: " << _ecs_manager->GetGO<Removal>().size() << std::endl;
		for (GO& obj : _ecs_manager->GetGO<Removal>())
			std::cout << "- " << obj->GetComponent<Properties>().m_Name << std::endl;

		std::cout << "GO with Transform and Component Size: " << _ecs_manager->GetGO<Transform, Component>().size() << std::endl;
		for (GO& obj : _ecs_manager->GetGO<Transform, Component>())
			std::cout << "- " << obj->GetComponent<Properties>().m_Name << std::endl;

		// Output
		// GO with Properties Size: 3
		// - Object1
		// - Object2
		// - Object3
		// GO with Transform Size: 3
		// - Object1
		// - Object2
		// - Object3
		// GO with Component Size: 2
		// - Object1
		// - Object3
		// GO with Removal Size: 1
		// - Object3
		// GO with Transform and Component and Removal Size: 1
		// - Object3
		*//*__________________________________________________________________________*/
		template <typename Comp, typename... Others>
		std::vector<GO> GetGO();

		// TODELETE
		void TESTRUN()
		{
			std::cout << "\nTEST RUNNING ECS\n====================================\n";

			std::cout << "Sizes: " << m_GOList.size() << "\n";
			for (auto&& storage : GetRegistry().storage())
				std::cout << "- " << storage.first << "|" << storage.second.size() << "\n";
			std::cout << "-------\n";

			GO test = CreateGO("Test 1");

			std::cout << "Sizes: " << m_GOList.size() << "\n";
			for (auto&& storage : GetRegistry().storage())
				std::cout << "- " << storage.first << "|" << storage.second.size() << "\n";
			std::cout << "-------\n";

			std::cout << "\n\nEntities IDs\n";
			for (GO& go : m_GOList)
			{
				std::cout << "> " << static_cast<std::uint32_t>(go->m_Entity) << "\n";
			}
			std::cout << "-- Storage --\n";
			for (auto&& id : GetRegistry().storage().begin()->second)
			{
				std::cout << "> " << static_cast<std::uint32_t>(id) << "\n";
			}
			std::cout << "++++++++++++\n";

			test->AddComponent<Transform>().m_Position.x = 19;
			std::cout << "Creating GO, Adding, Getting and editing a value: " << test->GetComponent<Transform>().m_Position.x << std::endl;
			std::cout << "Removing Editted Component...\n";
			test->RemoveComponent<Transform>();

			std::cout << "Sizes: " << m_GOList.size() << "\n";
			for (auto&& storage : GetRegistry().storage())
				std::cout << "- " << storage.first << "|" << storage.second.size() << "\n";
			std::cout << "-------\n";

			//std::cout << "Attempting to get a component it does not have: " << test->GetComponent<Transform>().m_Scale.x << std::endl; // Will call assert in GetComponent!
			std::cout << "Default Parent: " << test->GetParent() << "\n";
			DestroyGO(test);
			std::cout << "Destroyed earlier GO...\n";
			//std::cout << "Attempting to call a deleted/destroyed GO: " << test->GetComponent<Properties>().m_Name << std::endl; // Will not call assert in GetComponent until next loop!

			std::cout << "Sizes: " << m_GOList.size() << "\n";
			for (auto&& storage : GetRegistry().storage())
				std::cout << "- " << storage.first << "|" << storage.second.size() << "\n";
			std::cout << "-------\n";

			std::cout << "\n\nEntities IDs\n";
			for (GO& go : m_GOList)
			{
				std::cout << "> " << static_cast<std::uint32_t>(go->m_Entity) << "\n";
			}
			std::cout << "-- Storage --\n";
			for (auto&& id : GetRegistry().storage().begin()->second)
			{
				std::cout << "> " << static_cast<std::uint32_t>(id) << "\n";
			}
			std::cout << "++++++++++++\n";

			std::cout << "Creating GOs with 1 GO with only Properties and 2 GO with Transform and Properties...\n";
			GO test2 = CreateGO("test2");

			GO allobj = CreateGO("allobj");
			GO allobj2 = CreateGO("allobj2");

			std::cout << "Total GO with Transform: " << GetGO<Transform>().size() << "\n";
			std::cout << "Total GO with Properties: " << GetGO<Properties>().size() << "\n";
			std::cout << "Total GO with Transform and Properties: " << GetGO<Transform, Properties>().size() << "\n";

			std::cout << "Testing cloning GO...\n";
			std::cout << "- Setting Original GO value to 123...\n";
			GO oriobj = CreateGO("oriobj");
			oriobj->AddComponent<Transform>().m_Position.x = 123;
			std::cout << "- Cloning Original GO\n";
			GO cloneobj = CloneGO(oriobj);
			std::cout << "- Cloned GO value is " << cloneobj->GetComponent<Transform>().m_Position.x << "\n";
			std::cout << "- Setting Original GO value to 0...\n";
			oriobj->GetComponent<Transform>().m_Position.x = 0;
			std::cout << "- Cloned GO value is " << cloneobj->GetComponent<Transform>().m_Position.x << "\n";

			std::cout << "\nIterating All Available Component in ComponentManager\n";
			for (auto comp : _component_manager->m_Components)
			{
				std::cout << "- " << comp.second << "\n";
			}

			std::cout << "\nTesting iterating through All GO with Properties\n";
			for (auto go : GetGO<Properties>())
			{
				go->GetComponent<Properties>().m_Active = true;
			}
			std::cout << "- Testing Complete\n";

			std::cout << "Sizes: " << m_GOList.size() << "\n";
			for (auto&& storage : GetRegistry().storage())
				std::cout << "- " << storage.first << "|" << storage.second.size() << "\n";
			std::cout << "-------\n";

			std::cout << "\nTesting setting, getting and removing parent\n";
			GO parentGO = CreateGO();
			GO childGO = CreateGO();
			std::cout << "- Default childGO parent: " << childGO->GetParent() << "\n";
			std::cout << "- childGO address: " << childGO << "\n";
			std::cout << "- parentGO address: " << parentGO << "\n";
			std::cout << "- Setting parentGO as childGO parent...\n";
			childGO->SetParent(parentGO);
			std::cout << "- New childGO parent: " << childGO->GetParent() << "\n";
			std::cout << "- childGO children size: " << childGO->GetChildren().size() << "\n";
			std::cout << "- parentGO children size: " << parentGO->GetChildren().size() << "\n";
			std::cout << "- Removing childGO parent...\n";
			childGO->RemoveParent();
			std::cout << "- Removed childGO parent: " << childGO->GetParent() << "\n";
			std::cout << "- childGO children size: " << childGO->GetChildren().size() << "\n";
			std::cout << "- parentGO children size: " << parentGO->GetChildren().size() << "\n";
			std::cout << "- Setting childGO as childGO parent...\n";
			childGO->SetParent(childGO);
			std::cout << "- New childGO parent (Ideally it would set parent as a nullptr): " << childGO->GetParent() << "\n";
			std::cout << "- childGO children size: " << childGO->GetChildren().size() << "\n";
			std::cout << "- parentGO children size: " << parentGO->GetChildren().size() << "\n";
			std::cout << "\n- Adding 5 GOs to parentGO as children...\n";
			parentGO->AddChild(test2);
			parentGO->AddChild(allobj);
			parentGO->AddChild(allobj2);
			parentGO->AddChild(cloneobj);
			parentGO->AddChild(oriobj);
			std::cout << "- parentGO children size: " << parentGO->GetChildren().size() << "\n";
			std::cout << "- 1 GO removing parentGO...\n";
			test2->RemoveParent();
			std::cout << "- parentGO children size: " << parentGO->GetChildren().size() << "\n";
			std::cout << "- parentGO abandoning 1 children...\n";
			parentGO->AbandonChild(allobj);
			std::cout << "- parentGO children size: " << parentGO->GetChildren().size() << "\n";
			std::cout << "- 1 GO Setting another parent...\n";
			allobj2->SetParent(childGO);
			std::cout << "- parentGO children size: " << parentGO->GetChildren().size() << "\n";
			std::cout << "- parentGO abandoning all remaining children...\n";
			parentGO->AbandonChildren();
			std::cout << "- parentGO children size: " << parentGO->GetChildren().size() << "\n";
			std::cout << "- Attempting to remove a non child in parentGO...\n";
			parentGO->AbandonChild(test2);
			std::cout << "- Testing Complete\n";

			std::cout << "Sizes: " << m_GOList.size() << "\n";
			for (auto&& storage : GetRegistry().storage())
				std::cout << "- " << storage.first << "|" << storage.second.size() << "\n";
			std::cout << "-------\n";

			std::cout << "\nTesting Listener\n";
			registry.on_construct<Transform>().connect<&Transform::Init>();
			registry.on_update<Transform>().connect<&Transform::UpdateValues>();
			registry.on_destroy<Transform>().connect<&Transform::Destroy>();

			GO listenerGO = CreateGO("listenerGO");
			std::cout << "\nTesting Listening to Adding Component, should call Init\n";
			listenerGO->AddComponent<Transform>();
			std::cout << "\nTesting Listening to Changing Component Values, should call Update\n";
			listenerGO->GetComponent<Transform>().m_Position.x = 5; // This wont work
			listenerGO->GetComponent<Transform>().m_Position.y = 85; // This wont work
			std::cout << "Original Value: " << listenerGO->GetComponent<Transform>().m_Position.x << ", " << listenerGO->GetComponent<Transform>().m_Position.y << "\n";
			// replaces the component in-place
			//int newVal = 69;
			//GetRegistry().patch<Transform>(listenerGO->m_Entity, [&](Transform& pos) { pos.m_Position.x = newVal; });
			//GetRegistry().patch<Transform>(listenerGO->m_Entity, &Transform::SetPosX);
			std::cout << "New Value: " << listenerGO->GetComponent<Transform>().m_Position.x << ", " << listenerGO->GetComponent<Transform>().m_Position.y << "\n";
			std::cout << "\nTesting Listening to Destroying Values, should call Destroy\n";
			listenerGO->RemoveComponent<Transform>();

			std::cout << "\nDisconnecting Listeners...\n";
			registry.on_construct<Transform>().disconnect<&Transform::Init>();
			registry.on_update<Transform>().disconnect<&Transform::UpdateValues>();
			registry.on_destroy<Transform>().disconnect<&Transform::Destroy>();

			std::cout << "- Testing complete!\n";

			std::cout << "\nTesting observer noticing if any Transform change\n";
			entt::observer existingObserver{ registry, entt::collector.group<Transform>() };
			entt::observer updatedObserver{ registry, entt::collector.update<Transform>() };
			std::cout << "- Adding GO for observer to observe...\n";
			GO observerGO = CreateGO("ObserverGO");
			observerGO->AddComponent<Transform>();

			std::cout << "Existing Transform Observer Size: " << existingObserver.size() << "\n";

			std::cout << "Updated Transform Observer Size: " << updatedObserver.size() << "\n";

			std::cout << "\n- Testing if changing variables manually would affect\n";
			observerGO->GetComponent<Transform>().m_Position.x = 5;
			std::cout << "Updated Transform Observer Size: " << updatedObserver.size() << "\n";
			std::cout << "-- If size is same, it did not update\n";

			std::cout << "\n- Testing if changing variables using patch in entt would affect\n";
			GetRegistry().patch<Transform>(observerGO->m_Entity, [&](Transform& pos) { pos.m_Position.x = 5; });
			std::cout << "Updated Transform Observer Size: " << updatedObserver.size() << "\n";
			std::cout << "-- If size is same, it did not update\n";

			std::cout << "\n- Testing if changing variables using patch in entt on the same GO would cause dups\n";
			GetRegistry().patch<Transform>(observerGO->m_Entity, [&](Transform& pos) { pos.m_Position.x = 0; });
			std::cout << "Updated Transform Observer Size: " << updatedObserver.size() << "\n";
			std::cout << "-- If size is same, it did not dup\n";

			std::cout << "\n- Clearing observers size... (REMEMBER TO DISCONNECT FROM REGISTRY OR IT WILL CRASH ON SHUTDOWN!)\n";
			existingObserver.clear();
			updatedObserver.clear();
			existingObserver.disconnect();
			updatedObserver.disconnect();
			std::cout << "- Testing complete!\n";

			std::cout << "\nTrying out snapshot for archiving entities\n";
			std::cout << "- Total Objects: " << GetGO<Properties>().size() << "...\n";

			std::cout << "- Archiving to Output: " << GetGO<Properties>().size() << "...\n";
			ECSOutputArchive str{};
			entt::snapshot snapshot{ GetRegistry() };
			snapshot.entities(str);
			//snapshot.component<>(str);

			std::cout << "\n\nEntities IDs\n";
			for (GO& go : m_GOList)
			{
				std::cout << "> " << static_cast<std::uint32_t>(go->m_Entity) << "\n";
			}
			std::cout << "-- Storage --\n";
			for (auto&& id : GetRegistry().storage().begin()->second)
			{
				std::cout << "> " << static_cast<std::uint32_t>(id) << "\n";
			}
			std::cout << "++++++++++++\n";

			std::cout << "\n- Destroy All...\n";
			DestroyAll();
			std::cout << "- Current: " << GetGO<Properties>().size() << "...\n";

			std::cout << "Sizes: " << m_GOList.size() << "\n";
			for (auto&& storage : GetRegistry().storage())
				std::cout << "- " << storage.first << "|" << storage.second.size() << "\n";
			std::cout << "-------\n";

			std::cout << "\n\nEntities IDs\n";
			for (GO& go : m_GOList)
			{
				std::cout << "> " << static_cast<std::uint32_t>(go->m_Entity) << "\n";
			}
			std::cout << "-- Storage --\n";
			for (auto&& id : GetRegistry().storage().begin()->second)
			{
				std::cout << "> " << static_cast<std::uint32_t>(id) << "\n";
			}
			std::cout << "++++++++++++\n";
			/*ECSInputArchive instr{};
			entt::snapshot_loader snapshotLoader{ GetRegistry() };
			snapshotLoader.entities(instr);*/
			//snapshotLoader.component<>(str);

			std::cout << "\nDestroying all " << GetGO<Properties>().size() << "  test objects...\n";
			DestroyAll();
			std::cout << "- Remaining: " << GetGO<Properties>().size() << " | Successfully cleared: " << (GetGO<Properties>().empty() ? "true" : "false") << "\n";

			std::cout << "====================================\n\n";
		}

	private:
		// Delete possible copy ctor and assignment to ensure singleton
		ECSManager() {};
		ECSManager(ECSManager const&) = delete;
		void operator=(ECSManager const&) = delete;
		void* operator new(size_t) = delete;

		// EnTT stuff
		entt::registry registry;

		// Component Types
		std::map<size_t, void*> componentTypes;

		std::vector<GO> m_GOList;
	};
	static ECSManager* _ecs_manager{ &ECSManager::Instance() };


	template <typename Comp, typename... Others>
	std::vector<GO> ECSManager::GetGO()
	{
		std::vector<GO> objects{};
		auto view = registry.view<Comp, Others...>();

		// Get all GO owning the entities
		for (entt::entity obj : view)
		{
			auto it = std::find_if(m_GOList.begin(), m_GOList.end(), [&](GO& go) { return go->m_Entity == obj; });
			if (it != m_GOList.end())
			{
				objects.emplace_back(*it);
			}
		}

		return objects;
	}

	template <typename T>
	bool ECSManager::GOHasComponent(GO object)
	{
		return registry.any_of<T>(object->m_Entity);
	}

	template <typename T>
	bool GameObject::HasComponent()
	{
		if (!_component_manager->HasComponent<T>() && !_component_manager->HasHiddenComponent<T>())
		{
			std::string funcName{ __FUNCTION__ };
			std::string compName{ typeid(T).name() };
			//TRE_CORE_ERROR("[" + funcName + "] Component " + compName + " is not included in _component_manager");
			assert(_component_manager->HasComponent<T>() || _component_manager->HasHiddenComponent<T>());
		}
		return _ecs_manager->GOHasComponent<T>(shared_from_this());
	}

	template <typename T>
	T& GameObject::AddComponent()
	{
		if (!_component_manager->HasComponent<T>() && !_component_manager->HasHiddenComponent<T>())
		{
			std::string funcName{ __FUNCTION__ };
			std::string compName{ typeid(T).name() };
			//TRE_CORE_ERROR("[" + funcName + "] Component " + compName + " is not included in _component_manager");
			assert(_component_manager->HasComponent<T>() || _component_manager->HasHiddenComponent<T>());
		}

		if (HasComponent<T>())
		{
			std::string funcName{ __FUNCTION__ };
			std::string compName{ typeid(T).name() };
			//TRE_CORE_ERROR("[" + funcName + "] Component " + compName + " is already in " + GetComponent<Properties>().m_Name + "...");
			return GetComponent<T>();
		}
		return _ecs_manager->GetRegistry().emplace<T>(m_Entity);
	}

	template <typename T>
	T& GameObject::GetComponent()
	{
		// Ensure cannot get a component from a freed object and entity
		if (this == nullptr || &m_Entity == nullptr)
		{
			std::string funcName{ __FUNCTION__ };
			//TRE_CORE_ERROR("[" + funcName + "] " + GetComponent<Properties>().m_Name + " is no longer valid (this or entity is nullptr)");
			assert(this != nullptr);
			assert(&m_Entity != nullptr);
		}

		if (!_component_manager->HasComponent<T>() && !_component_manager->HasHiddenComponent<T>())
		{
			std::string funcName{ __FUNCTION__ };
			std::string compName{ typeid(T).name() };
			//TRE_CORE_ERROR("[" + funcName + "] Component " + compName + " is not included in _component_manager");
			assert(_component_manager->HasComponent<T>() || _component_manager->HasHiddenComponent<T>());
		}

		if (!HasComponent<T>())
		{
			std::string funcName{ __FUNCTION__ };
			std::string compName{ typeid(T).name() };
			//TRE_CORE_ERROR("[" + funcName + "] " + GetComponent<Properties>().m_Name + " does not have the component " + compName);
			assert(HasComponent<T>());
		}

		return _ecs_manager->GetRegistry().get<T>(m_Entity);
	}

	template <typename T>
	void GameObject::RemoveComponent()
	{
		// Ensure cannot get a component from a freed object and entity
		if (this == nullptr || &m_Entity == nullptr)
		{
			std::string funcName{ __FUNCTION__ };
			//TRE_CORE_ERROR("[" + funcName + "] " + GetComponent<Properties>().m_Name + " is no longer valid (this or entity is nullptr)");
			assert(this != nullptr);
			assert(&m_Entity != nullptr);
		}

		if (!_component_manager->HasComponent<T>() && !_component_manager->HasHiddenComponent<T>())
		{
			std::string funcName{ __FUNCTION__ };
			std::string compName{ typeid(T).name() };
			//TRE_CORE_ERROR("[" + funcName + "] Component " + compName + " is not included in _component_manager");
			assert(_component_manager->HasComponent<T>() || _component_manager->HasHiddenComponent<T>());
		}

		if (HasComponent<T>())
			_ecs_manager->GetRegistry().remove<T>(m_Entity);
	}
}