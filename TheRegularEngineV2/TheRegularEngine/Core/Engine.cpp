#include "pch.h"
#include "Engine.h"
#include "Physics/PhysicsSystem.h"

namespace TRE
{
	Engine* Engine::s_Instance = nullptr;

	std::shared_ptr<Window> Engine::GetWindow()
	{
		return m_Window;
	}

	Engine& Engine::GetInstance()
	{
		return *s_Instance;
	}

	Engine::Engine()
	{
		s_Instance = this;
		m_Window = std::make_shared<Window>(WindowConfig());
		m_SystemsManager = std::make_unique<SystemManager>();
		m_VulkanEditor = std::make_shared<VulkanEditor>(m_Window->GetRenderContext()->GetDeviceInternally());
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

			m_Window->GetSwapChain().BeginFrame();
			m_VulkanEditor->Update();

			m_Window->SwapBuffers();
		}
	}

	void Engine::Shutdown()
	{
		m_SystemsManager->ShutdownSystem();
	}
}