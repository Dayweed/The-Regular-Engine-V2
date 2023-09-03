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
		if (glfwCreateWindowSurface(RendererContext::GetVKInstance(), m_WindowHandle, nullptr, &m_SwapChain->GetSurface()) != VK_SUCCESS)
		{
			assert(false);
		}

		PhysicalDeviceSetup();
		CreateLogicalDevice();
		m_SwapChain->Initialize(m_LogicalDevice, m_WindowHandle, m_GraphicsQueue, m_PhysicalDevice);

		//m_SwapChain.CreateSwapChain(&m_Config.width, &m_Config.height, m_Config.Vsync);

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

	void Window::PhysicalDeviceSetup()
	{
		uint32_t PhysicalDeviceCount = GetPhysicalDeviceCount();
		std::vector<VkPhysicalDevice> PhysicalDevice(PhysicalDeviceCount);
		vkEnumeratePhysicalDevices(RendererContext::GetVKInstance(), &PhysicalDeviceCount, PhysicalDevice.data());

		std::cout << "Available GPUs:\n";
		for (const auto& device : PhysicalDevice)
		{
			VkPhysicalDeviceProperties DeviceProp;
			vkGetPhysicalDeviceProperties(device, &DeviceProp);
			std::cout << DeviceProp.deviceName << std::endl;
		}
		for (const auto& device : PhysicalDevice)
		{
			if (IsPhysicalDeviceSuitable(device))
			{
				m_PhysicalDevice = device;
				break;
			}
		}

		assert(m_PhysicalDevice != VK_NULL_HANDLE);
	}

	bool Window::CheckDeviceExtensionSupport(VkPhysicalDevice device)
	{
		uint32_t ExtensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &ExtensionCount, nullptr); //Get the count

		std::vector<VkExtensionProperties> AvailableExtensions(ExtensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &ExtensionCount, AvailableExtensions.data()); //Get the properties

		std::set<std::string> RequiredExtension(m_DeviceExtensions.begin(), m_DeviceExtensions.end());

		for (const auto& extension : AvailableExtensions)
		{
			RequiredExtension.erase(extension.extensionName);
		}

		return RequiredExtension.empty();
	}

	void Window::CreateLogicalDevice()
	{
		QueueFamilies queuefamily = m_SwapChain->FindQueueFamilies(m_PhysicalDevice);

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

		if (vkCreateDevice(m_PhysicalDevice, &deviceinfo, nullptr, &m_LogicalDevice) != VK_SUCCESS)
		{
			assert(false);
		}

		vkGetDeviceQueue(m_LogicalDevice, queuefamily.Graphics, 0, &m_GraphicsQueue);
		vkGetDeviceQueue(m_LogicalDevice, queuefamily.Present, 0, &m_PresentQueue);
	}

	bool Window::IsPhysicalDeviceSuitable(VkPhysicalDevice pd)
	{
		QueueFamilies queues = m_SwapChain->FindQueueFamilies(pd);
		bool ExtensionSupported = CheckDeviceExtensionSupport(pd);

		bool SwapChainSupported = false;
		if (ExtensionSupported)
		{
			SwapChainDetails Details = m_SwapChain->QuerySwapChainSupprt(pd);
			SwapChainSupported = !Details.Formats.empty() && !Details.PresentModes.empty();
		}

		VkPhysicalDeviceFeatures supportedFeatures;
		vkGetPhysicalDeviceFeatures(pd, &supportedFeatures);

		return queues.IsComplete() && ExtensionSupported && SwapChainSupported && supportedFeatures.samplerAnisotropy;
	}

	uint32_t Window::GetPhysicalDeviceCount()
	{
		uint32_t PhysicalDeviceCount = 0;
		vkEnumeratePhysicalDevices(RendererContext::GetVKInstance(), &PhysicalDeviceCount, nullptr);
		if (PhysicalDeviceCount == 0)
		{
			assert(false);
		}
		return PhysicalDeviceCount;
	}
}