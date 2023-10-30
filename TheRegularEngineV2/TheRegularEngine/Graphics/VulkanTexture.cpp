#include "pch.h"
#include "Device.h"
#include "VulkanTexture.h"
#include "Buffer.h"
#include "RendererContext.h"
#include "TREIncludes.h"

TRE::ResourceHandle TRE::VulkanTexture::m_DefaultTextureID{ 0 };

namespace TRE
{
	VulkanTexture::VulkanTexture(const CubeMapConfig& Config)
	{
		assert(Config.Textures.size() == 6 && "Cubemap textures not more than 0");

		auto Device = RendererContext::GetDevice();

		VkDeviceSize ImageCubeMapSize = Config.Textures[0]->GetWidth() * Config.Textures[0]->GetHeight() * 4 * 6;
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingMemory;

		VkBufferCreateInfo bufferCreateInfo{}; // This buffer is used as a transfer source for the buffer copy
		bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		bufferCreateInfo.size = ImageCubeMapSize;

		if (auto Result = vkCreateBuffer(Device->GetLogicalDevice(), &bufferCreateInfo, nullptr, &stagingBuffer); Result != VK_SUCCESS)
		{
			assert(Result == VK_SUCCESS && "Unable to create staging buffer for cubemap");
		}

		VkMemoryRequirements memReqs;
		vkGetBufferMemoryRequirements(Device->GetLogicalDevice(), stagingBuffer, &memReqs);

		VkMemoryAllocateInfo memAllocInfo{};
		memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memAllocInfo.allocationSize;
		memAllocInfo.allocationSize = memReqs.size;
		memAllocInfo.memoryTypeIndex = Device->FindMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		if (auto Result = vkAllocateMemory(Device->GetLogicalDevice(), &memAllocInfo, nullptr, &stagingMemory); Result != VK_SUCCESS)
		{
			assert(Result == VK_SUCCESS && "Unable to allocate memory for cubemap");
		}

		if (auto Result = vkBindBufferMemory(Device->GetLogicalDevice(), stagingBuffer, stagingMemory, 0); Result != VK_SUCCESS)
		{
			assert(Result == VK_SUCCESS && "Unable to bind memory for cubemap");
		}

		uint8_t* data;
		if (auto Result = vkMapMemory(Device->GetLogicalDevice(), stagingMemory, 0, memReqs.size, 0, (void**)&data); Result != VK_SUCCESS)
		{
			assert(Result == VK_SUCCESS && "Unable to map memory for cubemap");
		}

		VkDeviceSize layersize = ImageCubeMapSize / 6;
		for (uint32_t x = 0; x < 6; x++)
		{
			memcpy(data + layersize * x, Config.Textures[x]->GetBuffer(), layersize);
		}

		VkImageCreateInfo ImageCreateInfo{};
		ImageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		ImageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
		ImageCreateInfo.format = Config.Format;
		ImageCreateInfo.mipLevels = 1;
		ImageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		ImageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		ImageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		ImageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		ImageCreateInfo.extent = { Config.Textures[0]->GetWidth(), Config.Textures[0]->GetHeight(), 1 };
		ImageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		ImageCreateInfo.arrayLayers = 6;
		ImageCreateInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

		if (auto Result = vkCreateImage(Device->GetLogicalDevice(), &ImageCreateInfo, nullptr, &m_Image); Result != VK_SUCCESS)
		{
			assert(Result == VK_SUCCESS && "Unable to create image for cubemap");
		}

		// VkMemoryRequirements ImagememReqs;
		VkMemoryAllocateInfo ImagememAlloc{};

		ImagememAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		vkGetImageMemoryRequirements(Device->GetLogicalDevice(), m_Image, &memReqs);

		ImagememAlloc.allocationSize = memReqs.size;
		ImagememAlloc.memoryTypeIndex = RendererContext::GetDevice()->FindMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		if (auto result = vkAllocateMemory(Device->GetLogicalDevice(), &ImagememAlloc, nullptr, &m_ImageMemory); result != VK_SUCCESS)
		{
			assert(result == VK_SUCCESS && "Failed to allocate memory for image");
		}
		if (auto result = vkBindImageMemory(Device->GetLogicalDevice(), m_Image, m_ImageMemory, 0); result != VK_SUCCESS)
		{
			assert(result == VK_SUCCESS && "Failed to bind image memory");
		}

		VkImageViewCreateInfo ImageViewCreateInfo{};
		ImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		ImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
		ImageViewCreateInfo.format = Config.Format;
		ImageViewCreateInfo.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
		ImageViewCreateInfo.subresourceRange.layerCount = 6;
		ImageViewCreateInfo.image = m_Image;
		ImageViewCreateInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };

		if (auto Result = vkCreateImageView(Device->GetLogicalDevice(), &ImageViewCreateInfo, nullptr, &m_ImageView); Result != VK_SUCCESS)
		{
			assert(Result == VK_SUCCESS && "Unable to create cubemap image view");
		}

		auto cmd = Device->AllocateCommandBuffer(true);

		VkImageSubresourceRange range;
		range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		range.baseMipLevel = 0;
		range.levelCount = 1;
		range.baseArrayLayer = 0;
		range.layerCount = 6;

		std::vector<VkBufferImageCopy> bufferCopyRegions;
		for (int x = 0; x < 6; x++)
		{
			VkBufferImageCopy bufferCopyRegion = {};
			bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			bufferCopyRegion.imageSubresource.mipLevel = 0;
			bufferCopyRegion.imageSubresource.baseArrayLayer = x;
			bufferCopyRegion.imageSubresource.layerCount = 1;
			bufferCopyRegion.imageExtent.width = Config.Textures[0]->GetWidth();
			bufferCopyRegion.imageExtent.height = Config.Textures[0]->GetHeight();
			bufferCopyRegion.imageExtent.depth = 1;
			bufferCopyRegion.bufferOffset = layersize * x;
			bufferCopyRegions.push_back(bufferCopyRegion);
		}

		VkImageMemoryBarrier imageBarrier_toTransfer{};
		imageBarrier_toTransfer.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		imageBarrier_toTransfer.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageBarrier_toTransfer.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		imageBarrier_toTransfer.image = m_Image;
		imageBarrier_toTransfer.subresourceRange = range;

		imageBarrier_toTransfer.srcAccessMask = 0;
		imageBarrier_toTransfer.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageBarrier_toTransfer);

		vkCmdCopyBufferToImage(cmd, stagingBuffer, m_Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, static_cast<uint32_t>(bufferCopyRegions.size()), bufferCopyRegions.data());

		VkImageMemoryBarrier imageBarrier_toReadable = imageBarrier_toTransfer;
		imageBarrier_toReadable.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		imageBarrier_toReadable.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageBarrier_toReadable.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		imageBarrier_toReadable.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageBarrier_toReadable);

		Device->SubmitCommands(cmd);

		vkUnmapMemory(Device->GetLogicalDevice(), stagingMemory);

		VkSamplerCreateInfo samplerCreateInfo = {};
		samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerCreateInfo.maxAnisotropy = 1.0f;
		samplerCreateInfo.magFilter = VK_FILTER_NEAREST;
		samplerCreateInfo.minFilter = VK_FILTER_NEAREST;
		samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerCreateInfo.addressModeV = samplerCreateInfo.addressModeU;
		samplerCreateInfo.addressModeW = samplerCreateInfo.addressModeU;
		samplerCreateInfo.mipLodBias = 0.0f;
		samplerCreateInfo.minLod = 0.0f;
		samplerCreateInfo.maxLod = 100.0f;
		samplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
		if (auto Result = vkCreateSampler(Device->GetLogicalDevice(), &samplerCreateInfo, nullptr, &m_Sampler); Result != VK_SUCCESS)
		{
			assert(Result == VK_SUCCESS && "Sampler cannot be created");
		}

		m_DescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		m_DescriptorImageInfo.imageView= m_ImageView;
		m_DescriptorImageInfo.sampler = m_Sampler;

		vkDestroyBuffer(Device->GetLogicalDevice(), stagingBuffer, nullptr);
		vkFreeMemory(Device->GetLogicalDevice(), stagingMemory, nullptr);
	}

	VulkanTexture::VulkanTexture(const std::string& texturePath)
	{
		std::unique_ptr<Texture> texture = Texture::Deserialize(texturePath);

		m_Type = ResourceType::Texture;

		Buffer stagingBuffer(texture->DataSize, 1, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 4);

		stagingBuffer.Map();
		stagingBuffer.WriteToBuffer(texture->Data, texture->DataSize);
		stagingBuffer.Unmap();

		m_Width = texture->Width;
		m_Height = texture->Height;
		m_Format = VkFormat(texture->Format);
		m_Buffer = new void* [texture->DataSize];
		memcpy(m_Buffer, texture->Data, texture->DataSize);

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

		m_DescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		m_DescriptorImageInfo.imageView = m_ImageView;
		m_DescriptorImageInfo.sampler = m_Sampler;
	}

	VulkanTexture::~VulkanTexture()
	{
		auto device = RendererContext::GetDevice()->GetLogicalDevice();
		vkDeviceWaitIdle(device); //Temp fix (by right shldnt need wait no?)
		vkDestroySampler(device, m_Sampler, nullptr);
		vkDestroyImageView(device, m_ImageView, nullptr);
		vkDestroyImage(device, m_Image, nullptr);
		vkFreeMemory(device, m_ImageMemory, nullptr);
	}

	const VkDescriptorImageInfo& VulkanTexture::GetDescriptorImageInfo() const
	{
		return m_DescriptorImageInfo;
	}

	VulkanTexture::VulkanTexture()
	{
		//GenerateDefaultTexture();
		m_Type = ResourceType::Texture;

		const std::uint32_t width = 4;
		const std::uint32_t height = 4;
		// Create a default texture 4x4 white RGBA
		VkDeviceSize imageSize = width * height * 4;

		Buffer stagingBuffer(imageSize, 1, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 4);

		std::unique_ptr<std::uint8_t[]> data = std::make_unique<std::uint8_t[]>(imageSize);
		memset(data.get(), 255, imageSize);

		stagingBuffer.Map();
		stagingBuffer.WriteToBuffer(data.get(), imageSize);
		stagingBuffer.Unmap();

		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
		imageInfo.extent.width = width;
		imageInfo.extent.height = height;
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
		CopyBufferToImage(stagingBuffer.GetBuffer(), width, height);
		TransitionImageLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = m_Image;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
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
		samplerInfo.magFilter = VK_FILTER_NEAREST;
		samplerInfo.minFilter = VK_FILTER_NEAREST;
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

		m_DescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		m_DescriptorImageInfo.imageView = m_ImageView;
		m_DescriptorImageInfo.sampler = m_Sampler;
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

	std::shared_ptr<VulkanTexture> VulkanTexture::Deserialize(const std::string& assetHexGUID)
	{
		std::string textureString = "../Resources/" + assetHexGUID + ".DDS";
		std::unique_ptr<VulkanTexture> ro = std::make_unique<VulkanTexture>(textureString);
		ResourceHandle assetHandle = Resource::GetGUIDFromHex(assetHexGUID);
		ro->m_Handle = assetHandle;
		ResourceManager::Instance().AddResource(std::move(ro));

		return std::move(ResourceManager::Instance().GetResource<VulkanTexture>(assetHandle));
	}

	const ResourceHandle& VulkanTexture::GetDefaultTextureID()
	{
		if (m_DefaultTextureID == 0)
		{
			m_DefaultTextureID = Resource::GenerateGUID();
			std::unique_ptr<VulkanTexture> defaultTexture = std::make_unique<VulkanTexture>();
			defaultTexture->SetHandle(m_DefaultTextureID);
			ResourceManager::Instance().AddResource(std::move(defaultTexture));
		}

		return m_DefaultTextureID;
	}
}