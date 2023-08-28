#pragma once
#include "pch.h"

namespace TRE
{
	class Image
	{
	public:
		Image() {};
		Image(const uint32_t width, const uint32_t height, const VkFormat format = VK_FORMAT_R8G8B8A8_SRGB, const VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, const VkImageAspectFlags viewAspectFlag = VK_IMAGE_ASPECT_COLOR_BIT, const VkImageType imageType = VK_IMAGE_TYPE_2D, const VkImageViewType imageViewType = VK_IMAGE_VIEW_TYPE_2D);
		Image(const VkImageCreateInfo imageInfo, const VkImageViewCreateInfo imageViewInfo);
		~Image();

		Image(const Image&) = delete;
		Image& operator=(const Image&) = delete;

		VkImage GetImage() const { return m_Image; }
		VkImageView GetImageView() const { return m_ImageView; }
		VkDeviceMemory GetMemory() const { return m_ImageMemory; }
	private:
		VkImage m_Image = nullptr;
		VkImageView m_ImageView = nullptr;
		VkDeviceMemory m_ImageMemory = nullptr;
	};
}