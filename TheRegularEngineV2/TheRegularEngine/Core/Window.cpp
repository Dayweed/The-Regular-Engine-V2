#include "pch.h"
#include <chrono>
#include "Window.h"
#include "Core/Logger.h"
#include "InputHandler/InputHandler.h"
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

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

		if (m_Config.FullScreen)
		{
			GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
			const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);

			glfwWindowHint(GLFW_DECORATED, false);
			glfwWindowHint(GLFW_RED_BITS, mode->redBits);
			glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
			glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
			glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
			glfwWindowHint(GLFW_AUTO_ICONIFY, false);
			m_WindowHandle = glfwCreateWindow(mode->width, mode->height, m_Config.Title.c_str(), primaryMonitor, nullptr);

			glfwFocusWindow(m_WindowHandle);
		}
		else
			m_WindowHandle = glfwCreateWindow(m_Config.width, m_Config.height, m_Config.Title.c_str(), nullptr, nullptr);

		m_RenderContext = std::make_shared<RendererContext>();
		m_RenderContext->Initialize();
		
		m_SwapChain = std::make_shared<SwapChain>(m_RenderContext->GetDeviceInternally(), m_RenderContext->GetPhysicalDeviceInternally(), m_WindowHandle);
		m_SwapChain->Initialize(m_Config.width, m_Config.height);

		//glfwSetInputMode(m_WindowHandle, GLFW_STICKY_MOUSE_BUTTONS, GLFW_TRUE);
		glfwSetWindowUserPointer(m_WindowHandle, &m_Config);
		glfwSetKeyCallback(m_WindowHandle, InputHandler::KeyCb);
		glfwSetMouseButtonCallback(m_WindowHandle, InputHandler::MouseButtonCb);
		glfwSetCursorPosCallback(m_WindowHandle, InputHandler::MousePosCb);
		glfwSetScrollCallback(m_WindowHandle, InputHandler::MouseScrollCb);
		glfwSetCursorEnterCallback(m_WindowHandle, InputHandler::MouseFocusCb);
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
		/*CheckMouseEvent(m_WindowHandle, GLFW_MOUSE_BUTTON_1, GLFW_PRESS);
		CheckMouseEvent(m_WindowHandle, GLFW_MOUSE_BUTTON_2, GLFW_PRESS);
		CheckMouseEvent(m_WindowHandle, GLFW_MOUSE_BUTTON_3, GLFW_PRESS);*/
		for (int i{}; i < 5; ++i)
		{
			InputHandler::CheckMouseEvent(m_WindowHandle, i, GLFW_PRESS);
		}
	}

	int Window::ShouldWindowClose()
	{
		return glfwWindowShouldClose(m_WindowHandle);
	}

	void Window::UpdateDeltaTime()
	{
		static auto lastTime = std::chrono::high_resolution_clock::now();
		auto currentTime = std::chrono::high_resolution_clock::now();
		m_DeltaTime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - lastTime).count();
		lastTime = currentTime;
	}

	float Window::GetDeltaTime() const
	{
		return m_DeltaTime;
	}

	void Window::MaximizeWindow()
	{
		glfwMaximizeWindow(m_WindowHandle);
	}
}