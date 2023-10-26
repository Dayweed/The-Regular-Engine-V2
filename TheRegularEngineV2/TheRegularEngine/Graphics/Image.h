#pragma once
#include "vulkan/vulkan.h"

namespace TRE
{
	class Image
	{
	public:
		Image() {};
		Image(const uint32_t width, const uint32_t height, const VkFormat format = VK_FORMAT_R8G8B8A8_SRGB, const VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, const VkImageAspectFlags viewAspectFlag = VK_IMAGE_ASPECT_COLOR_BIT, const VkImageType imageType = VK_IMAGE_TYPE_2D, const VkImageViewType imageViewType = VK_IMAGE_VIEW_TYPE_2D);
		Image(const VkImageCreateInfo imageInfo, const VkImageViewCreateInfo imageViewInfo);
		~Image();

		VkImage GetImage() const { return m_Image; }
		VkImageView GetImageView() const { return m_ImageView; }
		VkDeviceMemory GetMemory() const { return m_ImageMemory; }
	private:
		VkImage m_Image = VK_NULL_HANDLE;
		VkImageView m_ImageView = VK_NULL_HANDLE;
		VkDeviceMemory m_ImageMemory = VK_NULL_HANDLE;
	};

	enum class ImageFormat
	{
		None = 0,
		RED8UN,
		RED8UI,
		RED16UI,
		RED32UI,
		RED32F,
		RG8,
		RG16F,
		RG32F,
		RGB,
		RGBA,
		RGBA16F,
		RGBA32F,
		B10R11G11UF,
		SRGB,
		DEPTH32FSTENCIL8UINT,
		DEPTH32F,
		DEPTH24STENCIL8,
		Depth = DEPTH24STENCIL8,
	};

	enum class ImageUsage
	{
		None = 0,
		Texture,
		Attachment,
		Storage
	};

	struct ImageConfig
	{
		std::string DebugName;

		ImageFormat Format = ImageFormat::RGBA;
		ImageUsage Usage = ImageUsage::Texture;
		bool Transfer = false;
		uint32_t Width = 1;
		uint32_t Height = 1;
		uint32_t Mips = 1;
		uint32_t Layers = 1;
		bool CreateSampler = true;
	};

	struct ImageData
	{
		VkImage Image = VK_NULL_HANDLE;
		VkImageView ImageView = VK_NULL_HANDLE;
		VkSampler Sampler = VK_NULL_HANDLE;
		VkDeviceMemory ImageMemory = VK_NULL_HANDLE;
	};

	class Image2D
	{
		public:
			Image2D(const ImageConfig Config);
			~Image2D();

			void Invalidate();
			void UpdateDescriptorInfo();

			const VkDescriptorImageInfo& GetImageInfo();
			const ImageConfig& GetImageConfig() const;
			const ImageData& GetImageData();

		private:
			ImageConfig m_Config;
			ImageData m_ImageData;

			VkDescriptorImageInfo m_DescriptorImageInfo{};
	};
}