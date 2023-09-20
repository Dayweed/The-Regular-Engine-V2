#pragma once
#include "Image.h"
#include "Texture.h"
#include "Resource/Resource.h"

namespace TRE
{
	class VulkanTexture : public Resource
	{
	public:
		VulkanTexture(const std::string& texturePath);
		~VulkanTexture();

		const VkDescriptorImageInfo& GetDescriptorImageInfo() const;

		const VkSampler& GetSampler() const { return m_Sampler; }
		const VkImage& GetImage() const { return m_Image; }
		const VkImageView& GetImageView() const { return m_ImageView; }
		const VkDeviceMemory& GetMemory() const { return m_ImageMemory; }

		static ResourceType GetType() { return ResourceType::Texture; }
		static std::shared_ptr<VulkanTexture> Deserialize(const std::string& assetHexGUID);

	private:
		void GenerateDefaultTexture();
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

		static ResourceHandle m_DefaultTextureID;
	};
}