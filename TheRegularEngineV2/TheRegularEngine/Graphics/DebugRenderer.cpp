#include "pch.h"
#include "DebugRenderer.h"
#include "Assets/AssetManager.h"
#include "RendererContext.h"

namespace TRE
{
	static void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
	{
		auto logicalDevice = RendererContext::GetDevice();
		VkCommandBuffer commandBuffer = logicalDevice->AllocateCommandBuffer(true);

		VkBufferCopy copyRegion{};
		copyRegion.srcOffset = 0;  // Optional
		copyRegion.dstOffset = 0;  // Optional
		copyRegion.size = size;
		vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

		logicalDevice->SubmitCommands(commandBuffer);
	}

	const VkDescriptorSet& DebugRenderer::GetDescriptor(uint32_t index)
	{
		return m_DebugMaterialInstance->GetDescriptor(index);
	}

	const VkPipelineLayout& DebugRenderer::GetPipelineLayout()
	{
		return m_DebugDrawPipeline->GetPipelineLayout();
	}

	DebugRenderer::DebugRenderer(const std::shared_ptr<RenderPass>& TargetPass)
	{
		auto DebugDrawVertShader = AssetManager::Instance().GetAsset<Shader>(7);
		auto DebugDrawFragShader = AssetManager::Instance().GetAsset<Shader>(8);

		//Debug Draw Pipelines
		std::vector<VkVertexInputBindingDescription> DebugDrawBindingDescriptions(1);
		DebugDrawBindingDescriptions[0].binding = 0;
		DebugDrawBindingDescriptions[0].stride = sizeof(DebugVertex);
		DebugDrawBindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		std::vector<VkVertexInputAttributeDescription> DebugDrawattributeDescriptions{};
		DebugDrawattributeDescriptions.push_back({ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(DebugVertex, Position) });
		DebugDrawattributeDescriptions.push_back({ 1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(DebugVertex, Color) });

		PipelineConfigurations DebugDrawPipelineConfig;
		DebugDrawPipelineConfig.Primitive = PrimitiveType::LinesStrip;
		DebugDrawPipelineConfig.VertexShader = DebugDrawVertShader;
		DebugDrawPipelineConfig.FragmentShader = DebugDrawFragShader;
		DebugDrawPipelineConfig.VertexBindingDescriptions = DebugDrawBindingDescriptions;
		DebugDrawPipelineConfig.VertexAttributeDescriptions = DebugDrawattributeDescriptions;
		m_DebugDrawPipeline = std::make_unique<Pipeline>(DebugDrawPipelineConfig, TargetPass);

		m_DebugMaterialInstance = std::make_shared<Material>(DebugDrawVertShader, DebugDrawFragShader);
		m_DebugMaterialInstance->AllocateLayouts();

		CreateDebugSphere();
		CreateDebugAABB();
	}

	void DebugRenderer::CreateDebugAABB()
	{
		std::vector<DebugVertex> DebugAABBVertices =
		{
			DebugVertex(glm::vec3(0.f,0.f,0.f), glm::vec4(0.f, 1.f, 0.f, 1.f)),
			DebugVertex(glm::vec3(0.f,1.f,0.f), glm::vec4(0.f, 1.f, 0.f, 1.f)),
			DebugVertex(glm::vec3(1.f,1.f,0.f), glm::vec4(0.f, 1.f, 0.f, 1.f)),
			DebugVertex(glm::vec3(1.f,0.f,0.f), glm::vec4(0.f, 1.f, 0.f, 1.f)),
			DebugVertex(glm::vec3(0.f,0.f,1.f), glm::vec4(0.f, 1.f, 0.f, 1.f)),
			DebugVertex(glm::vec3(0.f,1.f,1.f), glm::vec4(0.f, 1.f, 0.f, 1.f)),
			DebugVertex(glm::vec3(1.f,1.f,1.f), glm::vec4(0.f, 1.f, 0.f, 1.f)),
			DebugVertex(glm::vec3(1.f,0.f,1.f), glm::vec4(0.f, 1.f, 0.f, 1.f))
		};

		std::vector<int> DebugAABBIndices = { 0, 1, 2, 3, 0, 4, 5, 6, 7, 4, 0, 4, 1, 5, 2, 6, 3, 7, 0, 4 };

		int VertexCount = DebugAABBVertices.size();
		uint32_t vertexSize = sizeof(DebugAABBVertices[0]);
		Buffer stagingBuffer(vertexSize, VertexCount, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		//Create a staging buffer to copy the vertex data to
		stagingBuffer.Map();
		stagingBuffer.WriteToBuffer((void*)DebugAABBVertices.data());

		//Flush data from staging buffer to vertex buffer
		m_DebugAABBVertexBuffer = std::make_unique<Buffer>(vertexSize, VertexCount, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		VkDeviceSize bufferSize = vertexSize * VertexCount;
		CopyBuffer(stagingBuffer.GetBuffer(), m_DebugAABBVertexBuffer->GetBuffer(), bufferSize);

		//Index
		m_AABBIndexCount = DebugAABBIndices.size();

		uint32_t indexSize = sizeof(int);
		Buffer stagingBufferindex(indexSize, m_AABBIndexCount, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		stagingBufferindex.Map();
		stagingBufferindex.WriteToBuffer((void*)DebugAABBIndices.data());

		m_DebugAABBIndexBuffer = std::make_unique<Buffer>(indexSize, m_AABBIndexCount, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		VkDeviceSize indexbufferSize = indexSize * m_AABBIndexCount;
		CopyBuffer(stagingBufferindex.GetBuffer(), m_DebugAABBIndexBuffer->GetBuffer(), indexbufferSize);
	}

	void DebugRenderer::CreateDebugSphere()
	{
		std::vector<DebugVertex> DebugSphereVert;
		std::vector<int> DebugSphereIndices;
		float Theta = (3.14 * 2) / 48.f;
		for (int x = 0; x < 48; x++)
		{
			DebugSphereVert.push_back(DebugVertex(glm::vec3(cosf(Theta * x), sinf((Theta * x)), 0), glm::vec4(0.f, 1.f, 0.f, 1.f)));
			DebugSphereIndices.push_back(x);
		}

		int VertexCount = DebugSphereVert.size();

		uint32_t vertexSize = sizeof(DebugSphereVert[0]);
		Buffer stagingBuffer(vertexSize, VertexCount, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		//Create a staging buffer to copy the vertex data to
		stagingBuffer.Map();
		stagingBuffer.WriteToBuffer((void*)DebugSphereVert.data());

		//Flush data from staging buffer to vertex buffer
		m_DebugSphereVertexBuffer = std::make_unique<Buffer>(vertexSize, VertexCount, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		VkDeviceSize bufferSize = vertexSize * VertexCount;
		CopyBuffer(stagingBuffer.GetBuffer(), m_DebugSphereVertexBuffer->GetBuffer(), bufferSize);

		//Index
		m_SphereIndexCount = DebugSphereIndices.size();

		uint32_t indexSize = sizeof(int);
		Buffer stagingBufferindex(indexSize, m_SphereIndexCount, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		stagingBufferindex.Map();
		stagingBufferindex.WriteToBuffer((void*)DebugSphereIndices.data());

		m_DebugSphereIndexBuffer = std::make_unique<Buffer>(indexSize, m_SphereIndexCount, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		VkDeviceSize indexbufferSize = indexSize * m_SphereIndexCount;
		CopyBuffer(stagingBufferindex.GetBuffer(), m_DebugSphereIndexBuffer->GetBuffer(), indexbufferSize);
	}

	DebugRenderer::~DebugRenderer()
	{

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
		vkCmdBindVertexBuffers(CommandBuffer, 0, 1, &m_DebugSphereVertexBuffer->GetBuffer(), offsets);
		vkCmdBindIndexBuffer(CommandBuffer, m_DebugSphereIndexBuffer->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);
	}

	void DebugRenderer::DrawDebugSphere(VkCommandBuffer CommandBuffer)
	{
		vkCmdDrawIndexed(CommandBuffer, m_SphereIndexCount, 1, 0, 0, 0);
	}

	void DebugRenderer::BindDebugAABB(VkCommandBuffer CommandBuffer)
	{
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(CommandBuffer, 0, 1, &m_DebugAABBVertexBuffer->GetBuffer(), offsets);
		vkCmdBindIndexBuffer(CommandBuffer, m_DebugAABBIndexBuffer->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);
	}

	void DebugRenderer::DrawDebugAABB(VkCommandBuffer CommandBuffer)
	{
		vkCmdDrawIndexed(CommandBuffer, m_AABBIndexCount, 1, 0, 0, 0);
	}
}