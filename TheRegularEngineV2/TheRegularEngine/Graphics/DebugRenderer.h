#pragma once
#include "Material.h"
#include "Pipeline.h"
#include "Buffer.h"
#include "glm/glm.hpp"

namespace TRE
{
	struct DebugVertex
	{
		glm::vec3 Position;
		glm::vec4 Color;
	};

	class DebugRenderer
	{
		public:
			DebugRenderer(std::shared_ptr<RenderPass> TargetPass);
			~DebugRenderer();
			void CreateDebugSphere();
			void CreateDebugAABB();

			void UpdateMaterial(const std::shared_ptr<UniformBuffer>& UBO, uint32_t Index);
			
			void BindPipeline(VkCommandBuffer CommandBuffer);
			void BindDebugSphere(VkCommandBuffer CommandBuffer);
			void DrawDebugSphere(VkCommandBuffer CommandBuffer);

			void BindDebugAABB(VkCommandBuffer CommandBuffer);
			void DrawDebugAABB(VkCommandBuffer CommandBuffer);

		public:
			const VkDescriptorSet& GetDescriptor(uint32_t index);
			VkPipelineLayout GetPipelineLayout();

		private:
			std::shared_ptr<Material> m_DebugMaterialInstance;
			std::unique_ptr<Pipeline> m_DebugDrawPipeline;
			
			std::unique_ptr<Buffer> m_DebugSphereVertexBuffer;
			std::unique_ptr<Buffer> m_DebugSphereIndexBuffer;
			uint32_t m_SphereIndexCount;

			std::unique_ptr<Buffer> m_DebugAABBVertexBuffer;
			std::unique_ptr<Buffer> m_DebugAABBIndexBuffer;
			uint32_t m_AABBIndexCount;

			std::shared_ptr<RenderPass> m_RenderPass;
	};
}