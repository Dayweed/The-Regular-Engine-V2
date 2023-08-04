#pragma once

namespace TRE
{
	class PhysicalDevice
	{
		public:
			struct QueueFamily
			{
				int32_t Compute = -1;
				int32_t Graphics = -1;
				int32_t Transfer = -1;
			};

			PhysicalDevice();
			~PhysicalDevice();

			VkPhysicalDevice GetPhysicalDevice() const;
			const VkPhysicalDeviceProperties& GetPhysicalDeviceProperties() const;
			const QueueFamily& GetQueueFamily() const;
			VkFormat GetDepthFormat() const;

		private:
			QueueFamily GetQueueFamily(int RequestedQueues);
			VkFormat FindDepthFormat() const;

		private:
			VkPhysicalDevice m_PhysicalDevice;
			VkPhysicalDeviceProperties m_Properties;
			VkPhysicalDeviceFeatures m_Features;
			VkPhysicalDeviceMemoryProperties m_MemoryProperties;
			
			QueueFamily m_QueueFamily;
			std::vector<VkQueueFamilyProperties> m_QueueFamilyProperties;
			std::vector<VkDeviceQueueCreateInfo> m_QueueCreateInfo;

			std::set<std::string> m_SupportedExtensions;

			VkFormat m_DepthFormat;

			friend class LogicalDevice;
	};
}