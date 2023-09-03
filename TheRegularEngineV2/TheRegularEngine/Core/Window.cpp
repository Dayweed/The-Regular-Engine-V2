#include "pch.h"
#include "Window.h"
#include "Core/Logger.h"

namespace TRE
{
	bool Window::FrameBufferResized = false;

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

		m_SwapChain = std::make_shared<SwapChain>();
		if (glfwCreateWindowSurface(RendererContext::GetVKInstance(), m_WindowHandle, nullptr, &m_WindowSurface) != VK_SUCCESS)
		{
			assert(false);
		}

		m_PhysicalDevice = std::make_shared<PhysicalDevice>(m_WindowSurface);
		m_LogicalDevice = std::make_shared<Device>(m_PhysicalDevice);
		m_SwapChain->Initialize(m_LogicalDevice->GetLogicalDevice(), m_WindowHandle, m_LogicalDevice->GetGraphicsQ(), m_PhysicalDevice, m_WindowSurface);

		glfwSetWindowUserPointer(m_WindowHandle, &m_Config);
		glfwSetFramebufferSizeCallback(m_WindowHandle, [](GLFWwindow* window, int width, int height)
		{
			auto& data = *((WindowConfig*)glfwGetWindowUserPointer(window));
			if (data.width != width || data.height != height)
				data.resize = true;
			data.width = width;
			data.height = height;

			FrameBufferResized = true;
		});
	}

	Window::~Window()
	{
		//m_SwapChain.DestroySwapChain();
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

	GLFWwindow* Window::GetWindowHandle() const
	{
		return m_WindowHandle;
	}

	WindowConfig& Window::GetWindowConfig()
	{
		return m_Config;
	}

	std::shared_ptr<RendererContext> Window::GetRenderContext()
	{
		return m_RenderContext;
	}

	std::shared_ptr<SwapChain> Window::GetSwapChain()
	{
		return m_SwapChain;
	}
}