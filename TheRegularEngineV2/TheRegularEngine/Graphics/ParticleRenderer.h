#pragma once
#include "Pipeline.h"
#include "glm/glm.hpp"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "CommandBuffer.h"
#include "Material.h"

namespace TRE
{
	struct Particle_PushConstant
	{
		glm::mat4 L2W{ glm::mat4(1.f) };
	};

	struct ParticleUBO
	{
		glm::mat4 ProjView{ glm::mat4(1.f) };
		float Gamma{ 2.2f };
	};

	class ParticleRenderer
	{
		public:
			ParticleRenderer(const std::shared_ptr<Device>& device);

			void Render(std::shared_ptr<UniformBuffer> ubo2D, std::shared_ptr<UniformBuffer> ubo3D, const std::shared_ptr<CommandBuffer>& commandBuffer, bool isEditor);

			void Render3D(const std::shared_ptr<UniformBuffer>& UBO, const std::shared_ptr<CommandBuffer>& commandBuffer, bool isEditor);
		
		private:
			void Init2D();
			void Init3D();

		private:
			std::shared_ptr<Pipeline> m_3DPipeline;
			std::shared_ptr<Material> m_3DDefaultMaterial;

		private:
			std::shared_ptr<Pipeline> m_Pipeline;
			std::shared_ptr<RenderPass> m_Renderpass;
			std::shared_ptr<VertexBuffer> m_VertexBuffer;
			std::shared_ptr<IndexBuffer> m_IndexBuffer;

			std::shared_ptr<Material> m_DefaultMaterial;
			ResourceHandle m_PreviousMaterialHandle{0};
	};
}