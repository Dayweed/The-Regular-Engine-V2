#include "pch.h"
#include "LogicalDevice.h"

namespace TRE
{
	LogicalDevice::LogicalDevice(const std::shared_ptr<PhysicalDevice>& physicaldevice, VkPhysicalDeviceFeatures Features) : m_PhysicalDevice(physicaldevice)
	{
		std::vector<const char*> Extensions;
		
		//Add some assert to guard against GPU that doesn't support this for some magical reason
		Extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

		VkDeviceCreateInfo LogicalDeviceCreateInfo{};
		LogicalDeviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		LogicalDeviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(m_PhysicalDevice->m_QueueCreateInfo.size());
		LogicalDeviceCreateInfo.pQueueCreateInfos = m_PhysicalDevice->m_QueueCreateInfo.data();
		LogicalDeviceCreateInfo.pEnabledFeatures = &Features;

		if (!Extensions.empty())
		{
			LogicalDeviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(Extensions.size());
			LogicalDeviceCreateInfo.ppEnabledExtensionNames = Extensions.data();
		}

		if (VkResult Result = vkCreateDevice(m_PhysicalDevice->GetPhysicalDevice(), &LogicalDeviceCreateInfo, nullptr, &m_LogicalDevice); Result != VK_SUCCESS)
		{
			//Change proper assert message
			assert(Result == VK_SUCCESS);
		}

		vkGetDeviceQueue(m_LogicalDevice, m_PhysicalDevice->m_QueueFamily.Graphics, 0, &m_GraphicsQueue);
		vkGetDeviceQueue(m_LogicalDevice, m_PhysicalDevice->m_QueueFamily.Compute, 0, &m_ComputeQueue);
	}

	LogicalDevice::~LogicalDevice()
	{

	}

	VkDevice LogicalDevice::GetLogicalDevice() const
	{
		return m_LogicalDevice;
	}

	const std::shared_ptr<PhysicalDevice>& LogicalDevice::GetPhysicalDevice() const
	{
		return m_PhysicalDevice;
	}
}