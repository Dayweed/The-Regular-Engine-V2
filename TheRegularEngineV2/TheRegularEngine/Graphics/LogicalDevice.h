#pragma once
#include "vulkan/vulkan.h"
#include "PhysicalDevice.h"

namespace TRE
{
	class LogicalDevice
	{
		public:
			LogicalDevice(const std::shared_ptr<PhysicalDevice>& physicaldevice, VkPhysicalDeviceFeatures Features);
			~LogicalDevice();

			VkDevice GetLogicalDevice() const;
			const std::shared_ptr<PhysicalDevice>& GetPhysicalDevice() const;


		private:
			VkDevice m_LogicalDevice = nullptr;

			std::shared_ptr<PhysicalDevice> m_PhysicalDevice;
			VkPhysicalDeviceFeatures m_PhysicalDeviceFeatures;

			VkQueue m_GraphicsQueue;
			VkQueue m_ComputeQueue;
	};
}