#pragma once
#include "Material.h"
#include "Pipeline.h"
#include "glm/glm.hpp"
#include "VertexBuffer.h"
#include "IndexBuffer.h"

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

		void BindDebugCapsuleRadius(VkCommandBuffer CommandBuffer);
		void DrawDebugCapsuleRadius(VkCommandBuffer CommandBuffer);

		void BindDebugCapsuleHalfExtent(VkCommandBuffer CommandBuffer);
		void DrawDebugCapsuleHalfExtent(VkCommandBuffer CommandBuffer);

		void BindDebugCameraFrustum(VkCommandBuffer CommandBuffer);
		void DrawDebugCameraFrustum(VkCommandBuffer CommandBuffer);

		void BindDebugDirectionalLight(VkCommandBuffer CommandBuffer);
		void DrawDebugDirectionalLight(VkCommandBuffer CommandBuffer);
	private:
		void CreateDebugSphere();
		void CreateDebugAABB();
		void CreateDebugCapsule();
		void CreateDebugCameraFrustum();
		void CreateDebugDirectionalLight();
	public:
		const VkDescriptorSet& GetDescriptor(uint32_t index);
		VkPipelineLayout GetPipelineLayout();
		const std::shared_ptr<Pipeline>& GetPipeline() const;

	private:
		struct DebugType
		{
			std::unique_ptr<VertexBuffer> m_VertexBuffer;
			std::unique_ptr<IndexBuffer> m_IndexBuffer;
		};
	private:
		std::shared_ptr<Material> m_DebugMaterialInstance;
		std::shared_ptr<Pipeline> m_DebugDrawPipeline;

		std::unique_ptr<DebugType> m_DebugSphere;
		std::unique_ptr<DebugType> m_DebugAABB;
		std::unique_ptr<DebugType> m_DebugCapsule;
		std::unique_ptr<DebugType> m_DebugCapsuleRadius;
		std::unique_ptr<DebugType> m_DebugCapsuleHalfExtent;

		std::unique_ptr<DebugType> m_DebugCameraFrustum;
		std::unique_ptr<DebugType> m_DebugLightDirection;

		std::shared_ptr<RenderPass> m_RenderPass;
	};
}