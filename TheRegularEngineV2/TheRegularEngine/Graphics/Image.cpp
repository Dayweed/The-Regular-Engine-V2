#include "pch.h"
#include "Image.h"
#include "Device.h"
#include "RendererContext.h"
#include "VulkanUtilities.h"

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

	const ImageConfig& Image2D::GetImageConfig() const
	{
		return m_Config;
	}

	const ImageData& Image2D::GetImageData()
	{
		return m_ImageData;
	}

	const VkDescriptorImageInfo& Image2D::GetImageInfo()
	{
		return m_DescriptorImageInfo;
	}

	Image2D::Image2D(const ImageConfig Config) : m_Config(Config)
	{
		Invalidate();
	}

	Image2D::~Image2D()
	{
		if (m_ImageData.Image == VK_NULL_HANDLE) 
			return;

		auto Device = RendererContext::GetDevice()->GetLogicalDevice();

		vkDestroyImageView(Device, m_ImageData.ImageView, nullptr);
		vkDestroySampler(Device, m_ImageData.Sampler, nullptr);
		vkDestroyImage(Device, m_ImageData.Image, nullptr);
		vkFreeMemory(Device, m_ImageData.ImageMemory, nullptr);
	}

	void Image2D::Invalidate()
	{
		VkDevice Device = RendererContext::GetDevice()->GetLogicalDevice();
		VkImageUsageFlags UsageFlag = VK_IMAGE_USAGE_SAMPLED_BIT;

		if (m_Config.Usage == ImageUsage::Attachment)
		{
			if (vkUtils::IsDepthImage(m_Config.Format))
				UsageFlag |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
			else
				UsageFlag |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		}

		if ((m_Config.Usage == ImageUsage::Texture) && m_Config.Transfer)
		{
			UsageFlag |= VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		}

		VkImageAspectFlags AspectFlags = vkUtils::IsDepthImage(m_Config.Format) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
		
		VkFormat Format = vkUtils::VulkanImageFormat(m_Config.Format);

		VkImageCreateInfo ImageCreateInfo{};
		ImageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		ImageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
		ImageCreateInfo.format = Format;
		ImageCreateInfo.extent.width = m_Config.Width;
		ImageCreateInfo.extent.height = m_Config.Height;
		ImageCreateInfo.extent.depth = 1;
		ImageCreateInfo.mipLevels = m_Config.Mips;
		ImageCreateInfo.arrayLayers = m_Config.Layers;
		ImageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		ImageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		ImageCreateInfo.usage = UsageFlag;

		if (auto Result = vkCreateImage(Device, &ImageCreateInfo, nullptr, &m_ImageData.Image); Result != VK_SUCCESS)
		{
			assert(Result == VK_SUCCESS && "Image cannot be created");
		}

		VkMemoryRequirements memReqs;
		VkMemoryAllocateInfo memAlloc{};
		memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		vkGetImageMemoryRequirements(Device, m_ImageData.Image, &memReqs);
		memAlloc.allocationSize = memReqs.size;
		memAlloc.memoryTypeIndex = RendererContext::GetDevice()->FindMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		if (auto result = vkAllocateMemory(Device, &memAlloc, nullptr, &m_ImageData.ImageMemory); result != VK_SUCCESS)
		{
			assert(result == VK_SUCCESS && "Failed to allocate memory for image");
		}
		if (auto result = vkBindImageMemory(Device, m_ImageData.Image, m_ImageData.ImageMemory, 0); result != VK_SUCCESS)
		{
			assert(result == VK_SUCCESS && "Failed to bind image memory");
		}

		VkImageViewCreateInfo ImageViewInfo{};
		ImageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		ImageViewInfo.image = m_ImageData.Image;
		ImageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		ImageViewInfo.format = Format;
		ImageViewInfo.subresourceRange.aspectMask = AspectFlags;
		ImageViewInfo.subresourceRange.baseMipLevel = 0;
		ImageViewInfo.subresourceRange.levelCount = 1;
		ImageViewInfo.subresourceRange.baseArrayLayer = 0;
		ImageViewInfo.subresourceRange.layerCount = 1;

		if (auto result = vkCreateImageView(Device, &ImageViewInfo, nullptr, &m_ImageData.ImageView); result != VK_SUCCESS)
		{
			assert(result == VK_SUCCESS && "Failed to bind image memory");
		}
	}

	void Image2D::UpdateDescriptorInfo()
	{
		m_DescriptorImageInfo.imageView = m_ImageData.ImageView;
		m_DescriptorImageInfo.sampler = m_ImageData.Sampler;
		
		if (m_Config.Format == ImageFormat::DEPTH24STENCIL8 || m_Config.Format == ImageFormat::DEPTH32F || m_Config.Format == ImageFormat::DEPTH32FSTENCIL8UINT)
			m_DescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
		else
			m_DescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	}
}