#pragma once
#include "Image.h"
#include "Texture.h"

namespace TRE
{
	class VulkanTexture
	{
	public:
		VulkanTexture() = default;
		VulkanTexture(const uint32_t texWidth, const uint32_t texHeight, void* pixels,
			const VkFormat imageFormat = VK_FORMAT_R8G8B8A8_SRGB, VkFilter imageFilter = VK_FILTER_NEAREST);
		VulkanTexture(std::unique_ptr<Texture> texture);
		~VulkanTexture();

		VkDescriptorImageInfo GetDescriptorImageInfo() const;

		VkSampler GetSampler() const { return m_Sampler; }
		VkImage GetImage() const { return m_Image; }
		VkImageView GetImageView() const { return m_ImageView; }
		VkDeviceMemory GetMemory() const { return m_ImageMemory; }
	private:
		void TransitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout);
		void CopyBufferToImage(VkBuffer buffer, uint32_t width, uint32_t height, uint32_t layerCount = 1);
	private:
		VkSampler m_Sampler;
		VkImage m_Image;
		VkImageView m_ImageView;
		VkDeviceMemory m_ImageMemory;
	};

	class TextureManager
	{
	public:
		static TextureManager& Instance()
		{
			static TextureManager instance;
			return instance;
		}

		void LoadTexture(const std::string& path, const std::string& name, const VkFormat imageFormat = VK_FORMAT_R8G8B8A8_SRGB, const VkFilter imageFilter = VK_FILTER_NEAREST);
		void LoadTexture(std::unique_ptr<Texture> texture);
		std::shared_ptr<VulkanTexture> GetTexture(const std::string& name);
		
		void Shutdown();
	
	private:
		TextureManager() {};
		TextureManager(TextureManager const&) = delete;
		void operator=(TextureManager const&) = delete;
		void* operator new(size_t) = delete;
	private:
		std::unordered_map<std::string, std::shared_ptr<VulkanTexture>> m_Textures;
	};
}