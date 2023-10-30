#include "pch.h"
#include "CommandBuffer.h"
#include "Core/Logger.h"
#include "Core/Engine.h"

namespace TRE
{
	VkCommandBuffer CommandBuffer::GetInUseCommandBuffer()
	{
		return m_InUseCommandBuffer;
	}

	VkCommandBuffer CommandBuffer::GetCommandBuffer(uint32_t Index)
	{
		assert(Index < m_Commandbuffers.size() && "Command Buffer out of index");
		return m_Commandbuffers[Index];
	}

	CommandBuffer::CommandBuffer(std::string DebugName) : m_DebugName(std::move(DebugName)), m_SwapChainCB(false)
	{
		auto Device = RendererContext::GetDevice();
		uint32_t FramesInFlight = RendererContext::GetFramesInFlight();

		VkCommandPoolCreateInfo CommandPoolCreateInfo{};
		CommandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		CommandPoolCreateInfo.queueFamilyIndex = Device->GetPhysicalDevice()->GetQueueFamilies().Graphics;
		CommandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

		if (auto Result = vkCreateCommandPool(Device->GetLogicalDevice(), &CommandPoolCreateInfo, nullptr, &m_CommandPool); Result != VK_SUCCESS)
		{
			assert(Result == VK_SUCCESS && "Unable to create command pool");
		}

		VkCommandBufferAllocateInfo CBAllocateInfo{};
		CBAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		CBAllocateInfo.commandPool = m_CommandPool;
		CBAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		CBAllocateInfo.commandBufferCount = FramesInFlight;
		m_Commandbuffers.resize(FramesInFlight);
		
		if (auto Result = vkAllocateCommandBuffers(Device->GetLogicalDevice(), &CBAllocateInfo, m_Commandbuffers.data()); Result != VK_SUCCESS)
		{
			assert(Result == VK_SUCCESS && "Unable to allocate command buffers");
		}

		VkFenceCreateInfo FenceCreateInfo{};
		FenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		FenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		m_Fences.resize(FramesInFlight);
		
		for (uint32_t x = 0; x < FramesInFlight; x++)
		{
			if (auto Result = vkCreateFence(Device->GetLogicalDevice(), &FenceCreateInfo, nullptr, &m_Fences[x]); Result != VK_SUCCESS)
			{
				assert(Result == VK_SUCCESS && "Unable to create fences");
			}
		}
	}

	//We don't have to create command pool or allocate any commandbuffers here since we are using the ones from swapchain directly
	CommandBuffer::CommandBuffer(std::string DebugName, bool SwapChain) : m_DebugName(std::move(DebugName)), m_SwapChainCB(true)
	{
		(void)SwapChain;
	}

	CommandBuffer::~CommandBuffer()
	{
		if (m_SwapChainCB)
			return;

		auto Device = RendererContext::GetDevice()->GetLogicalDevice();
		vkDestroyCommandPool(Device, m_CommandPool, nullptr);
		for (int x = 0; x < m_Fences.size(); x++)
		{
			vkDestroyFence(Device, m_Fences[x], nullptr);
		}
	}

	void CommandBuffer::Begin()
	{
		VkCommandBufferBeginInfo CommandBufferBeginInfo{}; //pNext nullptr
		CommandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		CommandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		VkCommandBuffer TargetCommandBuffer = nullptr;
		if (m_SwapChainCB)
		{
			auto& SwapChain = Engine::GetInstance().GetWindow()->GetSwapChain();
			TargetCommandBuffer = SwapChain->GetCurrentCommandBuffer();
		}
		else
		{
			TargetCommandBuffer = m_Commandbuffers[Engine::GetInstance().GetWindow()->GetSwapChain()->GetCurrentBufferIndex()];
		}

		m_InUseCommandBuffer = TargetCommandBuffer;
		if (auto Result = vkBeginCommandBuffer(TargetCommandBuffer, &CommandBufferBeginInfo); Result != VK_SUCCESS)
		{
			TRE_CORE_ERROR("Unable to begin command buffer from {0}", m_DebugName);
			assert(Result == VK_SUCCESS);
		}
	}

	void CommandBuffer::End()
	{
		vkEndCommandBuffer(m_InUseCommandBuffer);
		m_InUseCommandBuffer = nullptr;
	}

	void CommandBuffer::Submit()
	{
		if (m_SwapChainCB) //Swapchain's commandbuffer is submitted by swapchain himself
			return;

		auto Device = RendererContext::GetDevice();
		uint32_t Index = Engine::GetInstance().GetWindow()->GetSwapChain()->GetCurrentBufferIndex();
		VkCommandBuffer TargetCommandBuffer = m_Commandbuffers[Index];

		VkSubmitInfo SubmitInfo{};
		SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		SubmitInfo.commandBufferCount = 1;
		SubmitInfo.pCommandBuffers = &TargetCommandBuffer;

		if (auto Result = vkWaitForFences(Device->GetLogicalDevice(), 1, &m_Fences[Index], VK_TRUE, UINT64_MAX); Result != VK_SUCCESS)
		{
			assert(Result == VK_SUCCESS && "Failed to wait on fence");
		}

		if (auto Result = vkResetFences(Device->GetLogicalDevice(), 1, &m_Fences[Index]); Result != VK_SUCCESS)
		{
			assert(Result == VK_SUCCESS && "Failed to reset fence");
		}

		if (auto Result = vkQueueSubmit(Device->GetGraphicsQ(), 1, &SubmitInfo, m_Fences[Index]); Result != VK_SUCCESS)
		{
			assert(Result == VK_SUCCESS && "Failed to submit commands");
		}
	}
}