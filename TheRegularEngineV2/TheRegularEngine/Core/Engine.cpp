#include "pch.h"
#include "Engine.h"
#include "LayerSystem.h"
#include "Scripting/ScriptEngine.h"
#include "Physics/PhysicsSystem.h"
#include "entt.hpp"

namespace TRE
{
	Engine* Engine::s_Instance = nullptr;

	Engine::Engine()
	{
		s_Instance = this;
		m_Window = std::make_unique<Window>();
		m_LayerSystems = LayerSystem::GetInstance();

		AddSystem(new PhysicsSystem);
		//ScriptEngine::InitMono();
	}

	Engine::~Engine()
	{
		
	}

	void Engine::Update()
	{
		while (!m_Window->ShouldWindowClose())
		{
			m_Window->PollEvents();

			m_LayerSystems->UpdateSystems();
		}
	}

	void Engine::Shutdown()
	{
		m_LayerSystems->ShutdownSystems();
		LayerSystem::ShutDownLayerSystem();
	}

	void Engine::AddSystem(Layer* system)
	{
		m_LayerSystems->AddSystem(system);
	}
}