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
		DebugDrawPipelineConfig.LineWidth = 3.5f;
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

		int VertexCount = (int)DebugAABBVertices.size();
		uint32_t vertexSize = sizeof(DebugAABBVertices[0]);
		Buffer stagingBuffer(vertexSize, VertexCount, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		//Create a staging buffer to copy the vertex data to
		stagingBuffer.Map();
		stagingBuffer.WriteToBuffer((void*)DebugAABBVertices.data());

		//Flush data from staging buffer to vertex buffer
		m_DebugAABB->m_VertexBuffer = std::make_unique<Buffer>(vertexSize, VertexCount, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		VkDeviceSize bufferSize = vertexSize * VertexCount;
		vkUtils::CopyBuffer(stagingBuffer.GetBuffer(), m_DebugAABB->m_VertexBuffer->GetBuffer(), bufferSize);

		//Index
		m_DebugAABB->m_IndexCount = (uint32_t)DebugAABBIndices.size();

		uint32_t indexSize = sizeof(int);
		Buffer stagingBufferindex(indexSize, m_DebugAABB->m_IndexCount, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		stagingBufferindex.Map();
		stagingBufferindex.WriteToBuffer((void*)DebugAABBIndices.data());

		m_DebugAABB->m_IndexBuffer = std::make_unique<Buffer>(indexSize, m_DebugAABB->m_IndexCount, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		VkDeviceSize indexbufferSize = indexSize * m_DebugAABB->m_IndexCount;
		vkUtils::CopyBuffer(stagingBufferindex.GetBuffer(), m_DebugAABB->m_IndexBuffer->GetBuffer(), indexbufferSize);
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

		int VertexCount = (int)DebugSphereVert.size();

		uint32_t vertexSize = sizeof(DebugSphereVert[0]);
		Buffer stagingBuffer(vertexSize, VertexCount, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		//Create a staging buffer to copy the vertex data to
		stagingBuffer.Map();
		stagingBuffer.WriteToBuffer((void*)DebugSphereVert.data());

		//Flush data from staging buffer to vertex buffer
		m_DebugSphere->m_VertexBuffer = std::make_unique<Buffer>(vertexSize, VertexCount, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		const VkDeviceSize bufferSize = vertexSize * VertexCount;
		vkUtils::CopyBuffer(stagingBuffer.GetBuffer(), m_DebugSphere->m_VertexBuffer->GetBuffer(), bufferSize);

		//Index
		m_DebugSphere->m_IndexCount = (uint32_t)DebugSphereIndices.size();

		uint32_t indexSize = sizeof(int);
		Buffer stagingBufferindex(indexSize, m_DebugSphere->m_IndexCount, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		stagingBufferindex.Map();
		stagingBufferindex.WriteToBuffer((void*)DebugSphereIndices.data());

		m_DebugSphere->m_IndexBuffer = std::make_unique<Buffer>(indexSize, m_DebugSphere->m_IndexCount, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		VkDeviceSize indexbufferSize = indexSize * m_DebugSphere->m_IndexCount;
		vkUtils::CopyBuffer(stagingBufferindex.GetBuffer(), m_DebugSphere->m_IndexBuffer->GetBuffer(), indexbufferSize);
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

		int VertexCount = (int)DebugCapsuleVert.size();

		uint32_t vertexSize = sizeof(DebugCapsuleVert[0]);
		Buffer stagingBuffer(vertexSize, VertexCount, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		//Create a staging buffer to copy the vertex data to
		stagingBuffer.Map();
		stagingBuffer.WriteToBuffer((void*)DebugCapsuleVert.data());

		//Flush data from staging buffer to vertex buffer
		m_DebugCapsule->m_VertexBuffer = std::make_unique<Buffer>(vertexSize, VertexCount, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		const VkDeviceSize bufferSize = vertexSize * VertexCount;
		vkUtils::CopyBuffer(stagingBuffer.GetBuffer(), m_DebugCapsule->m_VertexBuffer->GetBuffer(), bufferSize);

		//Index
		m_DebugCapsule->m_IndexCount = (uint32_t)DebugCapsuleIndices.size();

		uint32_t indexSize = sizeof(int);
		Buffer stagingBufferindex(indexSize, m_DebugCapsule->m_IndexCount, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		stagingBufferindex.Map();
		stagingBufferindex.WriteToBuffer((void*)DebugCapsuleIndices.data());

		m_DebugCapsule->m_IndexBuffer = std::make_unique<Buffer>(indexSize, m_DebugCapsule->m_IndexCount, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		VkDeviceSize indexbufferSize = indexSize * m_DebugCapsule->m_IndexCount;
		vkUtils::CopyBuffer(stagingBufferindex.GetBuffer(), m_DebugCapsule->m_IndexBuffer->GetBuffer(), indexbufferSize);

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
			DebugVertex(glm::vec3(0.f,0.f,0.f), glm::vec4(0.f, 1.f, 0.f, 1.f)),
			DebugVertex(glm::vec3(0.5f,-0.5f,1.f), glm::vec4(0.f, 1.f, 0.f, 1.f)),
			DebugVertex(glm::vec3(0.5f,0.5f,1.f), glm::vec4(0.f, 1.f, 0.f, 1.f)),
			DebugVertex(glm::vec3(-0.5f,0.5f,1.f), glm::vec4(0.f, 1.f, 0.f, 1.f)),
			DebugVertex(glm::vec3(-0.5f,-0.5f,1.f), glm::vec4(0.f, 1.f, 0.f, 1.f)),
		};

		std::vector<int> DebugFrustumIndices = { 1,2,2,3,3,4,4,1,1,0,0,2,0,3,3,4,0 };

		int VertexCount = (int)DebugFrustumVertices.size();
		uint32_t vertexSize = sizeof(DebugFrustumVertices[0]);
		Buffer stagingBuffer(vertexSize, VertexCount, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		//Create a staging buffer to copy the vertex data to
		stagingBuffer.Map();
		stagingBuffer.WriteToBuffer((void*)DebugFrustumVertices.data());

		//Flush data from staging buffer to vertex buffer
		m_DebugCameraFrustum->m_VertexBuffer = std::make_unique<Buffer>(vertexSize, VertexCount, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		VkDeviceSize bufferSize = vertexSize * VertexCount;
		vkUtils::CopyBuffer(stagingBuffer.GetBuffer(), m_DebugCameraFrustum->m_VertexBuffer->GetBuffer(), bufferSize);

		//Index
		m_DebugCameraFrustum->m_IndexCount = (uint32_t)DebugFrustumIndices.size();

		uint32_t indexSize = sizeof(int);
		Buffer stagingBufferindex(indexSize, m_DebugCameraFrustum->m_IndexCount, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		stagingBufferindex.Map();
		stagingBufferindex.WriteToBuffer((void*)DebugFrustumIndices.data());

		m_DebugCameraFrustum->m_IndexBuffer = std::make_unique<Buffer>(indexSize, m_DebugCameraFrustum->m_IndexCount, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		VkDeviceSize indexbufferSize = indexSize * m_DebugCameraFrustum->m_IndexCount;
		vkUtils::CopyBuffer(stagingBufferindex.GetBuffer(), m_DebugCameraFrustum->m_IndexBuffer->GetBuffer(), indexbufferSize);
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
		vkCmdBindVertexBuffers(CommandBuffer, 0, 1, &m_DebugSphere->m_VertexBuffer->GetBuffer(), offsets);
		vkCmdBindIndexBuffer(CommandBuffer, m_DebugSphere->m_IndexBuffer->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);
	}

	void DebugRenderer::DrawDebugSphere(VkCommandBuffer CommandBuffer)
	{
		vkCmdDrawIndexed(CommandBuffer, m_DebugSphere->m_IndexCount, 1, 0, 0, 0);
	}

	void DebugRenderer::BindDebugAABB(VkCommandBuffer CommandBuffer)
	{
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(CommandBuffer, 0, 1, &m_DebugAABB->m_VertexBuffer->GetBuffer(), offsets);
		vkCmdBindIndexBuffer(CommandBuffer, m_DebugAABB->m_IndexBuffer->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);
	}

	void DebugRenderer::DrawDebugAABB(VkCommandBuffer CommandBuffer)
	{
		vkCmdDrawIndexed(CommandBuffer, m_DebugAABB->m_IndexCount, 1, 0, 0, 0);
	}

	void DebugRenderer::BindDebugCapsule(VkCommandBuffer CommandBuffer)
	{
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(CommandBuffer, 0, 1, &m_DebugCapsule->m_VertexBuffer->GetBuffer(), offsets);
		vkCmdBindIndexBuffer(CommandBuffer, m_DebugCapsule->m_IndexBuffer->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);
	}

	void DebugRenderer::DrawDebugCapsule(VkCommandBuffer CommandBuffer)
	{
		vkCmdDrawIndexed(CommandBuffer, m_DebugCapsule->m_IndexCount, 1, 0, 0, 0);
	}

	void DebugRenderer::BindDebugCameraFrustum(VkCommandBuffer CommandBuffer)
	{
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(CommandBuffer, 0, 1, &m_DebugCameraFrustum->m_VertexBuffer->GetBuffer(), offsets);
		vkCmdBindIndexBuffer(CommandBuffer, m_DebugCameraFrustum->m_IndexBuffer->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);
	}

	void DebugRenderer::DrawDebugCameraFrustum(VkCommandBuffer CommandBuffer)
	{
		vkCmdDrawIndexed(CommandBuffer, m_DebugCameraFrustum->m_IndexCount, 1, 0, 0, 0);
	}
}