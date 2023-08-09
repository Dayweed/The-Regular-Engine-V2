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

	Engine::Engine(const EngineInfo& EngineInfo)
	{
		s_Instance = this;
		m_EngineInfo = EngineInfo;
		m_Window = std::make_shared<Window>(m_EngineInfo.WindowConfigurations);
		m_SystemsManager = std::make_unique<SystemManager>();
		
		if (m_EngineInfo.EnableEditor)
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
			
			m_Window->GetSwapChain().BeginFrame();
			
			if (m_EngineInfo.EnableEditor)
			{
				m_VulkanEditor->BeginFrame();
				m_SystemsManager->UpdateSystem();
				m_SystemsManager->RenderImgui();
				m_VulkanEditor->EndFrame();
			}
			else
			{
				m_SystemsManager->UpdateSystem();
			}

			m_Window->SwapBuffers();
		}
	}

	void Engine::Shutdown()
	{
		m_SystemsManager->ShutdownSystem();
	}
}