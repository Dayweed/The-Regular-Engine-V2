#pragma once
#include "pch.h"
#include "entt.hpp"
#include "System.h"

namespace TRE
{
	struct TESTUpdate
	{
		int j;
		void Update()
		{
			std::cout << "UPDATE-";
		}
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

	typedef std::unique_ptr<GameObject> GO;

	// ECS Manager (Entity Manager) THERE CAN ONLY BE ONE! >:o
	//==================================================
	class ECSManager
	{
	public:
		static ECSManager& Instance();

		entt::registry& GetRegistry();

		// Runtime Functions
		void UpdateAll();

		// Shutdown Functions
		void DestroyAll();

		// Entity Controller
		GO CreateGO();
		void DestroyGO(GO& object);

		// TODELETE
		void TESTRUN()
		{
			GO test = CreateGO();
			test->AddComponent<Position>().x = 19;
			std::cout << test->GetComponent<Position>().x << std::endl;
			DestroyGO(test);

			GO test2 = CreateGO();
			test2->AddComponent<TESTUpdate>();
		}

	private:
		// Delete possible copy ctor and assignment to ensure singleton
		ECSManager() {};
		ECSManager(ECSManager const&) = delete;
		void operator=(ECSManager const&) = delete;
		void* operator new(size_t) = delete;

		// EnTT stuff
		entt::registry registry;
	};
	static ECSManager* _ecs_manager{ &ECSManager::Instance() };


	template <typename T>
	T& GameObject::AddComponent()
	{
		return _ecs_manager->GetRegistry().emplace<T>(entity);
	}

	template <typename T>
	T& GameObject::GetComponent()
	{
		return _ecs_manager->GetRegistry().get<T>(entity);
	}
}