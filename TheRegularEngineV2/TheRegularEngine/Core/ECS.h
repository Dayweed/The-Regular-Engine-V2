#pragma once
#include "pch.h"
#include "entt.hpp"
#include "System.h"
#include "ComponentManager.h"
#include <typeindex>

namespace TRE
{
	// DO NOT USE THIS UNLESS YOU WANT THE GO TO BE DELETED!
	class Removal
	{
		bool fake; //This value is to ensure it can compile and be registered
	};

	class Properties
	{
	public:
		std::string m_Name;
		bool m_Active;
	};

	class Transform
	{
	public:
		float m_PosX;
		float m_PosY;
		float m_PosZ;

		float m_ScaleX;
		float m_ScaleY;
		float m_ScaleZ;
	};

	class GameObject;
	typedef std::shared_ptr<GameObject> GO;

	class GameObject : public std::enable_shared_from_this<GameObject>
	{
	public:
		GO GetThis();

		template <typename T>
		bool HasComponent();

		template <typename T>
		T& AddComponent();

		template <typename T>
		T& GetComponent();

		template <typename T>
		void RemoveComponent();

		void SetParent(GO parent);
		GO GetParent();
		void RemoveParent();

		void AddChild(GO child);
		std::vector<GO> GetChildren();
		void AbandonChild(GO child);
		void AbandonChildren();
		
	private:
		friend class ECSManager;

		GO m_Parent;
		std::vector<GO> m_Children;

		entt::entity m_Entity;
	};

	// ECS Manager (Entity Manager) THERE CAN ONLY BE ONE! >:o
	//==================================================
	class ECSManager
	{
	public:
		static ECSManager& Instance();

		entt::registry& GetRegistry();

		// Engine Loop
		void DestroyRemovalGO();

		// Shutdown Functions
		void DestroyAll();

		// Entity Controller
		GO CreateGO(std::string name = "GameObject");
		void DestroyGO(GO& object);
		GO CloneGO(GO& object, std::string name = "Cloned_GameObject");

		template <typename T>
		bool GOHasComponent(GO object);

		template <typename Comp, typename... Others>
		std::vector<GO> GetGO();

		// TODELETE
		void TESTRUN()
		{
			std::cout << "\nTEST RUNNING ECS\n====================================\n";

			GO test = CreateGO("Test 1");
			test->AddComponent<Transform>().m_PosX = 19;
			std::cout << "Creating GO, Adding, Getting and editing a value: " << test->GetComponent<Transform>().m_PosX << std::endl;
			std::cout << "Removing Editted Component...\n";
			test->RemoveComponent<Transform>();
			//std::cout << "Attempting to get a component it does not have: " << test->GetComponent<Transform>().m_PosX << std::endl; // Will call assert in GetComponent!
			std::cout << "Default Parent: " << test->GetParent() << "\n";
			DestroyGO(test);
			std::cout << "Destroyed earlier GO...\n";
			//std::cout << "Attempting to call a deleted/destroyed GO: " << test->GetComponent<Transform>().m_PosX << std::endl; // Will call assert in GetComponent!

			std::cout << "Creating GOs with 1 GO with only Properties and 2 GO with Transform and Properties...\n";
			GO test2 = CreateGO("test2");

			GO allobj = CreateGO("allobj");
			allobj->AddComponent<Transform>();

			GO allobj2 = CreateGO("allobj2");
			allobj2->AddComponent<Transform>();

			std::cout << "Total GO with Transform: " << GetGO<Transform>().size() << "\n";
			std::cout << "Total GO with RANDOCOMP: " << GetGO<Properties>().size() << "\n";
			std::cout << "Total GO with Transform and RANDOCOMP: " << GetGO<Transform, Properties>().size() << "\n";

			std::cout << "Testing cloning GO...\n";
			std::cout << "- Setting Original GO value to 123...\n";
			GO oriobj = CreateGO("oriobj");
			oriobj->AddComponent<Transform>().m_PosX = 123;
			std::cout << "- Cloning Original GO\n";
			GO cloneobj = CloneGO(oriobj);
			std::cout << "- Cloned GO value is " << cloneobj->GetComponent<Transform>().m_PosX << "\n";
			std::cout << "- Setting Original GO value to 0...\n";
			oriobj->GetComponent<Transform>().m_PosX = 0;
			std::cout << "- Cloned GO value is " << cloneobj->GetComponent<Transform>().m_PosX << "\n";

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
		assert(_component_manager->HasComponent<T>() || _component_manager->HasHiddenComponent<T>());
		return _ecs_manager->GOHasComponent<T>(shared_from_this());
	}

	template <typename T>
	T& GameObject::AddComponent()
	{
		assert(_component_manager->HasComponent<T>() || _component_manager->HasHiddenComponent<T>());

		if (HasComponent<T>())
		{
			return GetComponent<T>();
		}
		return _ecs_manager->GetRegistry().emplace<T>(m_Entity);
	}

	template <typename T>
	T& GameObject::GetComponent()
	{
		// Ensure cannot get a component from a freed object and entity
		assert(this != nullptr);
		assert(&m_Entity != nullptr);
		assert(_component_manager->HasComponent<T>() || _component_manager->HasHiddenComponent<T>());

		return _ecs_manager->GetRegistry().get<T>(m_Entity);
	}

	template <typename T>
	void GameObject::RemoveComponent()
	{
		// Ensure cannot get a component from a freed object and entity
		assert(this != nullptr);
		assert(&m_Entity != nullptr);
		assert(_component_manager->HasComponent<T>() || _component_manager->HasHiddenComponent<T>());

		_ecs_manager->GetRegistry().remove<T>(m_Entity);
	}
}