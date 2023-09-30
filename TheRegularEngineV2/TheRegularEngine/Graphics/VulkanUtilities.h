#pragma once
#include "glm/glm.hpp"

namespace TRE::vkUtils
{
	void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
	glm::quat Blend(const glm::quat& Start, float T, const glm::quat& End);
}