#pragma once

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include "Graphics/RendererContext.h"
#include "Graphics/SwapChain.h"

namespace TRE
{
	class Window
	{
		public:
			Window();
			~Window();

			//RAII
			Window(const Window&) = delete;
			Window& operator=(const Window&) = delete;

			void PollEvents();

			GLFWwindow* GetWindowHandle();
			int ShouldWindowClose();

		private:
			GLFWwindow* m_WindowHandle = nullptr;
			std::unique_ptr<RendererContext> m_RenderContext;
			SwapChain m_SwapChain;
	};
}