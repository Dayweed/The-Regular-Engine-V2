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

		CreateLogicalDevice();
		m_SwapChain->Initialize(m_LogicalDevice, m_WindowHandle, m_GraphicsQueue, m_PhysicalDevice, m_WindowSurface);

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

	void Window::CreateLogicalDevice()
	{
		QueueFamilies queuefamily = m_PhysicalDevice->FindQueueFamilies(m_PhysicalDevice->GetPhysicalDevice());

		std::vector<VkDeviceQueueCreateInfo> AllQueueInfos;
		std::set<int32_t> UniqueQueueFamilies = { queuefamily.Graphics, queuefamily.Present };

		float QueuePiority = 1.f;

		for (uint32_t QueueFamily : UniqueQueueFamilies)
		{
			VkDeviceQueueCreateInfo Queueinfo{};
			Queueinfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			Queueinfo.queueFamilyIndex = QueueFamily;
			Queueinfo.queueCount = 1;
			Queueinfo.pQueuePriorities = &QueuePiority;
			AllQueueInfos.push_back(Queueinfo);
		}

		VkPhysicalDeviceFeatures physicalfeatures{}; //Later
		physicalfeatures.samplerAnisotropy = VK_TRUE;

		VkDeviceCreateInfo deviceinfo{};
		deviceinfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceinfo.pQueueCreateInfos = AllQueueInfos.data();
		deviceinfo.queueCreateInfoCount = static_cast<uint32_t>(AllQueueInfos.size());
		deviceinfo.pEnabledFeatures = &physicalfeatures;
		deviceinfo.enabledExtensionCount = static_cast<uint32_t>(m_DeviceExtensions.size());
		deviceinfo.ppEnabledExtensionNames = m_DeviceExtensions.data();

		if (EnableValidationLayer)
		{
			deviceinfo.enabledLayerCount = static_cast<uint32_t>(m_ValidationLayers.size());
			deviceinfo.ppEnabledLayerNames = m_ValidationLayers.data();
		}
		else
		{
			deviceinfo.enabledLayerCount = 0;
		}

		if (vkCreateDevice(m_PhysicalDevice->GetPhysicalDevice(), &deviceinfo, nullptr, &m_LogicalDevice) != VK_SUCCESS)
		{
			assert(false);
		}

		vkGetDeviceQueue(m_LogicalDevice, queuefamily.Graphics, 0, &m_GraphicsQueue);
		vkGetDeviceQueue(m_LogicalDevice, queuefamily.Present, 0, &m_PresentQueue);
	}
}