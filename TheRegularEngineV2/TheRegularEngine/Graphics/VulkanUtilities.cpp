#include "pch.h"
#include "VulkanUtilities.h"
#include "RendererContext.h"
#include "glm/gtx/matrix_decompose.hpp"

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

    glm::quat Blend(const glm::quat& Start, float T, const glm::quat& End)
    {
        bool bFlip = false;

        // Determine if quats are further than 90 degrees
        float Cs = Start.x * End.x + Start.y * End.y + Start.z * End.z + Start.w * End.w;
        if (Cs < 0.0f)
        {
            Cs = -Cs;
            bFlip = !bFlip;
        }

        float inv_T;
        if ((1.0f - Cs) < 0.000001f)
        {
            inv_T = 1.0f - T;
        }
        else
        {
            const float Theta = acosf(Cs);
            const float S = 1.0f / sinf(Theta);

            inv_T = sinf((1.0f - T) * Theta) * S;
            T = sinf((T * Theta)) * S;
        }

        if (bFlip)
        {
            T = -T;
        }

        return End * T + (Start)*inv_T;
    }
}