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
			const std::vector<const char*> m_ValidationLayers = { "VK_LAYER_KHRONOS_validation" };
		public:
			void PhysicalDeviceSetup();
			uint32_t GetPhysicalDeviceCount();
			bool IsPhysicalDeviceSuitable(VkPhysicalDevice pd);
			void CreateLogicalDevice();
			QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice dev);
			void CreateWindowSurface(GLFWwindow* handle);
			const std::vector<const char*> m_DeviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
			VkPhysicalDevice m_PhysicalDevice;
			VkDevice m_Device;
			VkQueue m_GraphicsQ;
			VkQueue m_ComputeQ;
			VkSurfaceKHR m_Surface;
			bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
			SwapChainDetails QuerySwapChainSupprt(VkPhysicalDevice device);
			VkSurfaceKHR GetSurface() { return m_Surface; }

		private:
			bool CheckAPIVersion(uint32_t supportedversion);

		private:
			static VkInstance m_instance;
			std::shared_ptr<PhysicalDevice> m_PhysicalDevices;
			std::shared_ptr<Device> m_Devices;
			VkDebugUtilsMessengerEXT m_DebugUtilsMessenger;
	};
}