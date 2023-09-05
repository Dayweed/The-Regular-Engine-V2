#include "pch.h"
#include "Window.h"
#include "Core/Logger.h"
#include "InputHandler/InputHandler.h"

namespace TRE
{
	GLFWwindow* Window::GetWindowHandle() const
	{
		return m_WindowHandle;
	}

	WindowConfig& Window::GetWindowConfig()
	{
		return m_Config;
	}

	std::shared_ptr<RendererContext>& Window::GetRenderContext()
	{
		return m_RenderContext;
	}

	std::shared_ptr<SwapChain>& Window::GetSwapChain()
	{
		return m_SwapChain;
	}

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
		
		m_SwapChain = std::make_shared<SwapChain>(m_RenderContext->GetDeviceInternally(), m_RenderContext->GetPhysicalDeviceInternally(), m_WindowHandle);
		m_SwapChain->Initialize(m_Config.width, m_Config.height);

		glfwSetWindowUserPointer(m_WindowHandle, &m_Config);
		glfwSetKeyCallback(m_WindowHandle, InputHandler::key_cb);
		glfwSetMouseButtonCallback(m_WindowHandle, InputHandler::mousebutton_cb);
		glfwSetCursorPosCallback(m_WindowHandle, InputHandler::mousepos_cb);
		glfwSetScrollCallback(m_WindowHandle, InputHandler::mousescroll_cb);
		glfwSetCursorEnterCallback(m_WindowHandle, InputHandler::mousefocus_cb);
		glfwSetFramebufferSizeCallback(m_WindowHandle, [](GLFWwindow* window, int width, int height)
		{
			auto& Config = *(WindowConfig*)glfwGetWindowUserPointer(window);

			Config.width = width;
			Config.height = height;
			Config.resize = true;
		});
	}

	Window::~Window()
	{
		m_SwapChain->DestroySwapChain();
		glfwTerminate();
	}

	void Window::BeginFrame()
	{
		m_SwapChain->BeginFrame();
	}

	void Window::SwapBuffers()
	{
		m_SwapChain->Present();
	}

	void Window::PollEvents()
	{
		glfwPollEvents();
	}

	int Window::ShouldWindowClose()
	{
		return glfwWindowShouldClose(m_WindowHandle);
	}

	float Window::GetDeltaTime() const
	{
		static double lastTime = glfwGetTime();
		double currentTime = glfwGetTime();
		float deltaTime = float(currentTime - lastTime);
		lastTime = currentTime;
		return deltaTime;
	}
}