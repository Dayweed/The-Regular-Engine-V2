#include "pch.h"
#include "ECS.h"
#include "Engine.h"
#include "Physics/PhysicsSystem.h"

namespace TRE
{
	Engine* Engine::s_Instance = nullptr;

	Engine::Engine()
	{
		s_Instance = this;
		m_Window = std::make_unique<Window>();
	}

	Engine::~Engine()
	{
		
	}

	void Engine::RegisterECS()
	{
		// Register Component
		_component_manager->RegisterComponent<Properties>("Properties");
		_component_manager->RegisterComponent<Transform>("Transform");

		// Register System
		_system_manager->RegisterSystem<PhysicsSystem>();
	}

	void Engine::Update()
	{
		_ecs_manager->TESTRUN();
		while (!m_Window->ShouldWindowClose())
		{
			m_Window->PollEvents();
			_system_manager->UpdateSystem();
			_system_manager->RenderImgui();
		}
	}

	void Engine::Shutdown()
	{
		_system_manager->ShutdownSystem();
		_ecs_manager->DestroyAll();
	}
}