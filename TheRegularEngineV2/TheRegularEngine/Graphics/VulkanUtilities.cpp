#include "pch.h"
#include "VulkanUtilities.h"
#include "RendererContext.h"
#include "glm/gtx/matrix_decompose.hpp"
#include "Core/Engine.h"

namespace TRE::vkUtils
{
	VkBool32 formatIsFilterable(VkPhysicalDevice physicalDevice, VkFormat format, VkImageTiling tiling)
	{
		VkFormatProperties formatProps;
		vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &formatProps);

		if (tiling == VK_IMAGE_TILING_OPTIMAL)
			return formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT;

		if (tiling == VK_IMAGE_TILING_LINEAR)
			return formatProps.linearTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT;

		return false;
	}

	uint32_t BufferFindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
	{
		VkPhysicalDeviceMemoryProperties memProperties;
		auto physicalDevice = RendererContext::GetDevice()->GetPhysicalDevice()->GetPhysicalDevice();
		vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

		for (uint32_t x = 0; x < memProperties.memoryTypeCount; x++)
		{
			if ((typeFilter & (1 << x)) && (memProperties.memoryTypes[x].propertyFlags & properties) == properties)
			{
				return x;
			}
		}

		assert(false && "Unable to find a suitable buffer memory");
		return {};
	}

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

    bool IsDepthImage(const ImageFormat& Usage)
    {
        return Usage == ImageFormat::DEPTH24STENCIL8 || Usage == ImageFormat::DEPTH32F || Usage == ImageFormat::DEPTH32FSTENCIL8UINT || Usage == ImageFormat::DEPTH16UN;
    }

	VkFormat VulkanImageFormat(ImageFormat format)
	{
		switch (format)
		{
			case ImageFormat::RED8UN:               return VK_FORMAT_R8_UNORM;
			case ImageFormat::RED8UI:               return VK_FORMAT_R8_UINT;
			case ImageFormat::RED16UI:               return VK_FORMAT_R16_UINT;
			case ImageFormat::RED32UI:               return VK_FORMAT_R32_UINT;
			case ImageFormat::RED32F:				return VK_FORMAT_R32_SFLOAT;
			case ImageFormat::RG8:				    return VK_FORMAT_R8G8_UNORM;
			case ImageFormat::RG16F:				return VK_FORMAT_R16G16_SFLOAT;
			case ImageFormat::RG32F:				return VK_FORMAT_R32G32_SFLOAT;
			case ImageFormat::RGBA:					return Engine::GetInstance().GetWindow()->GetSwapChain()->GetColorFormat();
			case ImageFormat::RGBA16F:				return VK_FORMAT_R16G16B16A16_SFLOAT;
			case ImageFormat::RGBA32F:				return VK_FORMAT_R32G32B32A32_SFLOAT;
			case ImageFormat::B10R11G11UF:			return VK_FORMAT_B10G11R11_UFLOAT_PACK32;
			case ImageFormat::DEPTH32FSTENCIL8UINT: return VK_FORMAT_D32_SFLOAT_S8_UINT;
			case ImageFormat::DEPTH32F:				return VK_FORMAT_D32_SFLOAT;
			case ImageFormat::DEPTH24STENCIL8:		return RendererContext::GetDevice()->GetPhysicalDevice()->GetDepthFormat();
			case ImageFormat::DEPTH16UN:			return VK_FORMAT_D16_UNORM;
		}
		return VK_FORMAT_UNDEFINED;
	}
}