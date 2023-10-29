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

	DebugRenderer::DebugRenderer(std::shared_ptr<RenderPass> TargetPass) : m_RenderPass(TargetPass)
	{
		auto DebugDrawShader = ResourceManager::Instance().GetResource<Shader>(7);

		PipelineConfigurations DebugDrawPipelineConfig{};
		DebugDrawPipelineConfig.Primitive = PrimitiveType::LinesStrip;
		DebugDrawPipelineConfig.Shader = DebugDrawShader;
		DebugDrawPipelineConfig.LineWidth = 2.5f;
		m_DebugDrawPipeline = std::make_unique<Pipeline>(DebugDrawPipelineConfig, m_RenderPass);

		m_DebugMaterialInstance = std::make_shared<Material>(DebugDrawShader);
		m_DebugMaterialInstance->Invalidate();

		CreateDebugSphere();
		CreateDebugAABB();
		CreateDebugCapsule();
		CreateDebugCameraFrustum();
	}

	void DebugRenderer::CreateDebugAABB()
	{
		m_DebugAABB = std::make_unique<DebugType>();

		std::vector<DebugVertex> DebugAABBVertices =
		{
			DebugVertex(glm::vec3(-0.5f,-0.5f,-0.5f), glm::vec4(0.f, 1.f, 0.f, 1.f)),
			DebugVertex(glm::vec3(-0.5f,0.5f,-0.5f), glm::vec4(0.f, 1.f, 0.f, 1.f)),
			DebugVertex(glm::vec3(0.5f,0.5f,-0.5f), glm::vec4(0.f, 1.f, 0.f, 1.f)),
			DebugVertex(glm::vec3(0.5f,-0.5f,-0.5f), glm::vec4(0.f, 1.f, 0.f, 1.f)),
			DebugVertex(glm::vec3(-0.5f,-0.5f,0.5f), glm::vec4(0.f, 1.f, 0.f, 1.f)),
			DebugVertex(glm::vec3(-0.5f,0.5f,0.5f), glm::vec4(0.f, 1.f, 0.f, 1.f)),
			DebugVertex(glm::vec3(0.5f,0.5f,0.5f), glm::vec4(0.f, 1.f, 0.f, 1.f)),
			DebugVertex(glm::vec3(0.5f,-0.5f,0.5f), glm::vec4(0.f, 1.f, 0.f, 1.f))
		};

		std::vector<int> DebugAABBIndices = { 0, 1, 2, 3, 0, 4, 5, 6, 7, 4, 0, 4, 1, 5, 2, 6, 3, 7, 0, 4 };

		m_DebugAABB->m_VertexBuffer = std::make_unique<VertexBuffer>((void*)DebugAABBVertices.data(), sizeof(DebugAABBVertices[0]) * DebugAABBVertices.size());
		m_DebugAABB->m_IndexBuffer = std::make_unique<IndexBuffer>((void*)DebugAABBIndices.data(), sizeof(int) * DebugAABBIndices.size(), DebugAABBIndices.size());
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

		m_DebugSphere->m_VertexBuffer = std::make_unique<VertexBuffer>((void*)DebugSphereVert.data(), DebugSphereVert.size() * sizeof(DebugSphereVert[0]));
		m_DebugSphere->m_IndexBuffer = std::make_unique<IndexBuffer>((void*)DebugSphereIndices.data(), sizeof(int) * DebugSphereIndices.size(), DebugSphereIndices.size());
	}

	void DebugRenderer::CreateDebugCapsule()
	{
		m_DebugCapsule = std::make_unique<DebugType>();

		std::vector<DebugVertex> DebugCapsuleVert;
		std::vector<int> DebugCapsuleIndices;
		const int slices = 24;
		float Theta = (3.142f) / slices;
		//Top half
		for (int x = 0; x < slices; x++)
		{
			DebugCapsuleVert.push_back(DebugVertex(glm::vec3(cosf(Theta * x) / 2.f, sinf(Theta * x) / 4.f + 0.25f, 0), glm::vec4(0.f, 1.f, 0.f, 1.f)));
			DebugCapsuleIndices.push_back(x);
		}

		DebugCapsuleVert.push_back(DebugVertex(glm::vec3(-0.5f, 0.25f, 0), glm::vec4(0.f, 1.f, 0.f, 1.f)));
		DebugCapsuleIndices.push_back((int)DebugCapsuleIndices.size());

		//Circular rim
		Theta = -(3.14f * 2) / slices;
		const int rimIndex = (int)DebugCapsuleIndices.size();
		for (int x = 0; x < slices + 1; x++)
		{
			DebugCapsuleVert.push_back(DebugVertex(glm::vec3(cosf(Theta * x) / 2.f, 0.25f, sinf(Theta * x) / 2.f), glm::vec4(0.f, 1.f, 0.f, 1.f)));
			DebugCapsuleIndices.push_back(x + rimIndex);
		}

		DebugCapsuleVert.push_back(DebugVertex(glm::vec3(-0.5f, 0.25f, 0), glm::vec4(0.f, 1.f, 0.f, 1.f)));
		DebugCapsuleIndices.push_back((int)DebugCapsuleIndices.size());
		
		Theta = (-3.142f) / slices;
		const int StartIndex = (int)DebugCapsuleIndices.size();
		//Bottom half
		for (int x = 0; x < slices; x++)
		{
			DebugCapsuleVert.push_back(DebugVertex(glm::vec3(-cosf(Theta * x) / 2.f, sinf(Theta * x) / 4.f - 0.25f, 0), glm::vec4(0.f, 1.f, 0.f, 1.f)));
			DebugCapsuleIndices.push_back(x + StartIndex);
		}

		DebugCapsuleVert.push_back(DebugVertex(glm::vec3(0.5, 0.25, 0), glm::vec4(0.f, 1.f, 0.f, 1.f)));
		DebugCapsuleIndices.push_back((int)DebugCapsuleIndices.size());

		m_DebugCapsule->m_VertexBuffer = std::make_unique<VertexBuffer>((void*)DebugCapsuleVert.data(), DebugCapsuleVert.size() * sizeof(DebugVertex));
		m_DebugCapsule->m_IndexBuffer = std::make_unique<IndexBuffer>((void*)DebugCapsuleIndices.data(), DebugCapsuleIndices.size() * sizeof(int), DebugCapsuleIndices.size());
	}

	void DebugRenderer::CreateDebugCameraFrustum()
	{
		m_DebugCameraFrustum = std::make_unique<DebugType>();

		/*std::vector<DebugVertex> DebugFrustumVertices =
		{
			DebugVertex(glm::vec3(0.f,0.f,0.f), glm::vec4(0.f, 1.f, 0.f, 1.f)),
			DebugVertex(glm::vec3(0.f,0.f,1.f), glm::vec4(0.f, 1.f, 0.f, 1.f))
		};*/
		std::vector<DebugVertex> DebugFrustumVertices =
		{
			DebugVertex(glm::vec3(0.f,0.f,0.f), glm::vec4(0.2f, 0.2f, 0.2f, 1.f)),
			DebugVertex(glm::vec3(0.5f,-0.5f,1.f), glm::vec4(0.2f, 0.2f, 0.2f, 1.f)),
			DebugVertex(glm::vec3(0.5f,0.5f,1.f), glm::vec4(0.2f, 0.2f, 0.2f, 1.f)),
			DebugVertex(glm::vec3(-0.5f,0.5f,1.f), glm::vec4(0.2f, 0.2f, 0.2f, 1.f)),
			DebugVertex(glm::vec3(-0.5f,-0.5f,1.f), glm::vec4(0.2f, 0.2f, 0.2f, 1.f))
		};

		std::vector<int> DebugFrustumIndices = { 1,2,2,3,3,4,4,1,1,0,0,2,0,3,3,4,0 };
		
		m_DebugCameraFrustum->m_VertexBuffer = std::make_unique<VertexBuffer>((void*)DebugFrustumVertices.data(), DebugFrustumVertices.size() * sizeof(DebugVertex));
		m_DebugCameraFrustum->m_IndexBuffer = std::make_unique<IndexBuffer>((void*)DebugFrustumIndices.data(), DebugFrustumIndices.size() * sizeof(int), DebugFrustumIndices.size());
	}

	void DebugRenderer::UpdateMaterial(const std::shared_ptr<UniformBuffer>& UBO, uint32_t Index)
	{
		m_DebugMaterialInstance->UpdateForRendering(UBO, Index);
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

	void DebugRenderer::BindDebugCapsule(VkCommandBuffer CommandBuffer)
	{
		VkDeviceSize offsets[] = { 0 };
		VkBuffer VertexBuffer = m_DebugCapsule->m_VertexBuffer->GetBuffer();
		vkCmdBindVertexBuffers(CommandBuffer, 0, 1, &VertexBuffer, offsets);
		vkCmdBindIndexBuffer(CommandBuffer, m_DebugCapsule->m_IndexBuffer->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);
	}

	void DebugRenderer::DrawDebugCapsule(VkCommandBuffer CommandBuffer)
	{
		vkCmdDrawIndexed(CommandBuffer, m_DebugCapsule->m_IndexBuffer->GetIndexCount(), 1, 0, 0, 0);
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
}