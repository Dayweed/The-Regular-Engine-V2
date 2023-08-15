#pragma once
#include "pch.h"
#include "entt.hpp"
#include "System.h"
#include <typeindex>

namespace TRE
{
	struct RANDOCOMP
	{
		int j;
	};

	struct Position
	{
		float x;
		float y;
		float z;
	};

	class GameObject
	{
	public:
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

		template <typename Comp, typename... Others>
		std::vector<GO> GetGO();

		// TODELETE
		void TESTRUN()
		{
			std::cout << "\nTEST RUNNING ECS\n====================================\n";

			GO test = CreateGO();
			test->AddComponent<Position>().x = 19;
			std::cout << "Creating GO, Adding, Getting and editing a value: " << test->GetComponent<Position>().x << std::endl;
			DestroyGO(test);
			std::cout << "Destroyed earlier GO...\n";
			//std::cout << "Attempting to call a deleted/destroyed GO: " << test->GetComponent<Position>().x << std::endl; // Will call assert in GetComponent!

			std::cout << "Creating GOs with 1 GO with only RANDOCOMP and 2 GO with Position and RANDOCOMP...\n";
			GO test2 = CreateGO();
			test2->AddComponent<RANDOCOMP>();

			GO allobj = CreateGO();
			allobj->AddComponent<Position>();
			allobj->AddComponent<RANDOCOMP>();

			GO allobj2 = CreateGO();
			allobj2->AddComponent<Position>();
			allobj2->AddComponent<RANDOCOMP>();

			std::cout << "Total GO with Position: " << GetGO<Position>().size() << "\n";
			std::cout << "Total GO with RANDOCOMP: " << GetGO<RANDOCOMP>().size() << "\n";
			std::cout << "Total GO with Position and RANDOCOMP: " << GetGO<Position, RANDOCOMP>().size() << "\n";

			std::cout << "Testing cloning GO...\n";
			std::cout << "- Setting Original GO value to 123...\n";
			test2->GetComponent<RANDOCOMP>().j = 123;
			std::cout << "- Cloning Original GO\n";
			GO cloneobj = CloneGO(test2);
			std::cout << "- Cloned GO value is " << cloneobj->GetComponent<RANDOCOMP>().j << "\n";

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
	T& GameObject::AddComponent()
	{
		return _ecs_manager->GetRegistry().emplace<T>(entity);
	}

	template <typename T>
	T& GameObject::GetComponent()
	{
		// Ensure cannot get a component from a freed object and entity
		assert(&entity != nullptr);

		return _ecs_manager->GetRegistry().get<T>(entity);
	}
}