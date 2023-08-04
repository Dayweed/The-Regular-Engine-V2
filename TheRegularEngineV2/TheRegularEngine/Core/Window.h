#pragma once

#include "GLFW/glfw3.h"
#include "Graphics/RendererContext.h"
#include "Graphics/SwapChain.h"

namespace TRE
{
	struct WindowConfig
	{
		std::string Title = "TRE";
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

			GLFWwindow* GetWindowHandle() const;
			const WindowConfig& GetWindowConfig() const;
			int ShouldWindowClose();

		private:
			GLFWwindow* m_WindowHandle = nullptr;
			WindowConfig m_Config;
			std::unique_ptr<RendererContext> m_RenderContext;
			SwapChain m_SwapChain;
	};
}