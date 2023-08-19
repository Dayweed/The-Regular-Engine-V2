#include "pch.h"
#include "PhysicalDevice.h"
#include "RendererContext.h"

namespace TRE
{
	VkPhysicalDevice PhysicalDevice::GetPhysicalDevice() const
	{
		return m_PhysicalDevice;
	}

	PhysicalDevice::QueueFamilyIndices PhysicalDevice::GetQueueFamilies()
	{
		return m_QueueFamilies;
	}

	VkPhysicalDeviceProperties PhysicalDevice::GetPhysicalDeviceProperties()
	{
		return m_Properties;
	}

	VkPhysicalDeviceMemoryProperties PhysicalDevice::GetPhysicalDeviceMemoryProperties()
	{
		return m_MemoryProperties;
	}

	PhysicalDevice::PhysicalDevice()
	{
		auto VulkanInstance = RendererContext::GetVKInstance();

		uint32_t GPUcount;
		vkEnumeratePhysicalDevices(VulkanInstance, &GPUcount, nullptr);
		if (GPUcount == 0)
		{
			std::cout << "No GPU suitable to support vulkan" << std::endl;
			assert(false);
		}

		std::vector<VkPhysicalDevice> PhysicalDevices(GPUcount);
		if (auto Result = vkEnumeratePhysicalDevices(VulkanInstance, &GPUcount, PhysicalDevices.data()); Result != VK_SUCCESS)
		{
			std::cout << "Cannot get GPUs" << std::endl;
			assert(Result == VK_SUCCESS);
		}

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

		if (m_PhysicalDevice == nullptr) //Worst case scenario
		{
			std::cout << "No discrete GPU found, using integrated" << std::endl;
			m_PhysicalDevice = PhysicalDevices.back();
			assert(m_PhysicalDevice);
		}

		vkGetPhysicalDeviceFeatures(m_PhysicalDevice, &m_Features);
		vkGetPhysicalDeviceMemoryProperties(m_PhysicalDevice, &m_MemoryProperties);

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
			std::cout << std::endl;
		}

		uint32_t QueueFamilyCount;
		vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &QueueFamilyCount, nullptr);
		assert(QueueFamilyCount > 0);

		m_QueueFamilyProperties.resize(QueueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &QueueFamilyCount, m_QueueFamilyProperties.data());

		static const float DefaultQueuePriority = 0.f;
		int RequestedQueueTypes = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT;
		m_QueueFamilies = GetQueueFamilies(RequestedQueueTypes);

		if (RequestedQueueTypes & VK_QUEUE_GRAPHICS_BIT)
		{
			VkDeviceQueueCreateInfo QueueCreateInfo{};
			QueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			QueueCreateInfo.queueFamilyIndex = m_QueueFamilies.Graphics;
			QueueCreateInfo.queueCount = 1;
			QueueCreateInfo.pQueuePriorities = &DefaultQueuePriority;
			m_QueueCreateInfos.push_back(QueueCreateInfo);
		}

		if (RequestedQueueTypes & VK_QUEUE_COMPUTE_BIT)
		{
			if (m_QueueFamilies.Compute != m_QueueFamilies.Graphics) //If same no need create twice
			{
				VkDeviceQueueCreateInfo QueueCreateInfo{};
				QueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
				QueueCreateInfo.queueFamilyIndex = m_QueueFamilies.Compute;
				QueueCreateInfo.queueCount = 1;
				QueueCreateInfo.pQueuePriorities = &DefaultQueuePriority;
				m_QueueCreateInfos.push_back(QueueCreateInfo);
			}
		}

		if (RequestedQueueTypes & VK_QUEUE_TRANSFER_BIT)
		{
			if ((m_QueueFamilies.Compute != m_QueueFamilies.Transfer) && (m_QueueFamilies.Graphics != m_QueueFamilies.Transfer)) //If same no need create twice
			{
				VkDeviceQueueCreateInfo QueueCreateInfo{};
				QueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
				QueueCreateInfo.queueFamilyIndex = m_QueueFamilies.Transfer;
				QueueCreateInfo.queueCount = 1;
				QueueCreateInfo.pQueuePriorities = &DefaultQueuePriority;
				m_QueueCreateInfos.push_back(QueueCreateInfo);
			}
		}

		m_DepthFormat = GetDepthFormat();
		assert(m_DepthFormat);
	}

	PhysicalDevice::~PhysicalDevice()
	{

	}

	PhysicalDevice::QueueFamilyIndices PhysicalDevice::GetQueueFamilies(int flags)
	{
		QueueFamilyIndices NewFamily;

		if (flags & VK_QUEUE_COMPUTE_BIT)
		{
			for (int x = 0; x < m_QueueFamilyProperties.size(); x++)
			{
				auto& properties = m_QueueFamilyProperties[x];
				if ((properties.queueFlags & VK_QUEUE_COMPUTE_BIT) && ((properties.queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0))
				{
					NewFamily.Compute = x;
					break;
				}
			}
		}

		if (flags & VK_QUEUE_TRANSFER_BIT)
		{
			for (int x = 0; x < m_QueueFamilyProperties.size(); x++)
			{
				auto& properties = m_QueueFamilyProperties[x];
				if ((properties.queueFlags & VK_QUEUE_TRANSFER_BIT) && ((properties.queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0) 
																	&& ((properties.queueFlags & VK_QUEUE_COMPUTE_BIT) == 0))
				{
					NewFamily.Compute = x;
					break;
				}
			}
		}

		for (int x = 0; x < m_QueueFamilyProperties.size(); x++)
		{
			if ((flags & VK_QUEUE_COMPUTE_BIT) && NewFamily.Compute == -1)
			{
				if (m_QueueFamilyProperties[x].queueFlags & VK_QUEUE_COMPUTE_BIT)
					NewFamily.Compute = x;
			}

			if ((flags & VK_QUEUE_TRANSFER_BIT) && NewFamily.Transfer == -1)
			{
				if (m_QueueFamilyProperties[x].queueFlags & VK_QUEUE_TRANSFER_BIT)
					NewFamily.Transfer = x;
			}

			if (flags & VK_QUEUE_GRAPHICS_BIT)
			{
				if (m_QueueFamilyProperties[x].queueFlags & VK_QUEUE_GRAPHICS_BIT)
					NewFamily.Graphics = x;
			}
		}

		return NewFamily;
	}

	VkFormat PhysicalDevice::GetDepthFormat()
	{
		std::vector<VkFormat> depthFormats = 
		{
			VK_FORMAT_D32_SFLOAT_S8_UINT,
			VK_FORMAT_D32_SFLOAT,
			VK_FORMAT_D24_UNORM_S8_UINT,
			VK_FORMAT_D16_UNORM_S8_UINT,
			VK_FORMAT_D16_UNORM
		};

		for (auto& format : depthFormats)
		{
			VkFormatProperties formatProps;
			vkGetPhysicalDeviceFormatProperties(m_PhysicalDevice, format, &formatProps);
			if (formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
				return format;
		}

		return VK_FORMAT_UNDEFINED;
	}

	bool PhysicalDevice::IsExtensionSupported(const std::string& Extension)
	{
		return m_SupportedExtensions.find(Extension) != m_SupportedExtensions.end();
	}
}