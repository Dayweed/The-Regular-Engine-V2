#include "pch.h"
#include "Image.h"
#include "Device.h"
#include "RendererContext.h"
#include "VulkanUtilities.h"

namespace TRE
{
	const ImageConfig& Image2D::GetImageConfig() const
	{
		return m_Config;
	}

	ImageData Image2D::GetImageData()
	{
		return m_ImageData;
	}

	VkDescriptorImageInfo Image2D::GetDescriptorImageInfo()
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
		ImageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		ImageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

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

		if (m_Config.CreateSampler)
		{
			VkSamplerCreateInfo samplerCreateInfo = {};
			samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
			samplerCreateInfo.maxAnisotropy = 1.0f;
			samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
			samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
			samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			samplerCreateInfo.addressModeV = samplerCreateInfo.addressModeU;
			samplerCreateInfo.addressModeW = samplerCreateInfo.addressModeU;
			samplerCreateInfo.mipLodBias = 0.0f;
			samplerCreateInfo.minLod = 0.0f;
			samplerCreateInfo.maxLod = 100.0f;
			samplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
			if (auto Result = vkCreateSampler(Device, &samplerCreateInfo, nullptr, &m_ImageData.Sampler); Result != VK_SUCCESS)
			{
				assert(Result == VK_SUCCESS && "Sampler cannot be created");
			}
		}

		UpdateDescriptorInfo();
	}

	void Image2D::UpdateDescriptorInfo()
	{
		m_DescriptorImageInfo.imageView = m_ImageData.ImageView;
		m_DescriptorImageInfo.sampler = m_ImageData.Sampler;
		
		if (m_Config.Format == ImageFormat::DEPTH24STENCIL8 || 
			m_Config.Format == ImageFormat::DEPTH32F || 
			m_Config.Format == ImageFormat::DEPTH32FSTENCIL8UINT || 
			m_Config.Format == ImageFormat::DEPTH16UN)
			m_DescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
		else
		{
			m_DescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		}
	}
}