#pragma once
#include "Window.h"
#include "SystemManager.h"
#include "Graphics/Renderer.h"
#include "Graphics/VulkanEditor.h"

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
				EditorSystemManager::Instance().RegisterSystem<T>();
			}

			const std::shared_ptr<Window>& GetWindow();
			const std::shared_ptr<Renderer>& GetRenderer();
			const std::shared_ptr<VulkanEditor>& GetVulkanImgui();
			static Engine& GetInstance();

		protected:
			Engine(const EngineInfo& EngineInfo = EngineInfo());

		private:
			std::shared_ptr<Window> m_Window;
			std::shared_ptr<Renderer> m_Renderer;
			std::shared_ptr<VulkanEditor> m_VulkanEditor;
			EngineInfo m_EngineInfo;
			bool m_Running = true;

			static Engine* s_Instance;
	};

	Engine* CreateApp(); //Implemented by applications projects, used to seperate applications with engine
}