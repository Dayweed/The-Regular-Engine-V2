#pragma once
#include "vulkan/vulkan.h"
#include "PhysicalDevice.h"
#include "Device.h"
#include "GLFW/glfw3.h"

namespace TRE
{
	#ifdef _DEBUG
		static const bool EnableValidationLayer = true;
	#else
		static const bool EnableValidationLayer = false;
	#endif

	struct SwapChainDetails
	{
		VkSurfaceCapabilitiesKHR Capabilities{};
		std::vector<VkSurfaceFormatKHR> Formats;
		std::vector<VkPresentModeKHR> PresentModes;
	};

	class RendererContext
	{
		public:
			RendererContext();
			~RendererContext();

			void Initialize();

			std::shared_ptr<PhysicalDevice> GetPhysicalDeviceInternally();
			std::shared_ptr<Device> GetDeviceInternally();

			static VkInstance GetVKInstance();
			static std::shared_ptr<RendererContext> Get();
			static std::shared_ptr<Device> GetDevice();
			static std::shared_ptr<PhysicalDevice> GetPhysicalDevice();

		public:
			bool CheckValidationLayerSupported();
			void CreateVulkanInstance(GLFWwindow* Handle);
			std::vector<const char*> GetRequiredExtensions() const;
			void PopulateDebugMessengerInfo(VkDebugUtilsMessengerCreateInfoEXT& DebugInfo);
			void SetupDebugMessage();
			VkResult CreateDebugMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* info, const VkAllocationCallbacks* allocator, VkDebugUtilsMessengerEXT* debugmsger);
			

		private:
			bool CheckAPIVersion(uint32_t supportedversion);

		private:
			static VkInstance m_instance;


			std::shared_ptr<PhysicalDevice> m_PhysicalDevices;
			std::shared_ptr<Device> m_Devices;
			VkDebugUtilsMessengerEXT m_DebugUtilsMessenger;
	};
}