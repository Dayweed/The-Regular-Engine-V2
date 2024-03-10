#pragma once
#include "vulkan/vulkan.h"

namespace TRE
{
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
		DEPTH16UN,
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
		VkSamplerAddressMode AddressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT;
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

			VkDescriptorImageInfo& GetDescriptorImageInfo();
			const ImageConfig& GetImageConfig() const;
			ImageData GetImageData();

		private:
			ImageConfig m_Config;
			ImageData m_ImageData;

			VkDescriptorImageInfo m_DescriptorImageInfo{};
	};
}