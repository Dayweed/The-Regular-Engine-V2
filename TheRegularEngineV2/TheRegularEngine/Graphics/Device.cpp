#include "pch.h"
#include "Device.h"

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

		std::cout << "Unable to find memory type" << std::endl;
		assert(false);
		return 0;
	}

	Device::Device(const std::shared_ptr<PhysicalDevice>& physicalDevice, VkPhysicalDeviceFeatures Features)
	{
		m_PhysicalDevice = physicalDevice;
		std::vector<const char*> DeviceLevelExtensions;
		assert(m_PhysicalDevice->IsExtensionSupported(VK_KHR_SWAPCHAIN_EXTENSION_NAME)); //cannot dun have else cannot present on screen
		DeviceLevelExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

		VkDeviceCreateInfo DeviceCreateInfo = {};
		DeviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		DeviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(physicalDevice->m_QueueCreateInfos.size());
		DeviceCreateInfo.pQueueCreateInfos = physicalDevice->m_QueueCreateInfos.data();
		DeviceCreateInfo.pEnabledFeatures = &Features;
		DeviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(DeviceLevelExtensions.size());
		DeviceCreateInfo.ppEnabledExtensionNames = DeviceLevelExtensions.data();

		if (auto Result = vkCreateDevice(m_PhysicalDevice->m_PhysicalDevice, &DeviceCreateInfo, nullptr, &m_LogicalDevice); Result != VK_SUCCESS)
		{
			std::cout << "Unable to create Logical Device" << std::endl;
			assert(Result == VK_SUCCESS);
		}

		VkCommandPoolCreateInfo CommandPoolCreateInfo{};
		CommandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		CommandPoolCreateInfo.queueFamilyIndex = m_PhysicalDevice->m_QueueFamilies.Graphics;
		CommandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

		if (auto Result = vkCreateCommandPool(m_LogicalDevice, &CommandPoolCreateInfo, nullptr, &m_CommandPool); Result != VK_SUCCESS)
		{
			std::cout << "Unable to create command pool for graphics" << std::endl;
			assert(Result == VK_SUCCESS);
		}

		CommandPoolCreateInfo.queueFamilyIndex = m_PhysicalDevice->m_QueueFamilies.Compute;
		if (auto Result = vkCreateCommandPool(m_LogicalDevice, &CommandPoolCreateInfo, nullptr, &m_ComputeCommandPool); Result != VK_SUCCESS)
		{
			std::cout << "Unable to create command pool for compute" << std::endl;
			assert(Result == VK_SUCCESS);
		}

		vkGetDeviceQueue(m_LogicalDevice, m_PhysicalDevice->m_QueueFamilies.Graphics, 0, &m_GraphicsQ);
		vkGetDeviceQueue(m_LogicalDevice, m_PhysicalDevice->m_QueueFamilies.Compute, 0, &m_ComputeQ);
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
			std::cout << "Unable to allocate command buffer" << std::endl;
			assert(Result == VK_SUCCESS);
		}

		if (BeginBuffer)
		{
			VkCommandBufferBeginInfo BeginInfo{};
			BeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			if (auto Result = vkBeginCommandBuffer(CommandBuffer, &BeginInfo); Result != VK_SUCCESS)
			{
				std::cout << "Unable to begin command buffer" << std::endl;
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
			std::cout << "Unable to end command buffer" << std::endl;
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
			std::cout << "Unable to create fence" << std::endl;
			assert(Result == VK_SUCCESS);
		}

		if (auto Result = vkQueueSubmit(m_GraphicsQ, 1, &SubmitInfo, Fence); Result != VK_SUCCESS)
		{
			std::cout << "Unable to queue submit" << std::endl;
			assert(Result == VK_SUCCESS);
		}

		if (auto Result = vkWaitForFences(m_LogicalDevice, 1, &Fence, VK_TRUE, UINT64_MAX); Result != VK_SUCCESS)
		{
			std::cout << "Unable to wait for fence" << std::endl;
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
			std::cout << "unable to allocate secondary buffer" << std::endl;
			assert(Result == VK_SUCCESS);
		}
		return cmdBuffer;
	}
}