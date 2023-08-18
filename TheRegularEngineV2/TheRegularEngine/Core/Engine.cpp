#include "pch.h"
#include "Engine.h"
#include "Physics/PhysicsSystem.h"

namespace TRE
{
	Engine* Engine::s_Instance = nullptr;

	const std::shared_ptr<Window>& Engine::GetWindow()
	{
		return m_Window;
	}

	Engine& Engine::GetInstance()
	{
		return *s_Instance;
	}

	const std::shared_ptr<Renderer>& Engine::GetRenderer()
	{
		return m_Renderer;
	}

	const std::shared_ptr<VulkanEditor>& Engine::GetVulkanImgui()
	{
		return m_VulkanEditor;
	}

	Engine::Engine(const EngineInfo& EngineInfo)
	{
		s_Instance = this;
		m_EngineInfo = EngineInfo;
		m_Window = std::make_shared<Window>(m_EngineInfo.WindowConfigurations);
		m_SystemsManager = std::make_unique<SystemManager>();
		
		m_Renderer = std::make_shared<Renderer>(m_Window->GetRenderContext()->GetDevice());
		m_Renderer->Initialize();

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
			m_Renderer->BeginFrame();

			if (m_EngineInfo.EnableEditor)
			{
				m_VulkanEditor->BeginFrame();
				m_SystemsManager->UpdateSystem();
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