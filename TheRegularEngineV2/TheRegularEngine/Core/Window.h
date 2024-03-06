#pragma once
#include "Graphics/RendererContext.h"
#include "Graphics/SwapChain.h"

namespace TRE
{
	struct WindowConfig
	{
		std::string Title = "The Regular Engine";
		uint32_t width = 1600;
		uint32_t height = 900;
		bool Vsync = true;
		bool resize = false;
		bool FullScreen = false;
	};

	class Window
	{
		public:
			Window(const WindowConfig& config = WindowConfig());
			~Window();

			//RAII
			Window(const Window&) = delete;
			Window& operator=(const Window&) = delete;

			void PollEvents();
			int ShouldWindowClose();
			void BeginFrame();
			void SwapBuffers();
			void UpdateDeltaTime();
			void MaximizeWindow();

			GLFWwindow* GetWindowHandle() const;
			WindowConfig& GetWindowConfig();
			std::shared_ptr<RendererContext>& GetRenderContext();
			std::shared_ptr<SwapChain>& GetSwapChain();
			float GetDeltaTime() const;
			bool IsFocused() const;

		private:
			GLFWwindow* m_WindowHandle = nullptr;
			WindowConfig m_Config;
			
			std::shared_ptr<RendererContext> m_RenderContext;
			std::shared_ptr<SwapChain> m_SwapChain;
			float m_DeltaTime = 0.0f;

			bool m_IsFocused = false;
			bool m_WasFocused = false; // Flag to track whether it was minimized in the previous loop
	};
}