#pragma once
#include "glm/glm.hpp"
#include "vulkan/vulkan.h"
#include "Image.h"

#define UINT32_T_CAST(n) static_cast<uint32_t>((n))

namespace TRE::vkUtils
{
	void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
	glm::quat Blend(const glm::quat& Start, float T, const glm::quat& End);
	bool IsDepthImage(const ImageFormat& Usage);
	VkFormat VulkanImageFormat(ImageFormat format);
	uint32_t BufferFindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
}