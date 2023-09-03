#include "pch.h"
#include "Device.h"
#include "RendererContext.h"
#include "Core/Logger.h"

namespace TRE
{
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

	Device::Device(const std::shared_ptr<PhysicalDevice>& physicalDevice, VkPhysicalDeviceFeatures Features)
	{
		m_PhysicalDevice = physicalDevice;

		QueueFamilies queuefamily = m_PhysicalDevice->FindQueueFamilies(m_PhysicalDevice->GetPhysicalDevice());

		std::vector<VkDeviceQueueCreateInfo> AllQueueInfos;
		std::set<int32_t> UniqueQueueFamilies = { queuefamily.Graphics, queuefamily.Present };

		float QueuePiority = 1.f;

		for (uint32_t QueueFamily : UniqueQueueFamilies)
		{
			VkDeviceQueueCreateInfo Queueinfo{};
			Queueinfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			Queueinfo.queueFamilyIndex = QueueFamily;
			Queueinfo.queueCount = 1;
			Queueinfo.pQueuePriorities = &QueuePiority;
			AllQueueInfos.push_back(Queueinfo);
		}

		VkPhysicalDeviceFeatures physicalfeatures{}; //Later
		physicalfeatures.samplerAnisotropy = VK_TRUE;

		const std::vector<const char*> m_DeviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
		VkDeviceCreateInfo deviceinfo{};
		deviceinfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceinfo.pQueueCreateInfos = AllQueueInfos.data();
		deviceinfo.queueCreateInfoCount = static_cast<uint32_t>(AllQueueInfos.size());
		deviceinfo.pEnabledFeatures = &physicalfeatures;
		deviceinfo.enabledExtensionCount = static_cast<uint32_t>(m_DeviceExtensions.size());
		deviceinfo.ppEnabledExtensionNames = m_DeviceExtensions.data();

		if (EnableValidationLayer)
		{
			deviceinfo.enabledLayerCount = static_cast<uint32_t>(m_ValidationLayers.size());
			deviceinfo.ppEnabledLayerNames = m_ValidationLayers.data();
		}
		else
		{
			deviceinfo.enabledLayerCount = 0;
		}

		if (vkCreateDevice(m_PhysicalDevice->GetPhysicalDevice(), &deviceinfo, nullptr, &m_LogicalDevice) != VK_SUCCESS)
		{
			assert(false);
		}

		vkGetDeviceQueue(m_LogicalDevice, queuefamily.Graphics, 0, &m_GraphicsQ);
		vkGetDeviceQueue(m_LogicalDevice, queuefamily.Present, 0, &m_ComputeQ);

		//VkDeviceCreateInfo DeviceCreateInfo = {};
		//DeviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		//DeviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(physicalDevice->m_QueueCreateInfos.size());
		//DeviceCreateInfo.pQueueCreateInfos = physicalDevice->m_QueueCreateInfos.data();
		//DeviceCreateInfo.pEnabledFeatures = &Features;
		//DeviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(DeviceLevelExtensions.size());
		//DeviceCreateInfo.ppEnabledExtensionNames = DeviceLevelExtensions.data();

		VkCommandPoolCreateInfo CommandPoolCreateInfo{};
		CommandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		CommandPoolCreateInfo.queueFamilyIndex = m_PhysicalDevice->GetQueueFamilies().Graphics;
		CommandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

		if (auto Result = vkCreateCommandPool(m_LogicalDevice, &CommandPoolCreateInfo, nullptr, &m_CommandPool); Result != VK_SUCCESS)
		{
			TRE_CORE_CRITICAL("Unable to create command pool for graphics");
			assert(Result == VK_SUCCESS);
		}

		CommandPoolCreateInfo.queueFamilyIndex = m_PhysicalDevice->GetQueueFamilies().Present;
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