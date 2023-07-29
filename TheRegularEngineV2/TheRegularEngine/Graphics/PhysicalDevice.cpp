#include "pch.h"
#include "PhysicalDevice.h"
#include "RendererContext.h"

namespace TRE
{
	PhysicalDevice::~PhysicalDevice()
	{

	}

	VkPhysicalDevice PhysicalDevice::GetPhysicalDevice() const
	{
		return m_PhysicalDevice;
	}

	const VkPhysicalDeviceProperties& PhysicalDevice::GetPhysicalDeviceProperties() const
	{
		return m_Properties;
	}

	const PhysicalDevice::QueueFamily& PhysicalDevice::GetQueueFamily() const
	{
		return m_QueueFamily;
	}

	VkFormat PhysicalDevice::GetDepthFormat() const
	{
		return m_DepthFormat;
	}

	PhysicalDevice::PhysicalDevice()
	{
		uint32_t DeviceCount = 0;
		VkResult result = vkEnumeratePhysicalDevices(RendererContext::GetVKInstance(), &DeviceCount, nullptr);
		assert(result == VK_SUCCESS);

		if (DeviceCount == 0)
		{
			std::cout << "No GPU suitable to support vulkan" << std::endl;
		}

		std::vector<VkPhysicalDevice> PhysicalDevices(DeviceCount);
		result = vkEnumeratePhysicalDevices(RendererContext::GetVKInstance(), &DeviceCount, PhysicalDevices.data());
		assert(result == VK_SUCCESS);

		//Choose GPU that is discrete and ignore integrated ones.
		//Note that this does not account for computers with more than 1 discrete GPU.
		//It will just choose the first one that was iterated.
		for (VkPhysicalDevice PhysicalDevice : PhysicalDevices) //Pointer no const ref
		{
			vkGetPhysicalDeviceProperties(PhysicalDevice, &m_Properties);
			if (m_Properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
			{
				m_PhysicalDevice = PhysicalDevice;
				break;
			}
		}

		//In case no discrete GPU is found, we will just take whatever.
		if (!m_PhysicalDevice)
		{
			std::cout << "No discrete GPU found." << std::endl;
			m_PhysicalDevice = PhysicalDevices.back();
		}

		vkGetPhysicalDeviceFeatures(m_PhysicalDevice, &m_Features);
		vkGetPhysicalDeviceMemoryProperties(m_PhysicalDevice, &m_MemoryProperties);

		uint32_t QueueFamilyCount;
		vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &QueueFamilyCount, nullptr);
		//Add proper assert if no more than 0
		assert(QueueFamilyCount > 0);

		m_QueueFamilyProperties.resize(QueueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &QueueFamilyCount, m_QueueFamilyProperties.data());

		uint32_t ExtensionsCount;
		vkEnumerateDeviceExtensionProperties(m_PhysicalDevice, nullptr, &ExtensionsCount, nullptr);
		if (ExtensionsCount > 0)
		{
			std::vector<VkExtensionProperties> Extensions(ExtensionsCount);
			if (vkEnumerateDeviceExtensionProperties(m_PhysicalDevice, nullptr, &ExtensionsCount, Extensions.data()) == VK_SUCCESS)
			{
				std::cout << std::endl;
				std::cout << "Supported Extensions from this GPU: " << std::endl;
				for (const auto& Ext : Extensions)
				{
					m_SupportedExtensions.emplace(Ext.extensionName);
					std::cout << Ext.extensionName << std::endl;
				}
			}
		}

		int RequestedQeues = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT;
		m_QueueFamily = GetQueueFamily(RequestedQeues);
		static const float DefaultPriority = 0.f;

		if (RequestedQeues & VK_QUEUE_GRAPHICS_BIT)
		{
			VkDeviceQueueCreateInfo QueueCreateInfo{};
			QueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			QueueCreateInfo.queueFamilyIndex = m_QueueFamily.Graphics;
			QueueCreateInfo.queueCount = 1;
			QueueCreateInfo.pQueuePriorities = &DefaultPriority;
			m_QueueCreateInfo.push_back(QueueCreateInfo);
		}

		if (RequestedQeues & VK_QUEUE_COMPUTE_BIT)
		{
			if (m_QueueFamily.Compute != m_QueueFamily.Graphics)
			{
				VkDeviceQueueCreateInfo QueueCreateInfo{};
				QueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
				QueueCreateInfo.queueFamilyIndex = m_QueueFamily.Compute;
				QueueCreateInfo.queueCount = 1;
				QueueCreateInfo.pQueuePriorities = &DefaultPriority;
				m_QueueCreateInfo.push_back(QueueCreateInfo);
			}
		}

		if (RequestedQeues & VK_QUEUE_TRANSFER_BIT)
		{
			if ((m_QueueFamily.Transfer != m_QueueFamily.Graphics) && (m_QueueFamily.Compute != m_QueueFamily.Transfer))
			{
				VkDeviceQueueCreateInfo QueueCreateInfo{};
				QueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
				QueueCreateInfo.queueFamilyIndex = m_QueueFamily.Transfer;
				QueueCreateInfo.queueCount = 1;
				QueueCreateInfo.pQueuePriorities = &DefaultPriority;
				m_QueueCreateInfo.push_back(QueueCreateInfo);
			}
		}
		
		m_DepthFormat = FindDepthFormat();
		assert(m_DepthFormat);
	}

	VkFormat PhysicalDevice::FindDepthFormat() const
	{
		std::vector<VkFormat> DepthFormats = 
		{
			VK_FORMAT_D32_SFLOAT_S8_UINT,
			VK_FORMAT_D32_SFLOAT,
			VK_FORMAT_D24_UNORM_S8_UINT,
			VK_FORMAT_D16_UNORM_S8_UINT,
			VK_FORMAT_D16_UNORM
		};

		for (auto& Format : DepthFormats)
		{
			VkFormatProperties FormatProperties;
			vkGetPhysicalDeviceFormatProperties(m_PhysicalDevice, Format, &FormatProperties);
			// Format must support depth stencil attachment for optimal tiling
			if (FormatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
				return Format;
		}
		return VK_FORMAT_UNDEFINED;
	}

	PhysicalDevice::QueueFamily PhysicalDevice::GetQueueFamily(int RequestedQueues)
	{
		QueueFamily Queue;

		if (RequestedQueues & VK_QUEUE_COMPUTE_BIT)
		{
			for (int x = 0; x < m_QueueFamilyProperties.size(); x++)
			{
				if ((m_QueueFamilyProperties[x].queueFlags & VK_QUEUE_COMPUTE_BIT) && (!(m_QueueFamilyProperties[x].queueFlags & VK_QUEUE_GRAPHICS_BIT)))
				{
					Queue.Compute = x;
					break;
				}
			}
		}

		if (RequestedQueues & VK_QUEUE_TRANSFER_BIT)
		{
			for (int x = 0; x < m_QueueFamilyProperties.size(); x++)
			{
				if ((m_QueueFamilyProperties[x].queueFlags & VK_QUEUE_TRANSFER_BIT) &&
					(!(m_QueueFamilyProperties[x].queueFlags & VK_QUEUE_COMPUTE_BIT)) && 
					(!(m_QueueFamilyProperties[x].queueFlags & VK_QUEUE_GRAPHICS_BIT)))
				{
					Queue.Transfer = x;
					break;
				}
			}
		}

		if (RequestedQueues & VK_QUEUE_GRAPHICS_BIT)
		{
			for (int x = 0; x < m_QueueFamilyProperties.size(); x++)
			{
				if ((!(m_QueueFamilyProperties[x].queueFlags & VK_QUEUE_TRANSFER_BIT)) &&
					(!(m_QueueFamilyProperties[x].queueFlags & VK_QUEUE_COMPUTE_BIT)) &&
					(m_QueueFamilyProperties[x].queueFlags & VK_QUEUE_GRAPHICS_BIT))
				{
					Queue.Graphics = x;
					break;
				}
			}
		}

		for (int x = 0; x < m_QueueFamilyProperties.size(); x++)
		{
			if ((RequestedQueues & VK_QUEUE_TRANSFER_BIT) && Queue.Transfer == -1)
			{
				if (m_QueueFamilyProperties[x].queueFlags & VK_QUEUE_TRANSFER_BIT)
					Queue.Transfer = x;
			}

			if ((RequestedQueues & VK_QUEUE_COMPUTE_BIT) && Queue.Compute == -1)
			{
				if (m_QueueFamilyProperties[x].queueFlags & VK_QUEUE_COMPUTE_BIT)
					Queue.Compute = x;
			}

			if (RequestedQueues & VK_QUEUE_GRAPHICS_BIT)
			{
				if (m_QueueFamilyProperties[x].queueFlags & VK_QUEUE_GRAPHICS_BIT)
					Queue.Graphics = x;
			}
		}

		return Queue;
	}
}