#pragma once
#include "Pipeline.h"
#include "CommandBuffer.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "Pipeline.h"
#include "RenderPass.h"
#include "VulkanTexture.h"
#include "Material.h"
#include "RendererContext.h"

namespace TRE
{
	class Device;

	struct Font_PushConstant
	{
		glm::mat4 Proj;
		glm::vec4 Color;
	};

	class FontRenderer
	{
		public:
			FontRenderer(const std::shared_ptr<Device>& Device);
			~FontRenderer();

			void RenderFont(VkFramebuffer TargetFramebuffer, const std::shared_ptr<CommandBuffer>& CommandBuffer);

		private:
			std::shared_ptr<Device> m_Device;
			std::shared_ptr<RenderPass> m_FontRenderPass;
			std::shared_ptr<Pipeline> m_FontPipeline;
			std::shared_ptr<IndexBuffer> m_FontIndexBuffer;
	};
}