#include "pch.h"
#include "DebugRenderer.h"
#include "Resource/ResourceManager.h"
#include "RendererContext.h"
#include "VulkanUtilities.h"
#include "glm/gtc/matrix_transform.hpp"

namespace TRE
{
	const VkDescriptorSet& DebugRenderer::GetDescriptor(uint32_t index)
	{
		return m_DebugMaterialInstance->GetDescriptor(index);
	}

	VkPipelineLayout DebugRenderer::GetPipelineLayout()
	{
		return m_DebugDrawPipeline->GetPipelineLayout();
	}

	const std::shared_ptr<Pipeline>& DebugRenderer::GetPipeline() const
	{
		return m_DebugDrawPipeline;
	}

	DebugRenderer::DebugRenderer(std::shared_ptr<RenderPass> TargetPass) : m_RenderPass(TargetPass)
	{
		auto DebugDrawShader = ResourceManager::Instance().GetResource<Shader>(7);

		PipelineConfigurations DebugDrawPipelineConfig{};
		DebugDrawPipelineConfig.Primitive = PrimitiveType::Lines;
		DebugDrawPipelineConfig.Shader = DebugDrawShader;
		DebugDrawPipelineConfig.LineWidth = 2.5f;
		DebugDrawPipelineConfig.EnableDepthTest = true;
		DebugDrawPipelineConfig.EnableBlending = true;
		m_DebugDrawPipeline = std::make_shared<Pipeline>(DebugDrawPipelineConfig, m_RenderPass);

		m_DebugMaterialInstance = std::make_shared<Material>(DebugDrawShader);
		m_DebugMaterialInstance->Invalidate();

		CreateDebugSphere();
		CreateDebugAABB();
		CreateDebugCapsule();
		CreateDebugCameraFrustum();
		CreateDebugDirectionalLight();
	}

	void DebugRenderer::CreateDebugAABB()
	{
		m_DebugAABB = std::make_unique<DebugType>();

		std::vector<DebugVertex> DebugAABBVertices =
		{
			DebugVertex(glm::vec3(-0.5f,-0.5f,-0.5f), glm::vec4(0.f, 1.f, 0.f, 0.85f)),
			DebugVertex(glm::vec3(-0.5f,0.5f,-0.5f), glm::vec4(0.f, 1.f, 0.f, 0.85f)),
			DebugVertex(glm::vec3(0.5f,0.5f,-0.5f), glm::vec4(0.f, 1.f, 0.f, 0.85f)),
			DebugVertex(glm::vec3(0.5f,-0.5f,-0.5f), glm::vec4(0.f, 1.f, 0.f, 0.85f)),
			DebugVertex(glm::vec3(-0.5f,-0.5f,0.5f), glm::vec4(0.f, 1.f, 0.f, 0.85f)),
			DebugVertex(glm::vec3(-0.5f,0.5f,0.5f), glm::vec4(0.f, 1.f, 0.f, 0.85f)),
			DebugVertex(glm::vec3(0.5f,0.5f,0.5f), glm::vec4(0.f, 1.f, 0.f, 0.85f)),
			DebugVertex(glm::vec3(0.5f,-0.5f,0.5f), glm::vec4(0.f, 1.f, 0.f, 0.85f))
		};

		std::vector<int> DebugAABBIndices = { 0, 1, 1, 2, 2, 3, 3, 0, 0, 4, 4, 5, 5, 1, 1, 2, 2, 6, 6, 5, 5, 4, 4, 7, 7, 3, 7, 6 };

		m_DebugAABB->m_VertexBuffer = std::make_unique<VertexBuffer>(static_cast<void*>(DebugAABBVertices.data()),
			UINT32_T_CAST(sizeof(DebugAABBVertices[0]) * DebugAABBVertices.size()));

		m_DebugAABB->m_IndexBuffer = std::make_unique<IndexBuffer>(static_cast<void*>(DebugAABBIndices.data()),
			UINT32_T_CAST(sizeof(int) * DebugAABBIndices.size()),
			UINT32_T_CAST(DebugAABBIndices.size()));
	}

	void DebugRenderer::CreateDebugSphere()
	{
		m_DebugSphere = std::make_unique<DebugType>();

		std::vector<DebugVertex> DebugSphereVert;
		std::vector<int> DebugSphereIndices;
		const int slices = 48;
		float Theta = (3.14f * 2) / slices;
		for (int x = 0; x < slices; x++)
		{
			DebugSphereVert.push_back(DebugVertex(glm::vec3(cosf(Theta * x), sinf(Theta * x), 0), glm::vec4(0.f, 1.f, 0.f, 1.f)));
			DebugSphereIndices.push_back(x);
		}
		DebugSphereIndices.push_back(0); //Strip back to the first point

		m_DebugSphere->m_VertexBuffer = std::make_unique<VertexBuffer>(static_cast<void*>(DebugSphereVert.data()),
			UINT32_T_CAST(DebugSphereVert.size() * sizeof(DebugSphereVert[0])));

		m_DebugSphere->m_IndexBuffer = std::make_unique<IndexBuffer>(static_cast<void*>(DebugSphereIndices.data()),
			UINT32_T_CAST(sizeof(int) * DebugSphereIndices.size()),
			UINT32_T_CAST(DebugSphereIndices.size()));
	}

	void DebugRenderer::CreateDebugCapsule()
	{
		m_DebugCapsuleRadius = std::make_unique<DebugType>();
		m_DebugCapsuleHalfExtent = std::make_unique<DebugType>();

		const int slices = 12;
		float Theta = (3.142f) / slices;
		std::vector<DebugVertex> DebugCapsuleRadiusVert;
		std::vector<int> DebugCapsuleRadiusIndices;
		//Top half - first axis
		for (int x = 0; x < slices;)
		{
			DebugCapsuleRadiusVert.push_back(DebugVertex(glm::vec3(cosf(Theta * x) / 2.f, sinf(Theta * x) / 2.f, 0), glm::vec4(0.f, 1.f, 0.f, 1.f)));
			DebugCapsuleRadiusIndices.push_back(x++);
			DebugCapsuleRadiusIndices.push_back(x);
		}

		DebugCapsuleRadiusVert.push_back(DebugVertex(glm::vec3(-0.5f, 0, 0), glm::vec4(0.f, 1.f, 0.f, 1.f)));
		DebugCapsuleRadiusIndices.push_back((int)DebugCapsuleRadiusIndices.size());
		DebugCapsuleRadiusIndices.push_back((int)DebugCapsuleRadiusIndices.size());

		DebugCapsuleRadiusVert.push_back(DebugVertex(glm::vec3(0.f, 0.f, -0.5f), glm::vec4(0.f, 1.f, 0.f, 1.f)));
		DebugCapsuleRadiusIndices.push_back((int)DebugCapsuleRadiusIndices.size());

		m_DebugCapsuleRadius->m_VertexBuffer = std::make_unique<VertexBuffer>(static_cast<void*>(DebugCapsuleRadiusVert.data()),
			UINT32_T_CAST(DebugCapsuleRadiusVert.size() * sizeof(DebugVertex)));

		m_DebugCapsuleRadius->m_IndexBuffer = std::make_unique<IndexBuffer>(static_cast<void*>(DebugCapsuleRadiusIndices.data()),
			UINT32_T_CAST(DebugCapsuleRadiusIndices.size() * sizeof(int)),
			UINT32_T_CAST(DebugCapsuleRadiusIndices.size()));

		std::vector<DebugVertex> DebugCapsuleExtentVert;
		std::vector<int> DebugCapsuleExtentIndices;

		DebugCapsuleExtentVert.push_back(DebugVertex(glm::vec3(-0.5f, 0.f, 0.f), glm::vec4(0.f, 1.f, 0.f, 1.f)));
		DebugCapsuleExtentVert.push_back(DebugVertex(glm::vec3(-0.5f, 0.5f, 0.f), glm::vec4(0.f, 1.f, 0.f, 1.f)));
		DebugCapsuleExtentVert.push_back(DebugVertex(glm::vec3(0.5f, 0.f, 0.f), glm::vec4(0.f, 1.f, 0.f, 1.f)));
		DebugCapsuleExtentVert.push_back(DebugVertex(glm::vec3(0.5f, 0.5f, 0.f), glm::vec4(0.f, 1.f, 0.f, 1.f)));
		DebugCapsuleExtentIndices.push_back((int)DebugCapsuleExtentIndices.size());
		DebugCapsuleExtentIndices.push_back((int)DebugCapsuleExtentIndices.size());
		DebugCapsuleExtentIndices.push_back((int)DebugCapsuleExtentIndices.size());
		DebugCapsuleExtentIndices.push_back((int)DebugCapsuleExtentIndices.size());

		m_DebugCapsuleHalfExtent->m_VertexBuffer = std::make_unique<VertexBuffer>(static_cast<void*>(DebugCapsuleExtentVert.data()),
			UINT32_T_CAST(DebugCapsuleExtentVert.size() * sizeof(DebugVertex)));

		m_DebugCapsuleHalfExtent->m_IndexBuffer = std::make_unique<IndexBuffer>(static_cast<void*>(DebugCapsuleExtentIndices.data()),
			UINT32_T_CAST(DebugCapsuleExtentIndices.size() * sizeof(int)),
			UINT32_T_CAST(DebugCapsuleExtentIndices.size()));
	}

	void DebugRenderer::CreateDebugCameraFrustum()
	{
		m_DebugCameraFrustum = std::make_unique<DebugType>();

		std::vector<DebugVertex> DebugFrustumVertices =
		{
			DebugVertex(glm::vec3(0.f,0.f,0.f), glm::vec4(0.2f, 0.2f, 0.2f, 1.f)),
			DebugVertex(glm::vec3(0.5f,-0.5f,1.f), glm::vec4(0.2f, 0.2f, 0.2f, 1.f)),
			DebugVertex(glm::vec3(0.5f,0.5f,1.f), glm::vec4(0.2f, 0.2f, 0.2f, 1.f)),
			DebugVertex(glm::vec3(-0.5f,0.5f,1.f), glm::vec4(0.2f, 0.2f, 0.2f, 1.f)),
			DebugVertex(glm::vec3(-0.5f,-0.5f,1.f), glm::vec4(0.2f, 0.2f, 0.2f, 1.f))
		};

		std::vector<int> DebugFrustumIndices = { 1,2,2,3,3,4,4,1,1,0,0,2,0,3,3,4,0 };
		
		m_DebugCameraFrustum->m_VertexBuffer = std::make_unique<VertexBuffer>(static_cast<void*>(DebugFrustumVertices.data()),
			UINT32_T_CAST(DebugFrustumVertices.size() * sizeof(DebugVertex)));

		m_DebugCameraFrustum->m_IndexBuffer = std::make_unique<IndexBuffer>(static_cast<void*>(DebugFrustumIndices.data()),
			UINT32_T_CAST(DebugFrustumIndices.size() * sizeof(int)),
			UINT32_T_CAST(DebugFrustumIndices.size()));
	}

	void DebugRenderer::CreateDebugDirectionalLight()
	{
		m_DebugLightDirection = std::make_unique<DebugType>();

		std::vector<DebugVertex> DebugLightDirectionVertices =
		{
			DebugVertex(glm::vec3(0.f,0.f,-0.5f), glm::vec4(1.f, 1.f, 0.f, 1.f)),
			DebugVertex(glm::vec3(0.f,0.f,0.25f), glm::vec4(1.f, 1.f, 0.f, 1.f)),
			DebugVertex(glm::vec3(0.f,0.25f,0.25f), glm::vec4(1.f, 1.f, 0.f, 1.f)),
			DebugVertex(glm::vec3(0.f,0.f,0.5f), glm::vec4(1.f, 1.f, 0.f, 1.f)),
			DebugVertex(glm::vec3(0.f,-0.25f,0.25f), glm::vec4(1.f, 1.f, 0.f, 1.f)),
		};

		std::vector<int> DebugLightDirectionIndices = { 0,1,2,3,4,2 };

		m_DebugLightDirection->m_VertexBuffer = std::make_unique<VertexBuffer>(static_cast<void*>(DebugLightDirectionVertices.data()),
			UINT32_T_CAST(DebugLightDirectionVertices.size() * sizeof(DebugVertex)));

		m_DebugLightDirection->m_IndexBuffer = std::make_unique<IndexBuffer>(static_cast<void*>(DebugLightDirectionIndices.data()),
			UINT32_T_CAST(DebugLightDirectionIndices.size() * sizeof(int)),
			UINT32_T_CAST(DebugLightDirectionIndices.size()));
	}

	void DebugRenderer::UpdateMaterial(const std::shared_ptr<UniformBuffer>& UBO, uint32_t Index)
	{
		m_DebugMaterialInstance->UpdateForRendering(UBO, Index, VkDescriptorImageInfo());
	}

	void DebugRenderer::BindPipeline(VkCommandBuffer CommandBuffer)
	{
		vkCmdBindPipeline(CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_DebugDrawPipeline->GetPipeline());
	}

	void DebugRenderer::BindDebugSphere(VkCommandBuffer CommandBuffer)
	{
		VkDeviceSize offsets[] = { 0 };
		VkBuffer VertexBuffer = m_DebugSphere->m_VertexBuffer->GetBuffer();
		vkCmdBindVertexBuffers(CommandBuffer, 0, 1, &VertexBuffer, offsets);
		vkCmdBindIndexBuffer(CommandBuffer, m_DebugSphere->m_IndexBuffer->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);
	}

	void DebugRenderer::DrawDebugSphere(VkCommandBuffer CommandBuffer)
	{
		vkCmdDrawIndexed(CommandBuffer, m_DebugSphere->m_IndexBuffer->GetIndexCount(), 1, 0, 0, 0);
	}

	void DebugRenderer::BindDebugAABB(VkCommandBuffer CommandBuffer)
	{
		VkDeviceSize offsets[] = { 0 };
		VkBuffer VertexBuffer = m_DebugAABB->m_VertexBuffer->GetBuffer();
		vkCmdBindVertexBuffers(CommandBuffer, 0, 1, &VertexBuffer, offsets);
		vkCmdBindIndexBuffer(CommandBuffer, m_DebugAABB->m_IndexBuffer->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);
	}

	void DebugRenderer::DrawDebugAABB(VkCommandBuffer CommandBuffer)
	{
		vkCmdDrawIndexed(CommandBuffer, m_DebugAABB->m_IndexBuffer->GetIndexCount(), 1, 0, 0, 0);
	}

	void DebugRenderer::BindDebugCapsuleRadius(VkCommandBuffer CommandBuffer)
	{
		VkDeviceSize offsets[] = { 0 };
		VkBuffer VertexBuffer = m_DebugCapsuleRadius->m_VertexBuffer->GetBuffer();
		vkCmdBindVertexBuffers(CommandBuffer, 0, 1, &VertexBuffer, offsets);
		vkCmdBindIndexBuffer(CommandBuffer, m_DebugCapsuleRadius->m_IndexBuffer->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);
	}

	void DebugRenderer::DrawDebugCapsuleRadius(VkCommandBuffer CommandBuffer)
	{
		vkCmdDrawIndexed(CommandBuffer, m_DebugCapsuleRadius->m_IndexBuffer->GetIndexCount(), 1, 0, 0, 0);
	}

	void DebugRenderer::BindDebugCapsuleHalfExtent(VkCommandBuffer CommandBuffer)
	{
		VkDeviceSize offsets[] = { 0 };
		VkBuffer VertexBuffer = m_DebugCapsuleHalfExtent->m_VertexBuffer->GetBuffer();
		vkCmdBindVertexBuffers(CommandBuffer, 0, 1, &VertexBuffer, offsets);
		vkCmdBindIndexBuffer(CommandBuffer, m_DebugCapsuleHalfExtent->m_IndexBuffer->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);
	}

	void DebugRenderer::DrawDebugCapsuleHalfExtent(VkCommandBuffer CommandBuffer)
	{
		vkCmdDrawIndexed(CommandBuffer, m_DebugCapsuleHalfExtent->m_IndexBuffer->GetIndexCount(), 1, 0, 0, 0);
	}

	void DebugRenderer::BindDebugCameraFrustum(VkCommandBuffer CommandBuffer)
	{
		VkDeviceSize offsets[] = { 0 };
		VkBuffer VertexBuffer = m_DebugCameraFrustum->m_VertexBuffer->GetBuffer();
		vkCmdBindVertexBuffers(CommandBuffer, 0, 1, &VertexBuffer, offsets);
		vkCmdBindIndexBuffer(CommandBuffer, m_DebugCameraFrustum->m_IndexBuffer->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);
	}

	void DebugRenderer::DrawDebugCameraFrustum(VkCommandBuffer CommandBuffer)
	{
		vkCmdDrawIndexed(CommandBuffer, m_DebugCameraFrustum->m_IndexBuffer->GetIndexCount(), 1, 0, 0, 0);
	}

	void DebugRenderer::BindDebugDirectionalLight(VkCommandBuffer CommandBuffer)
	{
		VkDeviceSize offsets[] = { 0 };
		VkBuffer VertexBuffer = m_DebugLightDirection->m_VertexBuffer->GetBuffer();
		vkCmdBindVertexBuffers(CommandBuffer, 0, 1, &VertexBuffer, offsets);
		vkCmdBindIndexBuffer(CommandBuffer, m_DebugLightDirection->m_IndexBuffer->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);
	}

	void DebugRenderer::DrawDebugDirectionalLight(VkCommandBuffer CommandBuffer)
	{
		vkCmdDrawIndexed(CommandBuffer, m_DebugLightDirection->m_IndexBuffer->GetIndexCount(), 1, 0, 0, 0);
	}
}