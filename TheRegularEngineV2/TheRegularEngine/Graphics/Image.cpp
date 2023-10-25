#include "pch.h"
#include "Image.h"
#include "Device.h"
#include "RendererContext.h"

namespace TRE
{
	Image::Image(const uint32_t width, const uint32_t height, const VkFormat format, const VkImageUsageFlags usage, const VkImageAspectFlags viewAspectFlag, const VkImageType imageType, const VkImageViewType imageViewType)
	{
		auto device = RendererContext::GetDevice()->GetLogicalDevice();
		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = imageType;
		imageInfo.format = format;
		imageInfo.extent.width = width;
		imageInfo.extent.height = height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.usage = usage;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.flags = 0;
		if (auto result = vkCreateImage(device, &imageInfo, nullptr, &m_Image); result != VK_SUCCESS)
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

	Image::Image(const VkImageCreateInfo imageInfo, const VkImageViewCreateInfo imageViewInfo)
	{
		auto device = RendererContext::GetDevice()->GetLogicalDevice();

		if (auto result = vkCreateImage(device, &imageInfo, nullptr, &m_Image); result != VK_SUCCESS)
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

		if (auto result = vkCreateImageView(device, &imageViewInfo, nullptr, &m_ImageView); result != VK_SUCCESS)
		{
			assert(result == VK_SUCCESS && "Failed to bind image memory");
		}
	}

	Image::~Image()
	{
		auto device = RendererContext::GetDevice()->GetLogicalDevice();
		vkDestroyImage(device, m_Image, nullptr);
		vkDestroyImageView(device, m_ImageView, nullptr);
		vkFreeMemory(device, m_ImageMemory, nullptr);
	}


	uint32_t Image2D::GetWidth() const
	{
		return m_Config.Width;
	}

	uint32_t Image2D::GetHeight() const
	{
		return m_Config.Height;
	}

	const ImageConfig& Image2D::GetImageConfig() const
	{
		return m_Config;
	}

	Image2D::Image2D(const ImageConfig& Config) : m_Config(Config)
	{

	}

	Image2D::~Image2D()
	{
		if (m_Image == nullptr) 
			return;

		auto Device = RendererContext::GetDevice()->GetLogicalDevice();

		vkDestroyImageView(Device, m_ImageView, nullptr);
		vkDestroySampler(Device, m_Sampler, nullptr);
		vkDestroyImage(Device, m_Image, nullptr);
	}

	void Image2D::Invalidate()
	{
		// VkDevice Device = RendererContext::GetDevice()->GetLogicalDevice();

		// VkImageUsageFlags UsageFlag = VK_IMAGE_USAGE_SAMPLED_BIT;
	}
}