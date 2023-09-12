#include "pch.h"
#include "Device.h"
#include "VulkanTexture.h"
#include "Buffer.h"
#include "RendererContext.h"
#include "TREIncludes.h"

//#include "crnlib.h"
//#include "crn_decomp.h"
//#include "dds_defs.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace TRE
{
	VulkanTexture::VulkanTexture(const uint32_t texWidth, const uint32_t texHeight, void* pixels, const VkFormat imageFormat, VkFilter imageFilter)
	{
		VkDeviceSize imageSize = static_cast<uint64_t>(texWidth) * texHeight * 4;
		Buffer stagingBuffer(imageSize, 1, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		stagingBuffer.Map();
		stagingBuffer.WriteToBuffer(pixels);
		stagingBuffer.Unmap();

		//stbi_image_free(pixels);

		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = texWidth;
		imageInfo.extent.height = texHeight;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.format = imageFormat;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.flags = 0;

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

		TransitionImageLayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		CopyBufferToImage(stagingBuffer.GetBuffer(), texWidth, texHeight);
		TransitionImageLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = m_Image;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = imageFormat;
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;
		viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		if (auto result = vkCreateImageView(device, &viewInfo, nullptr, &m_ImageView); result != VK_SUCCESS)
		{
			assert(result == VK_SUCCESS && "Failed to bind image memory");
		}

		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = imageFilter;
		samplerInfo.minFilter = imageFilter;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.anisotropyEnable = VK_TRUE;

		VkPhysicalDeviceProperties properties{};
		vkGetPhysicalDeviceProperties(RendererContext::GetDevice()->GetPhysicalDevice()->GetPhysicalDevice(), &properties);
		samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 0.0f;

		if (auto result = vkCreateSampler(device, &samplerInfo, nullptr, &m_Sampler); result != VK_SUCCESS)
		{
			assert(result == VK_SUCCESS && "Failed to create texture sampler");
		}
	}

	VulkanTexture::VulkanTexture(std::unique_ptr<Texture> texture)
	{
		VkDeviceSize imageSize = texture->DataSize;
		Buffer stagingBuffer(imageSize, 1, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		stagingBuffer.Map();
		stagingBuffer.WriteToBuffer(texture->Data);
		stagingBuffer.Unmap();

		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.format = VkFormat(texture->Format);
		imageInfo.extent.width = texture->Width;
		imageInfo.extent.height = texture->Height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.flags = 0;

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

		TransitionImageLayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		CopyBufferToImage(stagingBuffer.GetBuffer(), texture->Width, texture->Height);
		TransitionImageLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = m_Image;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = VkFormat(texture->Format);
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;
		viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		if (auto result = vkCreateImageView(device, &viewInfo, nullptr, &m_ImageView); result != VK_SUCCESS)
		{
			assert(result == VK_SUCCESS && "Failed to bind image memory");
		}

		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VkFilter(texture->Filter);
		samplerInfo.minFilter = VkFilter(texture->Filter);
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.anisotropyEnable = VK_TRUE;

		VkPhysicalDeviceProperties properties{};
		vkGetPhysicalDeviceProperties(RendererContext::GetDevice()->GetPhysicalDevice()->GetPhysicalDevice(), &properties);
		samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 0.0f;

		if (auto result = vkCreateSampler(device, &samplerInfo, nullptr, &m_Sampler); result != VK_SUCCESS)
		{
			assert(result == VK_SUCCESS && "Failed to create texture sampler");
		}
	}

	VulkanTexture::~VulkanTexture()
	{
		auto device = RendererContext::GetDevice()->GetLogicalDevice();
		vkDestroySampler(device, m_Sampler, nullptr);
		vkDestroyImageView(device, m_ImageView, nullptr);
		vkDestroyImage(device, m_Image, nullptr);
		vkFreeMemory(device, m_ImageMemory, nullptr);
	}

	VkDescriptorImageInfo VulkanTexture::GetDescriptorImageInfo() const
	{
		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = m_ImageView;
		imageInfo.sampler = m_Sampler;
		return imageInfo;
	}

	void VulkanTexture::TransitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout)
	{
		auto device = RendererContext::GetDevice();
		VkCommandBuffer commandBuffer = device->AllocateCommandBuffer(true);

		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = m_Image;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		VkPipelineStageFlags sourceStage;
		VkPipelineStageFlags destinationStage;

		if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) 
		{
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) 
		{
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else 
		{
			throw std::invalid_argument("unsupported layout transition!");
		}

		vkCmdPipelineBarrier(
			commandBuffer,
			sourceStage, destinationStage,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);

		device->SubmitCommands(commandBuffer);
	}

	void VulkanTexture::CopyBufferToImage(VkBuffer buffer, uint32_t width, uint32_t height, uint32_t layerCount)
	{
		auto device = RendererContext::GetDevice();
		VkCommandBuffer commandBuffer = device->AllocateCommandBuffer(true);

		VkBufferImageCopy region{};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;

		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = layerCount;

		region.imageOffset = { 0, 0, 0 };
		region.imageExtent = { width, height, 1 };

		vkCmdCopyBufferToImage(
			commandBuffer,
			buffer,
			m_Image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1,
			&region);

		device->SubmitCommands(commandBuffer);
	}

	void TextureManager::LoadTexture(const std::string& path, const std::string& name, const VkFormat imageFormat, const VkFilter imageFilter)
	{
		int texWidth, texHeight, texChannels;
		stbi_uc* pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

		if (!pixels)
		{
			throw std::runtime_error("Failed to load texture image");
		}

		m_Textures[name] = std::make_shared<VulkanTexture>(texWidth, texHeight, reinterpret_cast<void*>(pixels), imageFormat, imageFilter);
	}

	void TextureManager::LoadTexture(std::unique_ptr<Texture> texture)
	{
		std::string name = std::string(texture->Name.data(), texture->Name.size());
		name = name.substr(0, name.find_first_of('\0'));
 		m_Textures[name] = std::make_shared<VulkanTexture>(std::move(texture));
	}

	std::shared_ptr<VulkanTexture> TextureManager::GetTexture(const std::string& name)
	{
		return m_Textures[name];
	}

	void TextureManager::Shutdown()
	{	
		m_Textures.clear();
	}
}