#pragma once
#include "Device.h"

namespace TRE
{
	struct RenderPassInfo
	{
		VkFormat ImageFormat;
		VkImageLayout FinalLayout;
		VkFormat DepthImageFormat;
		VkImageLayout DepthFinalLayout;
		bool DepthEnabled = false;
	};

	class RenderPass
	{
		public:
			RenderPass(std::shared_ptr<Device> Device, RenderPassInfo Info);
			RenderPass(std::shared_ptr<Device> Device, bool isshadow);
			~RenderPass();

			void Recreate();

			void BeginRenderPass(VkCommandBuffer cmdBuffer, VkFramebuffer FrameBuffer);
			void EndRenderPass(VkCommandBuffer cmdBuffer);

			VkRenderPass GetHandle();
			RenderPassInfo GetInfo();

		private:
			VkRenderPass m_Renderpass = VK_NULL_HANDLE;
			RenderPassInfo m_Info;
			std::shared_ptr<Device> m_Device;
	};
}