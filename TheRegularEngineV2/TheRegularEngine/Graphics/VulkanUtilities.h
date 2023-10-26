#pragma once
#include "glm/glm.hpp"
#include "vulkan/vulkan.h"
#include "Image.h"

namespace TRE::vkUtils
{
	void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
	glm::quat Blend(const glm::quat& Start, float T, const glm::quat& End);
	bool IsDepthImage(const ImageFormat& Usage);
	VkFormat VulkanImageFormat(ImageFormat format);
}