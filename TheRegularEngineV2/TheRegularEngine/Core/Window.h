#pragma once

#include "GLFW/glfw3.h"
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
			void SwapBuffers();

			GLFWwindow* GetWindowHandle() const;
			const WindowConfig& GetWindowConfig() const;
			std::shared_ptr<RendererContext> GetRenderContext();
			SwapChain GetSwapChain();

		private:
			GLFWwindow* m_WindowHandle = nullptr;
			WindowConfig m_Config;

			std::shared_ptr<RendererContext> m_RenderContext;
			SwapChain m_SwapChain;
	};
}