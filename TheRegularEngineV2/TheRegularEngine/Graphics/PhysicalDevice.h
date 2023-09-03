#pragma once
#include "pch.h"
#include "GLFW/glfw3.h"

namespace TRE
{
	struct QueueFamilyIndices
	{
		int32_t Graphics = -1;
		int32_t Compute = -1;
		//int32_t Transfer = -1;
	};

	struct QueueFamilies
	{
		int32_t Graphics = -1;
		int32_t Present = -1;

		bool IsComplete() { return ((Graphics != -1) && (Present != -1)); }
	};

	struct SwapChainDetails
	{
		VkSurfaceCapabilitiesKHR Capabilities{};
		std::vector<VkSurfaceFormatKHR> Formats;
		std::vector<VkPresentModeKHR> PresentModes;
	};

	class PhysicalDevice
	{
		public:
			PhysicalDevice(VkSurfaceKHR Surface);
			~PhysicalDevice();

			QueueFamilyIndices GetQueueFamilies();
			QueueFamilyIndices GetQueueFamilies(int flags);
			VkFormat GetDepthFormat();
			VkPhysicalDevice GetPhysicalDevice() const;
			VkPhysicalDeviceProperties GetPhysicalDeviceProperties();
			VkPhysicalDeviceMemoryProperties GetPhysicalDeviceMemoryProperties();
			QueueFamilies FindQueueFamilies(VkPhysicalDevice dev);

		private:
			bool IsExtensionSupported(const std::string& Extension);
			uint32_t GetPhysicalDeviceCount();
			bool IsPhysicalDeviceSuitable(VkPhysicalDevice pd);
			SwapChainDetails QuerySwapChainSupprt(VkPhysicalDevice device);
			bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
			const std::vector<const char*> m_DeviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

		private:
			VkSurfaceKHR m_Surface;
			VkPhysicalDevice m_PhysicalDevice;
			VkPhysicalDeviceProperties m_Properties;
			VkPhysicalDeviceFeatures m_Features;
			VkPhysicalDeviceMemoryProperties m_MemoryProperties;

			QueueFamilyIndices m_QueueFamilies;
			std::vector <VkQueueFamilyProperties> m_QueueFamilyProperties;
			std::vector<VkDeviceQueueCreateInfo> m_QueueCreateInfos;
			
			std::unordered_set<std::string> m_SupportedExtensions;
			VkFormat m_DepthFormat;
	};
}