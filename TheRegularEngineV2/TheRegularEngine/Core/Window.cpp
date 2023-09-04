#include "pch.h"
#include "Window.h"
#include "Core/Logger.h"
#include "InputHandler/InputHandler.h"

namespace TRE
{
	Window::Window(const WindowConfig& config) : m_Config(config)
	{
		if (int Error = glfwInit(); !Error)
		{
			TRE_CORE_CRITICAL("GLFW unable to initialise");
			assert(Error == GLFW_TRUE);
		}

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); //Default is opengl so we set to no default API since we going :eagle:

		m_WindowHandle = glfwCreateWindow(m_Config.width, m_Config.height, m_Config.Title.c_str(), nullptr, nullptr);

		m_RenderContext = std::make_shared<RendererContext>();
		m_RenderContext->Initialize();
		
		m_SwapChain.Initialize(m_RenderContext->GetVKInstance(), m_RenderContext->GetDeviceInternally(), m_WindowHandle);
		m_SwapChain.CreateSwapChain(&m_Config.width, &m_Config.height, m_Config.Vsync);

		glfwSetKeyCallback(GetWindowHandle(), InputHandler::key_cb);
		glfwSetMouseButtonCallback(GetWindowHandle(), InputHandler::mousebutton_cb);
		glfwSetCursorPosCallback(GetWindowHandle(), InputHandler::mousepos_cb);
		glfwSetScrollCallback(GetWindowHandle(), InputHandler::mousescroll_cb);
		glfwSetCursorEnterCallback(GetWindowHandle(), InputHandler::mousefocus_cb);
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