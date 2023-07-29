#include "pch.h"
#include "Window.h"

namespace TRE
{
	Window::Window()
	{
		if (!glfwInit())
		{
			//Assert
		}

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); //Default is opengl so we set to no since we going V

		m_WindowHandle = glfwCreateWindow(1000, 600, "zhui guang zhe", nullptr, nullptr);

		m_RenderContext = std::make_unique<RendererContext>();
		m_RenderContext->Initialize();

		m_SwapChain.Initialize(m_RenderContext->GetVKInstance(), m_RenderContext->GetLogicalDevice());
		m_SwapChain.InitializeWindowSurface(m_WindowHandle);
	}

	Window::~Window()
	{
		glfwDestroyWindow(m_WindowHandle);
		glfwTerminate();
	}

	void Window::PollEvents()
	{
		glfwPollEvents();
	}

	int Window::ShouldWindowClose()
	{
		return glfwWindowShouldClose(m_WindowHandle);
	}

	GLFWwindow* Window::GetWindowHandle()
	{
		return m_WindowHandle;
	}
}