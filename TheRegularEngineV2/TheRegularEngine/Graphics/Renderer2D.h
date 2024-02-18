#pragma once
#include "Device.h"
#include "UniformBuffer.h"
#include "Pipeline.h"
#include "RenderPass.h"
#include "Material.h"
#include "CommandBuffer.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"

namespace TRE
{
	// This should work for any things related to 2D, no more multiple PC struct for different 2D stuff.
	struct Render2D_PC
	{
		glm::mat4 L2W;
		glm::vec4 Color;
	};

	class Renderer2D
	{
		public:
			Renderer2D(const std::shared_ptr<Device>& Device);
			~Renderer2D();

			void Render(VkFramebuffer TargetFramebuffer, const std::shared_ptr<CommandBuffer>& CommandBuffer);

		private:
			std::shared_ptr<Device> m_Device;

			std::shared_ptr<Pipeline> m_Render2DPipeline;
			std::shared_ptr<RenderPass> m_Render2DPass;
	};
}