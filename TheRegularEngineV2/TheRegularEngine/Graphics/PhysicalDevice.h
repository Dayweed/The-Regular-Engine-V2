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

	class PhysicalDevice
	{
		public:
			PhysicalDevice();
			~PhysicalDevice();

			QueueFamilyIndices GetQueueFamilies();
			QueueFamilyIndices GetQueueFamilies(int flags);
			VkFormat GetDepthFormat();
			VkPhysicalDevice GetPhysicalDevice() const;
			VkPhysicalDeviceProperties GetPhysicalDeviceProperties();
			VkPhysicalDeviceMemoryProperties GetPhysicalDeviceMemoryProperties();

		private:
			bool IsExtensionSupported(const std::string& Extension);

		private:
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