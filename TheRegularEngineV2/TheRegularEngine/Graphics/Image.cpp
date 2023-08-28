#include "pch.h"
#include "Image.h"
#include "Device.h"
#include "RendererContext.h"

namespace TRE
{
	Image::Image(const uint32_t width, const uint32_t height, const VkFormat format, const VkImageUsageFlags usage, const VkImageAspectFlags viewAspectFlag, const VkImageType imageType, const VkImageViewType imageViewType)
	{
		auto device = RendererContext::GetDevice()->GetLogicalDevice();
		VkImageCreateInfo image{};
		image.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		image.imageType = imageType;
		image.format = format;
		image.extent.width = width;
		image.extent.height = height;
		image.extent.depth = 1;
		image.mipLevels = 1;
		image.arrayLayers = 1;
		image.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		image.samples = VK_SAMPLE_COUNT_1_BIT;
		image.tiling = VK_IMAGE_TILING_OPTIMAL;
		image.usage = usage;
		image.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		image.flags = 0;
		if (auto result = vkCreateImage(device, &image, nullptr, &m_Image); result != VK_SUCCESS)
		{
			assert(result == VK_SUCCESS && "Failed to create image");
		}

		VkMemoryRequirements memReqs;
		VkMemoryAllocateInfo memAlloc{};
		memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		vkGetImageMemoryRequirements(device, m_Image, &memReqs);
		memAlloc.allocationSize = memReqs.size;
		memAlloc.memoryTypeIndex = RendererContext::GetDevice()->FindMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		if (auto result = vkAllocateMemory(device, &memAlloc, nullptr, &m_ImageMemory); result != VK_SUCCESS)
		{
			assert(result == VK_SUCCESS && "Failed to allocate memory for image");
		}
		if (auto result = vkBindImageMemory(device, m_Image, m_ImageMemory, 0); result != VK_SUCCESS)
		{
			assert(result == VK_SUCCESS && "Failed to bind image memory");
		}

		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = m_Image;
		viewInfo.viewType = imageViewType;
		viewInfo.format = format;
		viewInfo.subresourceRange.aspectMask = viewAspectFlag;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		if (auto result = vkCreateImageView(device, &viewInfo, nullptr, &m_ImageView); result != VK_SUCCESS)
		{
			assert(result == VK_SUCCESS && "Failed to bind image memory");
		}
	}

	Image::~Image()
	{
		auto device = RendererContext::GetDevice()->GetLogicalDevice();
		vkDestroyImageView(device, m_ImageView, nullptr);
		vkDestroyImage(device, m_Image, nullptr);
		vkFreeMemory(device, m_ImageMemory, nullptr);
	}
}