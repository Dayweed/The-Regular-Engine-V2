#pragma once
#include "pch.h"
#include "entt.hpp"
#include "System.h"
#include "ComponentManager.h"
#include <typeindex>

namespace TRE
{
	class Properties : Component
	{
	public:
		std::string m_Name;
		bool active;
	};

	class Position : Component
	{
	public:
		float x;
		float y;
		float z;
	};

	class GameObject : public std::enable_shared_from_this<GameObject>
	{
	public:
		template <typename T>
		bool HasComponent();

		template <typename T>
		T& AddComponent();

		template <typename T>
		T& GetComponent();
		
	private:
		friend class ECSManager;

		entt::entity entity;
	};

	typedef std::shared_ptr<GameObject> GO;

	// ECS Manager (Entity Manager) THERE CAN ONLY BE ONE! >:o
	//==================================================
	class ECSManager
	{
	public:
		static ECSManager& Instance();

		entt::registry& GetRegistry();

		// Shutdown Functions
		void DestroyAll();

		// Entity Controller
		GO CreateGO();
		void DestroyGO(GO& object);
		GO CloneGO(GO& object);

		template <typename T>
		bool GOHasComponent(entt::entity entity);

		template <typename Comp, typename... Others>
		std::vector<GO> GetGO();

		// TODELETE
		void TESTRUN()
		{
			std::cout << "\nTEST RUNNING ECS\n====================================\n";

			GO test = CreateGO();
			test->AddComponent<Position>().x = 19;
			std::cout << "Creating GO, Adding, Getting and editing a value: " << test->GetComponent<Position>().x << std::endl;
			//std::cout << "Attempting to get a component it does not have: " << test->GetComponent<RANDOCOMP>().j << std::endl; // Will call assert in GetComponent!
			DestroyGO(test);
			std::cout << "Destroyed earlier GO...\n";
			//std::cout << "Attempting to call a deleted/destroyed GO: " << test->GetComponent<Position>().x << std::endl; // Will call assert in GetComponent!

			std::cout << "Creating GOs with 1 GO with only Properties and 2 GO with Position and Properties...\n";
			GO test2 = CreateGO();

			GO allobj = CreateGO();
			allobj->AddComponent<Position>();

			GO allobj2 = CreateGO();
			allobj2->AddComponent<Position>();

			std::cout << "Total GO with Position: " << GetGO<Position>().size() << "\n";
			std::cout << "Total GO with RANDOCOMP: " << GetGO<Properties>().size() << "\n";
			std::cout << "Total GO with Position and RANDOCOMP: " << GetGO<Position, Properties>().size() << "\n";

			std::cout << "Testing cloning GO...\n";
			std::cout << "- Setting Original GO value to 123...\n";
			GO oriobj = CreateGO();
			oriobj->AddComponent<Position>().x = 123;
			std::cout << "- Cloning Original GO\n";
			GO cloneobj = CloneGO(oriobj);
			std::cout << "- Cloned GO value is " << cloneobj->GetComponent<Position>().x << "\n";

			std::cout << "\nIterating All Available Component in ComponentManager\n";
			for (auto comp : _component_manager->m_Components)
			{
				std::cout << "- " << comp.second << "\n";
			}

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

		std::vector<GO> GOList;
	};
	static ECSManager* _ecs_manager{ &ECSManager::Instance() };


	template <typename Comp, typename... Others>
	std::vector<GO> ECSManager::GetGO()
	{
		std::vector<GO> objects{};
		auto view = registry.view<Comp>();

		// Get all GO owning the entities
		for (entt::entity obj : view)
		{
			auto it = std::find_if(GOList.begin(), GOList.end(), [&](GO& go) { return go->entity == obj; });
			if (it != GOList.end())
			{
				objects.emplace_back(*it);
			}
		}

		return objects;
	}

	template <typename T>
	bool ECSManager::GOHasComponent(entt::entity entity)
	{
		return registry.any_of<T>(entity);
	}

	template <typename T>
	bool GameObject::HasComponent()
	{
		assert(_component_manager->HasComponent<T>());
		return _ecs_manager->GOHasComponent<T>(entity);
	}

	template <typename T>
	T& GameObject::AddComponent()
	{
		assert(_component_manager->HasComponent<T>());

		if (HasComponent<T>())
		{
			return GetComponent<T>();
		}
		return _ecs_manager->GetRegistry().emplace<T>(entity);
	}

	template <typename T>
	T& GameObject::GetComponent()
	{
		// Ensure cannot get a component from a freed object and entity
		assert(&entity != nullptr);
		assert(_component_manager->HasComponent<T>());

		return _ecs_manager->GetRegistry().get<T>(entity);
	}
}