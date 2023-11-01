#pragma once
#include "Window.h"
#include "SystemManager.h"
#include "Graphics/VulkanEditor.h"
#include "Graphics/Renderer.h"
#include "Graphics/SceneRenderer.h"

namespace TRE
{
	struct EngineInfo
	{
		WindowConfig WindowConfigurations;
		bool EnableEditor = false;
		bool EnableGame = false;
		bool MaximizeWindow = false;
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
			const std::shared_ptr<VulkanEditor>& GetVulkanImgui();
			const EngineInfo& GetEngineInfo();
			static Engine& GetInstance();
			const std::shared_ptr<SceneRenderer>& GetMainSceneRenderer();
			const std::shared_ptr<SceneRenderer>& GetEditorSceneRenderer();

		protected:
			Engine(const EngineInfo& EngineInfo = EngineInfo());

		private:
			std::shared_ptr<Window> m_Window;
			std::shared_ptr<VulkanEditor> m_VulkanEditor;
			std::shared_ptr<SceneRenderer> m_SceneRenderer;
			std::shared_ptr<SceneRenderer> m_EditorSceneRenderer;
			EngineInfo m_EngineInfo;
			bool m_Running = true;

			static Engine* s_Instance;
	};

	Engine* CreateApp(); //Implemented by applications projects, used to seperate applications with engine
}