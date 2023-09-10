#include "pch.h"
#include "Device.h"
#include "RendererContext.h"
#include "Core/Logger.h"

namespace TRE
{
	VkPhysicalDevice PhysicalDevice::GetPhysicalDevice() const
	{
		return m_PhysicalDevice;
	}

	QueueFamilies PhysicalDevice::GetQueueFamilies()
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

	std::vector<VkDeviceQueueCreateInfo>& PhysicalDevice::GetQueueCreateInfos()
	{
		return m_QueueCreateInfos;
	}

	PhysicalDevice::PhysicalDevice()
	{
		uint32_t PhysicalDeviceCount = 0;
		vkEnumeratePhysicalDevices(RendererContext::GetVKInstance(), &PhysicalDeviceCount, nullptr);
		if (PhysicalDeviceCount == 0)
		{
			assert(false); //No GPU available
		}

		std::vector<VkPhysicalDevice> PhysicalDevice(PhysicalDeviceCount);
		if (auto Result = vkEnumeratePhysicalDevices(RendererContext::GetVKInstance(), &PhysicalDeviceCount, PhysicalDevice.data()); Result != VK_SUCCESS)
		{
			TRE_CORE_CRITICAL("Cannot get GPUs");
			assert(Result == VK_SUCCESS);
		}

		//Choose GPU that is discrete and ignore integrated ones.
		//Note that this does not account for computers with more than 1 discrete GPU.
		//It will just choose the first one that was iterated.
		TRE_CORE_INFO("Available GPUs:");
		for (VkPhysicalDevice physicalDevice : PhysicalDevice)
		{
			vkGetPhysicalDeviceProperties(physicalDevice, &m_Properties);
			if (m_Properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
			{
				TRE_CORE_INFO("GPU Chosen: {0}", m_Properties.deviceName);
				m_PhysicalDevice = physicalDevice;
				break;
			}
		}

		if (m_PhysicalDevice == VK_NULL_HANDLE) //Worst case scenario if no GPU found just use something
		{
			TRE_CORE_WARN("No discrete GPU found, using any GPU found");
			m_PhysicalDevice = PhysicalDevice.back();
			assert(m_PhysicalDevice);
		}

		vkGetPhysicalDeviceFeatures(m_PhysicalDevice, &m_Features);
		vkGetPhysicalDeviceMemoryProperties(m_PhysicalDevice, &m_MemoryProperties);

		uint32_t ExtensionCount;
		vkEnumerateDeviceExtensionProperties(m_PhysicalDevice, nullptr, &ExtensionCount, nullptr);
		if (ExtensionCount > 0)
		{
			std::vector<VkExtensionProperties> AvailableExtensions(ExtensionCount);
			if (auto Result = vkEnumerateDeviceExtensionProperties(m_PhysicalDevice, nullptr, &ExtensionCount, AvailableExtensions.data()); Result == VK_SUCCESS)
			{
				TRE_CORE_TRACE("This GPU has {0] extensions", ExtensionCount);
				for (const auto& extension : AvailableExtensions)
				{
					m_SupportedExtensions.emplace(extension.extensionName);
					TRE_CORE_TRACE("{0}", extension.extensionName);
				}
			}
		}

		uint32_t QueueFamilyCount;
		vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &QueueFamilyCount, nullptr);
		assert(QueueFamilyCount > 0);

		m_QueueFamilyProperties.resize(QueueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &QueueFamilyCount, m_QueueFamilyProperties.data());

		static const float DefaultQueuePriority = 0.f;
		int RequestedQueueTypes = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT;
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

		m_DepthFormat = GetDepthFormat();
		assert(m_DepthFormat);
	}

	PhysicalDevice::~PhysicalDevice()
	{

	}

	QueueFamilies PhysicalDevice::GetQueueFamilies(int flags)
	{
		QueueFamilies NewFamily;

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

		for (int x = 0; x < m_QueueFamilyProperties.size(); x++)
		{
			if ((flags & VK_QUEUE_COMPUTE_BIT) && NewFamily.Compute == -1)
			{
				if (m_QueueFamilyProperties[x].queueFlags & VK_QUEUE_COMPUTE_BIT)
					NewFamily.Compute = x;
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

	//-----------------------Device--------------------------//

	const std::shared_ptr<PhysicalDevice>& Device::GetPhysicalDevice() const
	{
		return m_PhysicalDevice;
	}

	VkDevice Device::GetLogicalDevice() const
	{
		return m_LogicalDevice;
	}

	VkQueue Device::GetGraphicsQ()
	{
		return m_GraphicsQ;
	}

	VkQueue Device::GetComputeQ()
	{
		return m_ComputeQ;
	}

	Device::Device(const std::shared_ptr<PhysicalDevice>& physicalDevice, VkPhysicalDeviceFeatures Features) : m_PhysicalDevice(physicalDevice)
	{
		auto QueueCreateInfos = m_PhysicalDevice->GetQueueCreateInfos();
		auto QueueFam = m_PhysicalDevice->GetQueueFamilies();

		const std::vector<const char*> m_DeviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
		VkDeviceCreateInfo deviceinfo{};
		deviceinfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceinfo.pQueueCreateInfos = QueueCreateInfos.data();
		deviceinfo.queueCreateInfoCount = static_cast<uint32_t>(QueueCreateInfos.size());
		deviceinfo.pEnabledFeatures = &Features;
		deviceinfo.enabledExtensionCount = static_cast<uint32_t>(m_DeviceExtensions.size());
		deviceinfo.ppEnabledExtensionNames = m_DeviceExtensions.data();

		const std::vector<const char*> m_ValidationLayers = { "VK_LAYER_KHRONOS_validation" };
		if (EnableValidationLayer)
		{
			deviceinfo.enabledLayerCount = static_cast<uint32_t>(m_ValidationLayers.size());
			deviceinfo.ppEnabledLayerNames = m_ValidationLayers.data();
		}
		else
		{
			deviceinfo.enabledLayerCount = 0;
		}

		if (auto Result = vkCreateDevice(m_PhysicalDevice->GetPhysicalDevice(), &deviceinfo, nullptr, &m_LogicalDevice); Result != VK_SUCCESS)
		{
			TRE_CORE_CRITICAL("Unable to create logical device");
			assert(Result == VK_SUCCESS);
		}

		vkGetDeviceQueue(m_LogicalDevice, QueueFam.Graphics, 0, &m_GraphicsQ);
		vkGetDeviceQueue(m_LogicalDevice, QueueFam.Compute, 0, &m_ComputeQ);

		VkCommandPoolCreateInfo CommandPoolCreateInfo{};
		CommandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		CommandPoolCreateInfo.queueFamilyIndex = QueueFam.Graphics;
		CommandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

		if (auto Result = vkCreateCommandPool(m_LogicalDevice, &CommandPoolCreateInfo, nullptr, &m_CommandPool); Result != VK_SUCCESS)
		{
			TRE_CORE_CRITICAL("Unable to create command pool for graphics");
			assert(Result == VK_SUCCESS);
		}

		CommandPoolCreateInfo.queueFamilyIndex = QueueFam.Compute;
		if (auto Result = vkCreateCommandPool(m_LogicalDevice, &CommandPoolCreateInfo, nullptr, &m_ComputeCommandPool); Result != VK_SUCCESS)
		{
			TRE_CORE_CRITICAL("Unable to create command pool for compute");
			assert(Result == VK_SUCCESS);
		}
	}

	Device::~Device()
	{

	}

	void Device::Destroy()
	{
		vkDestroyCommandPool(m_LogicalDevice, m_CommandPool, nullptr);
		vkDestroyCommandPool(m_LogicalDevice, m_ComputeCommandPool, nullptr);
		vkDeviceWaitIdle(m_LogicalDevice);
		vkDestroyDevice(m_LogicalDevice, nullptr);
	}

	VkCommandBuffer Device::AllocateCommandBuffer(bool BeginBuffer)
	{
		VkCommandBuffer CommandBuffer;
		VkCommandBufferAllocateInfo AllocateInfo{};
		AllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		AllocateInfo.commandPool = m_CommandPool;
		AllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		AllocateInfo.commandBufferCount = 1;

		if (auto Result = vkAllocateCommandBuffers(m_LogicalDevice, &AllocateInfo, &CommandBuffer); Result != VK_SUCCESS)
		{
			TRE_CORE_CRITICAL("Unable to allocate command buffer");
			assert(Result == VK_SUCCESS);
		}

		if (BeginBuffer)
		{
			VkCommandBufferBeginInfo BeginInfo{};
			BeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			if (auto Result = vkBeginCommandBuffer(CommandBuffer, &BeginInfo); Result != VK_SUCCESS)
			{
				TRE_CORE_ERROR("Unable to begin command buffer");
				assert(Result == VK_SUCCESS);
			}
		}

		return CommandBuffer;
	}

	void Device::SubmitCommands(VkCommandBuffer CommandBuffer)
	{
		assert(CommandBuffer != VK_NULL_HANDLE); //Cannot be empty

		if (auto Result = vkEndCommandBuffer(CommandBuffer); Result != VK_SUCCESS)
		{
			TRE_CORE_ERROR("Unable to end command buffer");
			assert(Result == VK_SUCCESS);
		}

		VkSubmitInfo SubmitInfo{};
		SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		SubmitInfo.commandBufferCount = 1;
		SubmitInfo.pCommandBuffers = &CommandBuffer;

		VkFence Fence;
		VkFenceCreateInfo FenceCreateInfo{};
		FenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		if (auto Result = vkCreateFence(m_LogicalDevice, &FenceCreateInfo, nullptr, &Fence); Result != VK_SUCCESS)
		{
			TRE_CORE_ERROR("Unable to create fence");
			assert(Result == VK_SUCCESS);
		}

		if (auto Result = vkQueueSubmit(m_GraphicsQ, 1, &SubmitInfo, Fence); Result != VK_SUCCESS)
		{
			TRE_CORE_ERROR("Unable to queue submit");
			assert(Result == VK_SUCCESS);
		}

		if (auto Result = vkWaitForFences(m_LogicalDevice, 1, &Fence, VK_TRUE, UINT64_MAX); Result != VK_SUCCESS)
		{
			TRE_CORE_ERROR("Unable to wait for fence");
			assert(Result == VK_SUCCESS);
		}

		vkDestroyFence(m_LogicalDevice, Fence, nullptr);
		vkFreeCommandBuffers(m_LogicalDevice, m_CommandPool, 1, &CommandBuffer);
	}

	VkCommandBuffer Device::AllocateSecondaryCommandBuffer()
	{
		VkCommandBuffer cmdBuffer;

		VkCommandBufferAllocateInfo cmdBufAllocateInfo = {};
		cmdBufAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cmdBufAllocateInfo.commandPool = m_CommandPool;
		cmdBufAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
		cmdBufAllocateInfo.commandBufferCount = 1;

		if (auto Result = vkAllocateCommandBuffers(m_LogicalDevice, &cmdBufAllocateInfo, &cmdBuffer); Result != VK_SUCCESS)
		{
			TRE_CORE_ERROR("unable to allocate secondary buffer");
			assert(Result == VK_SUCCESS);
		}
		return cmdBuffer;
	}

	uint32_t Device::FindMemoryType(uint32_t memorytypebits, VkMemoryPropertyFlags MemoryPropertyFlags)
	{
		VkPhysicalDeviceMemoryProperties MemoryProperties = m_PhysicalDevice->GetPhysicalDeviceMemoryProperties();

		for (uint32_t x = 0; x < MemoryProperties.memoryTypeCount; x++)
		{
			if ((memorytypebits & (1 << x)) && (MemoryProperties.memoryTypes[x].propertyFlags & MemoryPropertyFlags) == MemoryPropertyFlags)
			{
				return x;
			}
		}

		TRE_CORE_ERROR("Unable to find memory type");
		assert(false);
		return 0;
	}
}