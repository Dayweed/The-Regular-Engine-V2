#include "pch.h"
#include "VulkanUtilities.h"
#include "RendererContext.h"

namespace TRE::vkUtils
{
	void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
	{
		auto logicalDevice = RendererContext::GetDevice();
		VkCommandBuffer commandBuffer = logicalDevice->AllocateCommandBuffer(true);

		VkBufferCopy copyRegion{};
		copyRegion.srcOffset = 0;  // Optional
		copyRegion.dstOffset = 0;  // Optional
		copyRegion.size = size;
		vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

		logicalDevice->SubmitCommands(commandBuffer);
	}
}