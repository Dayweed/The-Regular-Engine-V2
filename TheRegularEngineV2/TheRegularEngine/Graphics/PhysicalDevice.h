#pragma once
#include "pch.h"

namespace TRE
{
	class PhysicalDevice
	{
		public:
			struct QueueFamilyIndices
			{
				int32_t Graphics = -1;
				int32_t Compute = -1;
				int32_t Transfer = -1;
			};

		public:
			PhysicalDevice();
			~PhysicalDevice();

			bool IsExtensionSupported(const std::string& Extension);
			QueueFamilyIndices GetQueueFamilies();
			QueueFamilyIndices GetQueueFamilies(int flags);
			VkFormat GetDepthFormat();
			VkPhysicalDevice GetPhysicalDevice() const;

		private:
			VkPhysicalDevice m_PhysicalDevice;
			VkPhysicalDeviceProperties m_Properties;
			VkPhysicalDeviceFeatures m_Features;
			VkPhysicalDeviceMemoryProperties m_MemoryProperties;

			VkFormat m_DepthFormat;

			QueueFamilyIndices m_QueueFamilies;
			std::vector <VkQueueFamilyProperties> m_QueueFamilyProperties;
			std::unordered_set<std::string> m_SupportedExtensions;
			std::vector<VkDeviceQueueCreateInfo> m_QueueCreateInfos;

			friend class Device;
	};
}