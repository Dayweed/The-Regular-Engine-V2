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

		void UpdateMaterial(const std::shared_ptr<UniformBuffer>& UBO, uint32_t Index);
			
		void BindPipeline(VkCommandBuffer CommandBuffer);

		void BindDebugSphere(VkCommandBuffer CommandBuffer);
		void DrawDebugSphere(VkCommandBuffer CommandBuffer);

		void BindDebugAABB(VkCommandBuffer CommandBuffer);
		void DrawDebugAABB(VkCommandBuffer CommandBuffer);

		void BindDebugCapsule(VkCommandBuffer CommandBuffer);
		void DrawDebugCapsule(VkCommandBuffer CommandBuffer);

		void BindDebugCameraFrustum(VkCommandBuffer CommandBuffer);
		void DrawDebugCameraFrustum(VkCommandBuffer CommandBuffer);
	private:
		void CreateDebugSphere();
		void CreateDebugAABB();
		void CreateDebugCapsule();
		void CreateDebugCameraFrustum();
	public:
		const VkDescriptorSet& GetDescriptor(uint32_t index);
		VkPipelineLayout GetPipelineLayout();

	private:
		struct DebugType
		{
			std::unique_ptr<Buffer> m_VertexBuffer;
			std::unique_ptr<Buffer> m_IndexBuffer;
			uint32_t m_IndexCount;
		};
	private:
		std::shared_ptr<Material> m_DebugMaterialInstance;
		std::unique_ptr<Pipeline> m_DebugDrawPipeline;

		std::unique_ptr<DebugType> m_DebugSphere;
		std::unique_ptr<DebugType> m_DebugAABB;
		std::unique_ptr<DebugType> m_DebugCapsule;
		std::unique_ptr<DebugType> m_DebugCameraFrustum;

		std::shared_ptr<RenderPass> m_RenderPass;
	};
}