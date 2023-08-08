#include "pch.h"
#include "Window.h"

namespace TRE
{
	Window::Window(const WindowConfig& config) : m_Config(config)
	{
		if (int Error = glfwInit(); !Error)
		{
			assert(Error == GLFW_TRUE);
		}

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); //Default is opengl so we set to no since we going V

		m_WindowHandle = glfwCreateWindow(m_Config.width, m_Config.height, m_Config.Title.c_str(), nullptr, nullptr);

		m_RenderContext = std::make_shared<RendererContext>();
		m_RenderContext->Initialize();

		m_SwapChain.Initialize(m_RenderContext->GetVKInstance(), m_RenderContext->GetDeviceInternally(), m_WindowHandle);
		m_SwapChain.CreateSwapChain(&m_Config.width, &m_Config.height, m_Config.Vsync);
	}

	Window::~Window()
	{
		m_SwapChain.DestroySwapChain();
		glfwTerminate();
	}

	void Window::SwapBuffers()
	{
		m_SwapChain.Present();
	}

	void Window::PollEvents()
	{
		glfwPollEvents();
	}

	int Window::ShouldWindowClose()
	{
		return glfwWindowShouldClose(m_WindowHandle);
	}

	GLFWwindow* Window::GetWindowHandle() const
	{
		return m_WindowHandle;
	}

	const WindowConfig& Window::GetWindowConfig() const
	{
		return m_Config;
	}

	std::shared_ptr<RendererContext> Window::GetRenderContext()
	{
		return m_RenderContext;
	}

	SwapChain Window::GetSwapChain()
	{
		return m_SwapChain;
	}
}