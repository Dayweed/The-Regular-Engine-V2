#pragma once
#include "Image.h"
#include "Texture.h"
#include "Assets/Asset.h"

namespace TRE
{
	class VulkanTexture : public Asset
	{
	public:
		VulkanTexture() = default;
		VulkanTexture(std::unique_ptr<Texture> texture);
		~VulkanTexture();

		const VkDescriptorImageInfo& GetDescriptorImageInfo() const;

		const VkSampler& GetSampler() const { return m_Sampler; }
		const VkImage& GetImage() const { return m_Image; }
		const VkImageView& GetImageView() const { return m_ImageView; }
		const VkDeviceMemory& GetMemory() const { return m_ImageMemory; }
	private:
		void TransitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout);
		void CopyBufferToImage(VkBuffer buffer, uint32_t width, uint32_t height, uint32_t layerCount = 1);
	private:
		VkSampler m_Sampler;
		VkImage m_Image;
		VkImageView m_ImageView;
		VkDeviceMemory m_ImageMemory;
	};
}