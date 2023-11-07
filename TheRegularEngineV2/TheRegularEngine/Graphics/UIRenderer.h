#pragma once
#include "Device.h"

namespace TRE
{
	class UIRenderer
	{
		public:
			UIRenderer(const std::shared_ptr<Device>& Device);
			~UIRenderer();

			void Render(VkFramebuffer TargetFramebuffer, const std::shared_ptr<CommandBuffer>& CommandBuffer);

		private:
			std::shared_ptr<Device> m_Device;
			std::shared_ptr<Pipeline> m_UIPipeline;
			std::shared_ptr<RenderPass> m_UIRenderpass;
			std::shared_ptr<Material> m_UIMaterial;
	};
}