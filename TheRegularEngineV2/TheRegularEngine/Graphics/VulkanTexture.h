#pragma once
#include "Image.h"
#include "Texture.h"
#include "Resource/Resource.h"

namespace TRE
{
	class VulkanTexture;
	struct CubeMapConfig
	{
		VkFormat Format;
		uint32_t Width = 1;
		uint32_t Height = 1;
		VkSamplerAddressMode SamplerAddressMode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		VkFilter Filter = VK_FILTER_NEAREST;
		std::vector<std::shared_ptr<VulkanTexture>> Textures;
	};

	class VulkanTexture : public Resource
	{
	public:
		VulkanTexture();
		VulkanTexture(const CubeMapConfig& Config); //Only used to create cubemap for now
		VulkanTexture(const std::string& texturePath);
		~VulkanTexture();

		const VkDescriptorImageInfo& GetDescriptorImageInfo() const;

		const VkSampler& GetSampler() const { return m_Sampler; }
		const VkImage& GetImage() const { return m_Image; }
		const VkImageView& GetImageView() const { return m_ImageView; }
		const VkDeviceMemory& GetMemory() const { return m_ImageMemory; }
		const uint32_t& GetWidth() const { return m_Width; }
		const uint32_t& GetHeight() const { return m_Height; }
		void* GetBuffer() { return m_Buffer; }
		VkFormat GetFormat() { return m_Format; }

		static ResourceType GetType() { return ResourceType::Texture; }
		static std::shared_ptr<VulkanTexture> Deserialize(const std::string& assetHexGUID);

		static const ResourceHandle& GetDefaultTextureID();
	private:
		void TransitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout);
		void CopyBufferToImage(VkBuffer buffer, uint32_t width, uint32_t height, uint32_t layerCount = 1);

		VulkanTexture(VulkanTexture&) = delete;
		void operator=(const VulkanTexture&) = delete;
	private:
		VkSampler m_Sampler;
		VkImage m_Image;
		VkImageView m_ImageView;
		VkDeviceMemory m_ImageMemory;
		VkDescriptorImageInfo m_DescriptorImageInfo;

		uint32_t m_Width;
		uint32_t m_Height;
		VkFormat m_Format;

		void* m_Buffer = nullptr;

		static ResourceHandle m_DefaultTextureID;
	};
}