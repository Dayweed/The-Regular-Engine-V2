#include "pch.h"
#include "TREIncludes.h"
#include "Core/Logger.h"
#include "GLFW/glfw3.h"

namespace TRE
{
	#define VK_KHR_WIN32_EXTENSION_NAME "VK_KHR_win32_surface"
	VkInstance RendererContext::s_Instance = VK_NULL_HANDLE;
	uint32_t RendererContext::s_FramesInFlight = 3;

	VkInstance RendererContext::GetVKInstance()
	{
		return s_Instance;
	}

	uint32_t RendererContext::GetFramesInFlight()
	{
		return s_FramesInFlight;
	}

	std::shared_ptr<RendererContext> RendererContext::Get()
	{
		return Engine::GetInstance().GetWindow()->GetRenderContext();
	}

	std::shared_ptr<Device> RendererContext::GetDevice()
	{
		return Get()->GetDeviceInternally();
	}

	std::shared_ptr<PhysicalDevice> RendererContext::GetPhysicalDevice()
	{
		return Get()->GetPhysicalDeviceInternally();
	}

	std::shared_ptr<Device>& RendererContext::GetDeviceInternally()
	{
		return m_Device;
	}

	std::shared_ptr<PhysicalDevice>& RendererContext::GetPhysicalDeviceInternally()
	{
		return m_PhysicalDevice;
	}

	VkSurfaceKHR RendererContext::GetSurface()
	{
		return m_Surface;
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
			TRE_CORE_WARN(Message);
		}
		else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
		{
			Message += "[Vulkan Debug Error] ";
			Message += pCallbackData->pMessage;
			TRE_CORE_WARN(Message);
		}

		return VK_FALSE;
	}

	RendererContext::RendererContext()
	{

	}

	RendererContext::~RendererContext()
	{
		if (EnableValidationLayer)
		{
			auto vkDestroyDebugUtilsMessengerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(s_Instance, "vkDestroyDebugUtilsMessengerEXT");
			vkDestroyDebugUtilsMessengerEXT(s_Instance, m_DebugUtilsMessenger, nullptr);
		}
		m_Device->Destroy();
		vkDestroyInstance(s_Instance, nullptr);
		s_Instance = nullptr;
	}

	void RendererContext::Initialize()
	{
		TRE_CORE_INFO("Initializing Renderer Context");
		if (int Supported = glfwVulkanSupported(); !Supported)
		{
			TRE_CORE_ERROR("GLFW doesn't support vulkan");
			assert(Supported); //Change to proper assert
		}

		if (bool Supported = CheckAPIVersion(VK_API_VERSION_1_3); !Supported)
		{
			TRE_CORE_ERROR("Vulkan API version not supported");
			assert(Supported); //Change to proper assert
		}

		VkApplicationInfo Appinfo{};
		Appinfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		Appinfo.apiVersion = VK_API_VERSION_1_3;
		Appinfo.pEngineName = "The Regular Engine";
		Appinfo.pApplicationName = "The Regular Engine";

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

		if (VkResult Result = vkCreateInstance(&InstanceCreateInfo, nullptr, &s_Instance); Result != VK_SUCCESS)
		{
			TRE_CORE_INFO("Failed to create vulkan instance ");
			assert(Result == VK_SUCCESS);
		}

		if (EnableValidationLayer)
		{
			auto CreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(s_Instance, "vkCreateDebugUtilsMessengerEXT");
			assert(CreateDebugUtilsMessengerEXT);

			VkDebugUtilsMessengerCreateInfoEXT DebugCreateInfo{};
			DebugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
			DebugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
			DebugCreateInfo.pfnUserCallback = VulkanDebugUtilsMessengerCallback;
			DebugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

			if (VkResult Result = CreateDebugUtilsMessengerEXT(s_Instance, &DebugCreateInfo, nullptr, &m_DebugUtilsMessenger); Result != VK_SUCCESS)
			{
				TRE_CORE_INFO("Failed to create debug utils messenger");
				assert(Result == VK_SUCCESS);
			}
		}

		m_PhysicalDevice = std::make_shared<PhysicalDevice>();

		VkPhysicalDeviceFeatures PhysicalDeviceFeatures{};
		PhysicalDeviceFeatures.samplerAnisotropy = true;
		PhysicalDeviceFeatures.wideLines = true;
		PhysicalDeviceFeatures.fillModeNonSolid = true;
		PhysicalDeviceFeatures.independentBlend = true;
		PhysicalDeviceFeatures.pipelineStatisticsQuery = true;

		m_Device = std::make_shared<Device>(m_PhysicalDevice, PhysicalDeviceFeatures);
	}

	bool RendererContext::CheckAPIVersion(uint32_t supportedversion)
	{
		uint32_t CurrentVersion;
		vkEnumerateInstanceVersion(&CurrentVersion); //Get current version

		if (CurrentVersion < supportedversion)
		{
			TRE_CORE_INFO("Vulkan driver not supported, Go update");
			TRE_CORE_INFO("You have: {0}.{1}.{2}", VK_API_VERSION_MAJOR(CurrentVersion), VK_API_VERSION_MINOR(CurrentVersion), VK_API_VERSION_PATCH(CurrentVersion));
			TRE_CORE_INFO("You need: {0}.{1}.{2}", VK_API_VERSION_MAJOR(supportedversion), VK_API_VERSION_MINOR(supportedversion), VK_API_VERSION_PATCH(supportedversion));
			return false;
		}

		return true;
	}
}