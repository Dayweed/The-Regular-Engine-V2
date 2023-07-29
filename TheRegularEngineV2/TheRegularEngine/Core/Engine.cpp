#include "pch.h"
#include "Engine.h"
#include "Physics/PhysicsSystem.h"

namespace TRE
{
	Engine* Engine::s_Instance = nullptr;

	Engine::Engine()
	{
		s_Instance = this;
		m_Window = std::make_unique<Window>();
		m_SystemsManager = std::make_unique<SystemManager>();
		RegisterSystems<PhysicsSystem>();
	}

	Engine::~Engine()
	{
		
	}

	void Engine::Update()
	{
		while (!m_Window->ShouldWindowClose())
		{
			m_Window->PollEvents();

			m_SystemsManager->UpdateSystem();
			m_SystemsManager->RenderImgui();
		}
	}

	void Engine::Shutdown()
	{
		m_SystemsManager->ShutdownSystem();
	}
}