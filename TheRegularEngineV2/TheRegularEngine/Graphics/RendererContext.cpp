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
		return m_Device;
	}

	std::shared_ptr<PhysicalDevice> RendererContext::GetPhysicalDeviceInternally()
	{
		return m_PhysicalDevice;
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
		m_Device->Destroy();
		vkDestroyInstance(m_instance, nullptr);
		m_instance = nullptr;
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

		VkInstanceCreateInfo InstanceCreateInfo{};
		InstanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		InstanceCreateInfo.pApplicationInfo = &Appinfo;
		InstanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(Extentions.size());
		InstanceCreateInfo.ppEnabledExtensionNames = Extentions.data();
		
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
			TRE_CORE_INFO("Vulkan driver not supported");
			TRE_CORE_INFO("You have: {0}.{1}.{2}", VK_API_VERSION_MAJOR(CurrentVersion), VK_API_VERSION_MINOR(CurrentVersion), VK_API_VERSION_PATCH(CurrentVersion));
			TRE_CORE_INFO("You need: {0}.{1}.{2}", VK_API_VERSION_MAJOR(supportedversion), VK_API_VERSION_MINOR(supportedversion), VK_API_VERSION_PATCH(supportedversion));
			return false;
		}

		return true;
	}
}