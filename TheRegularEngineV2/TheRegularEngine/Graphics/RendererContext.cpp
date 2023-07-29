#include "pch.h"
#include "RendererContext.h"
#include "GLFW/glfw3.h"

namespace TRE
{
	#define VK_KHR_WIN32_EXTENSION_NAME "VK_KHR_win32_surface"
	PFN_vkSetDebugUtilsObjectNameEXT FP_DebugUtilsObjectNameEXT;
	PFN_vkCmdBeginDebugUtilsLabelEXT FP_CmdBeginDebugUtilsLabelEXT;
	PFN_vkCmdEndDebugUtilsLabelEXT FP_CmdEndDebugUtilsUtilsEXT;
	PFN_vkCmdInsertDebugUtilsLabelEXT FP_CmdInsertDebugUtilsLabelEXT;
	VkInstance RendererContext::m_instance = nullptr;

	static VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugUtilsMessengerCallback(const VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, 
																					  const VkDebugUtilsMessageTypeFlagsEXT messageType, 
																					  const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, 
																					  void* pUserData)
	{
		(void)pUserData;
		return VK_FALSE;
	}

	void RendererContext::Initialize()
	{
		std::cout << "Initializing Renderer" << std::endl;

		if (int Supported = glfwVulkanSupported(); !Supported)
		{
			std::cout << "GLFW doesn't support vulkan" << std::endl;
			assert(Supported); //Change to proper assert
		}

		if (bool Supported = CheckAPIVersion(VK_API_VERSION_1_3); !Supported)
		{
			assert(Supported); //Change to proper assert
		}

		VkApplicationInfo Appinfo{};
		Appinfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		Appinfo.apiVersion = VK_API_VERSION_1_3;
		Appinfo.pEngineName = "The Reglar Engine";
		Appinfo.pApplicationName = "The Reglar Engine";
		Appinfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		Appinfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		
		std::vector<const char*> Extentions = { VK_KHR_SURFACE_EXTENSION_NAME,
												VK_KHR_WIN32_EXTENSION_NAME, //Can be removed if not needed later
												VK_EXT_DEBUG_UTILS_EXTENSION_NAME }; //Can be used in release if needed with minimal performance hit

		if (EnableValidationLayer)
		{
			Extentions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
			Extentions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
		}

		VkInstanceCreateInfo instanceinfo{};
		instanceinfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		instanceinfo.enabledExtensionCount = Extentions.size();
		instanceinfo.ppEnabledExtensionNames = Extentions.data();
		instanceinfo.pApplicationInfo = &Appinfo;

		if (EnableValidationLayer)
		{
			const char* ValidationLayer = "VK_LAYER_KHRONOS_validation"; //Help to debug
			uint32_t Layercount;
			vkEnumerateInstanceLayerProperties(&Layercount, nullptr);
			std::vector<VkLayerProperties> LayerProp(Layercount);
			vkEnumerateInstanceLayerProperties(&Layercount, LayerProp.data());
			
			std::cout << "Vulkan instance layers:" << std::endl;
			bool ContainLayer = false;
			for (const VkLayerProperties& layer : LayerProp)
			{
				std::cout << layer.layerName << std::endl;
				if (strcmp(layer.layerName, ValidationLayer) == 0)
				{
					ContainLayer = true;
					break;
				}
			}

			if (ContainLayer)
			{
				instanceinfo.enabledLayerCount = 1;
				instanceinfo.ppEnabledLayerNames = &ValidationLayer;
			}
			else
			{
				std::cout << " Validation Layer VK_LAYER_KHRONOS_validation not here, validation automatically disabled" << std::endl; //Replace with logging
			}
		}

		if (VkResult Result = vkCreateInstance(&instanceinfo, nullptr, &m_instance); Result != VK_SUCCESS)
		{
			std::cout << "Failed to create vulkan instance " << std::endl;
			//Add assert
		}

		LoadDebugExtensions(m_instance);

		if (EnableValidationLayer)
		{
			auto CreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_instance, "vkCreateDebugUtilsMessengerEXT");
			assert(CreateDebugUtilsMessengerEXT);
			//Add Assert 

			VkDebugUtilsMessengerCreateInfoEXT DebugMessengerCreateInfo{};
			DebugMessengerCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
			DebugMessengerCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
			DebugMessengerCreateInfo.pfnUserCallback = VulkanDebugUtilsMessengerCallback;
			DebugMessengerCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

			if (VkResult Result = CreateDebugUtilsMessengerEXT(m_instance, &DebugMessengerCreateInfo, nullptr, &m_DebugUtilsMessenger); Result != VK_SUCCESS)
			{
				std::cout << "Failed to create debug utils messenger" << std::endl;
				//add assert
			}
		}

		m_PhysicalDevice = std::make_shared<PhysicalDevice>();

		VkPhysicalDeviceFeatures Features{};
		Features.samplerAnisotropy = true;
		Features.wideLines = true;
		Features.fillModeNonSolid = true;
		Features.independentBlend = true;
		Features.pipelineStatisticsQuery = true;

		m_LogicalDevice = std::make_shared<LogicalDevice>(m_PhysicalDevice, Features);

		VkPipelineCacheCreateInfo PipelineCacheCreateInfo{};
		PipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
		if (VkResult Result = vkCreatePipelineCache(m_LogicalDevice->GetLogicalDevice(), &PipelineCacheCreateInfo, nullptr, &m_PipelineCache); Result != VK_SUCCESS)
		{
			std::cout << "Failed to create PipelineCache" << std::endl;
			//Add assert
		}
	}

	bool RendererContext::CheckAPIVersion(uint32_t supportedversion)
	{
		uint32_t CurrentVersion;
		vkEnumerateInstanceVersion(&CurrentVersion); //Get current version

		if (CurrentVersion < supportedversion)
		{
			std::cout << "Vulkan driver not supported" << std::endl;
			std::cout << " You have: " << VK_API_VERSION_MAJOR(CurrentVersion) << "." << VK_API_VERSION_MINOR(CurrentVersion) << "." << VK_API_VERSION_PATCH(CurrentVersion);
			std::cout << " You need: " << VK_API_VERSION_MAJOR(supportedversion) << "." << VK_API_VERSION_MINOR(supportedversion) << "." << VK_API_VERSION_PATCH(supportedversion);
			return false;
		}

		return true;
	}

	void RendererContext::LoadDebugExtensions(VkInstance instance)
	{
		FP_DebugUtilsObjectNameEXT = (PFN_vkSetDebugUtilsObjectNameEXT)(vkGetInstanceProcAddr(instance, "vkSetDebugUtilsObjectNameEXT"));
		if (FP_DebugUtilsObjectNameEXT == nullptr)
			FP_DebugUtilsObjectNameEXT = [](VkDevice device, const VkDebugUtilsObjectNameInfoEXT* pNameInfo) { return VK_SUCCESS; };

		FP_CmdBeginDebugUtilsLabelEXT = (PFN_vkCmdBeginDebugUtilsLabelEXT)(vkGetInstanceProcAddr(instance, "vkCmdBeginDebugUtilsLabelEXT"));
		if (FP_CmdBeginDebugUtilsLabelEXT == nullptr)
			FP_CmdBeginDebugUtilsLabelEXT = [](VkCommandBuffer commandBuffer, const VkDebugUtilsLabelEXT* pLabelInfo) {};

		FP_CmdEndDebugUtilsUtilsEXT = (PFN_vkCmdEndDebugUtilsLabelEXT)(vkGetInstanceProcAddr(instance, "vkCmdEndDebugUtilsLabelEXT"));
		if (FP_CmdEndDebugUtilsUtilsEXT == nullptr)
			FP_CmdEndDebugUtilsUtilsEXT = [](VkCommandBuffer commandBuffer) {};

		FP_CmdInsertDebugUtilsLabelEXT = (PFN_vkCmdInsertDebugUtilsLabelEXT)(vkGetInstanceProcAddr(instance, "vkCmdInsertDebugUtilsLabelEXT"));
		if (FP_CmdInsertDebugUtilsLabelEXT == nullptr)
			FP_CmdInsertDebugUtilsLabelEXT = [](VkCommandBuffer commandBuffer, const VkDebugUtilsLabelEXT* pLabelInfo) {};
	}

	VkInstance RendererContext::GetVKInstance()
	{
		return m_instance;
	}

	std::shared_ptr<PhysicalDevice> RendererContext::GetPhysicalDevice()
	{
		return m_PhysicalDevice;
	}

	std::shared_ptr<LogicalDevice> RendererContext::GetLogicalDevice()
	{
		return m_LogicalDevice;
	}
}