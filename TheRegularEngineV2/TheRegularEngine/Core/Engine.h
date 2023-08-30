#pragma once
#include "pch.h"
#include "Window.h"
#include "SystemManager.h"
#include "Graphics/VulkanEditor.h"
#include "Graphics/Renderer.h"

namespace TRE
{
	struct EngineInfo
	{
		WindowConfig WindowConfigurations;
		bool EnableEditor = false;
	};

	class Engine
	{
		public:
			virtual ~Engine();

			Engine(Engine&) = delete;
			void operator=(const Engine&) = delete;

			void RegisterECS();
			void Update();
			virtual void Shutdown();

			template <typename T>
			void RegisterSystems()
			{
				_system_manager->RegisterSystem<T>();
			}

			const std::shared_ptr<Window>& GetWindow();
			const std::shared_ptr<Renderer>& GetRenderer();
			const std::shared_ptr<VulkanEditor>& GetVulkanImgui();
			static Engine& GetInstance();

		protected:
			Engine(const EngineInfo& EngineInfo = EngineInfo());

		private:
			std::shared_ptr<Window> m_Window;
			//std::unique_ptr<SystemManager> m_SystemsManager;
			std::shared_ptr<Renderer> m_Renderer;
			std::shared_ptr<VulkanEditor> m_VulkanEditor;

			EngineInfo m_EngineInfo;

			static Engine* s_Instance;
	};

	Engine* CreateApp(); //Implemented by applications projects, used to seperate applications with engine
}