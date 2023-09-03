#include "pch.h"
#include "TREIncludes.h"
#include "GLFW/glfw3.h"
#include "Core/Logger.h"

namespace TRE
{
	#define VK_KHR_WIN32_EXTENSION_NAME "VK_KHR_win32_surface"
	VkInstance RendererContext::m_instance = nullptr;

	VkInstance RendererContext::GetVKInstance()
	{
		return m_instance;
	}

	std::shared_ptr<RendererContext> RendererContext::Get()
	{
		return Engine::GetInstance().GetWindow()->GetRenderContext();
	}

	std::shared_ptr<Device> RendererContext::GetDevice()
	{
		return Get()->GetDeviceInternally();
	}

	std::shared_ptr<Device> RendererContext::GetDeviceInternally()
	{
		return m_Devices;
	}

	std::shared_ptr<PhysicalDevice> RendererContext::GetPhysicalDeviceInternally()
	{
		return m_PhysicalDevices;
	}

	std::shared_ptr<PhysicalDevice> RendererContext::GetPhysicalDevice()
	{
		return Get()->GetPhysicalDeviceInternally();
	}

	static VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugUtilsMessengerCallback(const VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
																			const VkDebugUtilsMessageTypeFlagsEXT messageType, 
																			const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
	{
		(void)pUserData;
		(void)messageType;
		
		std::string Message;
		if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
		{
			Message += "[Vulkan Debug Warning] ";
			Message += pCallbackData->pMessage;
			TRE_CORE_INFO(Message);
		}
		else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
		{
			Message += "[Vulkan Debug Error] ";
			Message += pCallbackData->pMessage;
			TRE_CORE_INFO(Message);
		}

		return VK_FALSE;
	}

	RendererContext::RendererContext()
	{

	}

	RendererContext::~RendererContext()
	{
		auto vkDestroyDebugUtilsMessengerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_instance, "vkDestroyDebugUtilsMessengerEXT");
		vkDestroyDebugUtilsMessengerEXT(m_instance, m_DebugUtilsMessenger, nullptr);
		//m_Device->Destroy();
		//vkDestroyInstance(m_instance, nullptr);
		//m_instance = nullptr;
	}

	void RendererContext::Initialize()
	{
		TRE_CORE_INFO("Initializing Renderer Context");
		if (int Supported = glfwVulkanSupported(); !Supported)
		{
			TRE_CORE_INFO("GLFW doesn't support vulkan");
			assert(Supported); //Change to proper assert
		}

		if (bool Supported = CheckAPIVersion(VK_API_VERSION_1_3); !Supported)
		{
			TRE_CORE_INFO("Vulkan API version not supported");
			assert(Supported); //Change to proper assert
		}

		VkApplicationInfo Appinfo{};
		Appinfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		Appinfo.apiVersion = VK_API_VERSION_1_3;
		Appinfo.pEngineName = "The Reglar Engine";
		Appinfo.pApplicationName = "The Reglar Engine";

		std::vector<const char*> Extentions =
		{ 
			VK_KHR_SURFACE_EXTENSION_NAME,
			VK_KHR_WIN32_EXTENSION_NAME,	  //Can be removed if not needed later
			VK_EXT_DEBUG_UTILS_EXTENSION_NAME //Can be used in release if needed with minimal performance hit
		};

		if (EnableValidationLayer)
		{
			Extentions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
			Extentions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
			Extentions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
		}

		uint32_t glfwExtensionsCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionsCount); //Vulkan requires extensions to interface with window, GLFW returns the required extensions.

		std::vector<const char*> AllRequiredExtensions(glfwExtensions, glfwExtensions + glfwExtensionsCount);
		if (EnableValidationLayer)
		{
			AllRequiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
			AllRequiredExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
			AllRequiredExtensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
		}

		VkInstanceCreateInfo InstanceCreateInfo{};
		InstanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		InstanceCreateInfo.pApplicationInfo = &Appinfo;
		InstanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(AllRequiredExtensions.size());
		InstanceCreateInfo.ppEnabledExtensionNames = AllRequiredExtensions.data();
		
		std::vector<const char*> ValidationLayer;

		if (EnableValidationLayer)
		{
			ValidationLayer.push_back("VK_LAYER_KHRONOS_validation");
			ValidationLayer.push_back("VK_LAYER_RENDERDOC_Capture");

			uint32_t Layercount;
			vkEnumerateInstanceLayerProperties(&Layercount, nullptr);
			std::vector<VkLayerProperties> LayerProp(Layercount);
			vkEnumerateInstanceLayerProperties(&Layercount, LayerProp.data());

			TRE_CORE_INFO("Vulkan instance layers:");

			bool ContainLayer = true;
			for (const char* layerName : ValidationLayer)
			{
				ContainLayer = false;

				for (const auto& layerProperties : LayerProp)
				{
					if (strcmp(layerName, layerProperties.layerName) == 0)
					{
						ContainLayer = true;
						break;
					}
				}
			}

			if(ContainLayer)
			{
				TRE_CORE_INFO("{0} Validation Layer(s) found", ValidationLayer.size());
				InstanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(ValidationLayer.size());
				InstanceCreateInfo.ppEnabledLayerNames = ValidationLayer.data();
			}
			else
			{
				InstanceCreateInfo.enabledLayerCount = 0;
				TRE_CORE_INFO(" Validation Layer VK_LAYER_KHRONOS_validation not here, validation automatically disabled");
			}
		}

		if (VkResult Result = vkCreateInstance(&InstanceCreateInfo, nullptr, &m_instance); Result != VK_SUCCESS)
		{
			TRE_CORE_INFO("Failed to create vulkan instance ");
			assert(Result == VK_SUCCESS);
		}

		if (EnableValidationLayer)
		{
			auto CreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_instance, "vkCreateDebugUtilsMessengerEXT");
			assert(CreateDebugUtilsMessengerEXT);

			VkDebugUtilsMessengerCreateInfoEXT DebugMessengerCreateInfo{};
			DebugMessengerCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
			DebugMessengerCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
			DebugMessengerCreateInfo.pfnUserCallback = VulkanDebugUtilsMessengerCallback;
			DebugMessengerCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

			if (VkResult Result = CreateDebugUtilsMessengerEXT(m_instance, &DebugMessengerCreateInfo, nullptr, &m_DebugUtilsMessenger); Result != VK_SUCCESS)
			{
				TRE_CORE_INFO("Failed to create debug utils messenger");
				assert(Result == VK_SUCCESS);
			}
		}

		//m_PhysicalDevice = std::make_shared<PhysicalDevice>();
		//
		//VkPhysicalDeviceFeatures PhysicalDeviceFeatures{};
		//PhysicalDeviceFeatures.samplerAnisotropy = true;
		//PhysicalDeviceFeatures.wideLines = true;
		//PhysicalDeviceFeatures.fillModeNonSolid = true;
		//PhysicalDeviceFeatures.independentBlend = true;
		//PhysicalDeviceFeatures.pipelineStatisticsQuery = true;
		//
		//m_Device = std::make_shared<Device>(m_PhysicalDevice, PhysicalDeviceFeatures);
	}

	bool RendererContext::CheckAPIVersion(uint32_t supportedversion)
	{
		uint32_t CurrentVersion;
		vkEnumerateInstanceVersion(&CurrentVersion); //Get current version

		if (CurrentVersion < supportedversion)
		{
			TRE_CORE_INFO("Vulkan driver not supported");
			TRE_CORE_INFO("You have: {0}.{1}.{2}", VK_API_VERSION_MAJOR(CurrentVersion), VK_API_VERSION_MINOR(CurrentVersion), VK_API_VERSION_PATCH(CurrentVersion));
			TRE_CORE_INFO("You need: {0}.{1}.{2}", VK_API_VERSION_MAJOR(supportedversion), VK_API_VERSION_MINOR(supportedversion), VK_API_VERSION_PATCH(supportedversion));
			return false;
		}

		return true;
	}

	//-----------------------------------------------------------------------------------------------------------------------

	bool RendererContext::CheckValidationLayerSupported()
	{
		uint32_t LayerCount = 0;
		vkEnumerateInstanceLayerProperties(&LayerCount, nullptr); //Returns the available layers

		std::vector<VkLayerProperties> AvailableLayers(LayerCount);
		vkEnumerateInstanceLayerProperties(&LayerCount, AvailableLayers.data());

		for (const char* LayerName : m_ValidationLayers)
		{
			bool LayerFound = false;
			for (const auto& LayerProp : AvailableLayers)
			{
				if (strcmp(LayerName, LayerProp.layerName) == 0)
				{
					LayerFound = true;
					break;
				}
			}

			if (!LayerFound)
			{
				return false;
			}
		}

		return true;
	}

	void RendererContext::CreateVulkanInstance(GLFWwindow* Handle)
	{
		if (EnableValidationLayer && !CheckValidationLayerSupported())
		{
			throw std::runtime_error("Validation layers requested but not available\n");
		}

		auto Extensions = GetRequiredExtensions();

		std::cout << "GLFW Extensions to be supported: \n";
		for (uint32_t i = 0; i < Extensions.size(); i++)
		{
			std::cout << Extensions[i] << "\n";
		}

		VkApplicationInfo VulkanAppInfo{}; //Value initialization pnext to nullptr
		VulkanAppInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		VulkanAppInfo.pApplicationName = "Dulan Renderer";
		VulkanAppInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		VulkanAppInfo.pEngineName = "Engine";
		VulkanAppInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		VulkanAppInfo.apiVersion = VK_API_VERSION_1_0;

		VkInstanceCreateInfo VulkanInstanceInfo{};
		VulkanInstanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		VulkanInstanceInfo.pApplicationInfo = &VulkanAppInfo;
		VulkanInstanceInfo.enabledExtensionCount = static_cast<uint32_t>(Extensions.size());
		VulkanInstanceInfo.ppEnabledExtensionNames = Extensions.data();

		VkDebugUtilsMessengerCreateInfoEXT DebugCreateInfo{};

		if (EnableValidationLayer)
		{
			VulkanInstanceInfo.enabledLayerCount = static_cast<uint32_t>(m_ValidationLayers.size());
			VulkanInstanceInfo.ppEnabledLayerNames = m_ValidationLayers.data();

			PopulateDebugMessengerInfo(DebugCreateInfo);
			VulkanInstanceInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&DebugCreateInfo;
		}
		else
		{
			VulkanInstanceInfo.pNext = nullptr;
			VulkanInstanceInfo.enabledLayerCount = 0;
		}

		uint32_t ExtensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &ExtensionCount, nullptr);

		std::vector<VkExtensionProperties> ExtensionsList(ExtensionCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &ExtensionCount, ExtensionsList.data());

		std::cout << "\nVulkan supported extensions: \n";
		for (const auto& Extension : ExtensionsList)
		{
			std::cout << Extension.extensionName << " | " << Extension.specVersion << "\n";
		}

		VkResult result = vkCreateInstance(&VulkanInstanceInfo, nullptr, &m_instance); //Creates instance
		(result != VK_SUCCESS) ? throw std::runtime_error("\nFailed to create vulkan instance\n") : std::cout << "\nVulkan instance created\n\n";
	}

	std::vector<const char*> RendererContext::GetRequiredExtensions() const
	{
		uint32_t glfwExtensionsCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionsCount); //Vulkan requires extensions to interface with window, GLFW returns the required extensions.

		std::vector<const char*> AllRequiredExtensions(glfwExtensions, glfwExtensions + glfwExtensionsCount);

		if (EnableValidationLayer)
		{
			AllRequiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}

		return AllRequiredExtensions;
	}

	void RendererContext::SetupDebugMessage()
	{
		if (!EnableValidationLayer) return;

		VkDebugUtilsMessengerCreateInfoEXT DebugInfo;
		PopulateDebugMessengerInfo(DebugInfo);


		if (CreateDebugMessengerEXT(m_instance, &DebugInfo, nullptr, &m_DebugUtilsMessenger) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to setup debug messanger");
		}
	}

	VkResult RendererContext::CreateDebugMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* info, const VkAllocationCallbacks* allocator, VkDebugUtilsMessengerEXT* debugmsger)
	{
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
		if (func != nullptr)
		{
			return func(instance, info, allocator, debugmsger);
		}
		else
		{
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}

	void RendererContext::PopulateDebugMessengerInfo(VkDebugUtilsMessengerCreateInfoEXT& DebugInfo)
	{
		DebugInfo = {};
		DebugInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		DebugInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		DebugInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
		DebugInfo.pfnUserCallback = VulkanDebugUtilsMessengerCallback;
	}

	void RendererContext::PhysicalDeviceSetup()
	{
		uint32_t PhysicalDeviceCount = GetPhysicalDeviceCount();
		std::vector<VkPhysicalDevice> PhysicalDevice(PhysicalDeviceCount);
		vkEnumeratePhysicalDevices(m_instance, &PhysicalDeviceCount, PhysicalDevice.data());

		std::cout << "Available GPUs:\n";
		for (const auto& device : PhysicalDevice)
		{
			VkPhysicalDeviceProperties DeviceProp;
			vkGetPhysicalDeviceProperties(device, &DeviceProp);
			std::cout << DeviceProp.deviceName << std::endl;
		}
		std::cout << "-----------------------------------------------------------------------------------------------" << std::endl;

		for (const auto& device : PhysicalDevice)
		{
			if (IsPhysicalDeviceSuitable(device))
			{
				m_PhysicalDevice = device;
				break;
			}
		}

		if (m_PhysicalDevice == VK_NULL_HANDLE)
		{
			throw std::runtime_error("Failed to find suitable GPU!");
		}
	}

	bool RendererContext::IsPhysicalDeviceSuitable(VkPhysicalDevice pd)
	{
		QueueFamilyIndices queues = FindQueueFamilies(pd);
		bool ExtensionSupported = CheckDeviceExtensionSupport(pd);

		bool SwapChainSupported = false;
		if (ExtensionSupported)
		{
			SwapChainDetails Details = QuerySwapChainSupprt(pd);
			SwapChainSupported = !Details.Formats.empty() && !Details.PresentModes.empty();
		}

		VkPhysicalDeviceFeatures supportedFeatures;
		vkGetPhysicalDeviceFeatures(pd, &supportedFeatures);

		return (queues.Graphics != -1) && ExtensionSupported && SwapChainSupported && supportedFeatures.samplerAnisotropy;
	}

	SwapChainDetails RendererContext::QuerySwapChainSupprt(VkPhysicalDevice device)
	{
		SwapChainDetails Details;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_Surface, &Details.Capabilities);

		uint32_t FormatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_Surface, &FormatCount, nullptr);

		if (FormatCount != 0)
		{
			Details.Formats.resize(FormatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_Surface, &FormatCount, Details.Formats.data());
		}

		uint32_t PresentCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_Surface, &PresentCount, nullptr);

		if (PresentCount != 0)
		{
			Details.PresentModes.resize(PresentCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_Surface, &PresentCount, Details.PresentModes.data());
		}

		return Details;
	}


	uint32_t RendererContext::GetPhysicalDeviceCount()
	{
		uint32_t PhysicalDeviceCount = 0;
		vkEnumeratePhysicalDevices(m_instance, &PhysicalDeviceCount, nullptr);
		if (PhysicalDeviceCount == 0)
		{
			throw std::runtime_error("No GPU that supports Vulkan is found!");
		}
		return PhysicalDeviceCount;
	}

	void RendererContext::CreateLogicalDevice()
	{
		QueueFamilyIndices queuefamily = FindQueueFamilies(m_PhysicalDevice);

		std::vector<VkDeviceQueueCreateInfo> AllQueueInfos;
		std::set<int32_t> UniqueQueueFamilies = { queuefamily.Graphics, queuefamily.Compute };

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

		if (vkCreateDevice(m_PhysicalDevice, &deviceinfo, nullptr, &m_Device) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create logical device!");
		}
		else
		{
			TRE_CORE_INFO("Logical Device Created");
		}

		vkGetDeviceQueue(m_Device, queuefamily.Graphics, 0, &m_GraphicsQ);
		vkGetDeviceQueue(m_Device, queuefamily.Compute, 0, &m_ComputeQ);
	}

	QueueFamilyIndices RendererContext::FindQueueFamilies(VkPhysicalDevice dev)
	{
		QueueFamilyIndices MyQueues;

		uint32_t Queuecount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(dev, &Queuecount, nullptr); //Get number of queue by passing nullptr

		std::vector<VkQueueFamilyProperties> QueueFamilies(Queuecount);
		vkGetPhysicalDeviceQueueFamilyProperties(dev, &Queuecount, QueueFamilies.data()); //Get the actual queues properties by passing data

		int i = 0;
		for (const auto& queuefamily : QueueFamilies)
		{
			if (queuefamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				MyQueues.Graphics = i;
			}

			VkBool32 PresentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(dev, i, m_Surface, &PresentSupport);

			if (PresentSupport)
			{
				MyQueues.Compute = i;
			}

			if ((MyQueues.Graphics != -1) && (MyQueues.Compute != -1))
			{
				break;
			}
			i++;
		}
		return MyQueues;
	}

	bool RendererContext::CheckDeviceExtensionSupport(VkPhysicalDevice device)
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

	void RendererContext::CreateWindowSurface(GLFWwindow* handle)
	{
		if (glfwCreateWindowSurface(m_instance, handle, nullptr, &m_Surface) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create window surface!");
		}
	}
}