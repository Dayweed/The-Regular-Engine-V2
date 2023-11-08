#pragma once
#include "Device.h"
#include "UniformBuffer.h"
#include "Pipeline.h"
#include "RenderPass.h"
#include "Material.h"
#include "CommandBuffer.h"

namespace TRE
{
	struct UIUBO
	{
		glm::mat4 m_ProjView2DSpace;
	};

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
			std::shared_ptr<UniformBuffer> m_UIUBO;
	};
}