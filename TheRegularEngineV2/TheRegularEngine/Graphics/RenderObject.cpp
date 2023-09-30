#include "pch.h"
#include "RenderObject.h"
#include "RendererContext.h"
#include "Core/Engine.h"
#include "Resource/ResourceManager.h"
#include "VulkanUtilities.h"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/hash.hpp"

namespace TRE
{
	RenderObject::RenderObject(const std::string& geomAsset)
	{
		std::unique_ptr<Geom> geom = Geom::Deserialize(geomAsset);

		m_Type = ResourceType::Mesh;

		std::vector<Vertex> vertices(geom->nPosition);
		std::vector<std::uint32_t> indices(geom->nIndices);

		for (uint32_t i = 0; i < geom->nPosition; ++i)
		{
			vertices[i].m_Position = geom->pPosition[i].Position;
			vertices[i].m_Normal = geom->pExtra[i].Normal;
			vertices[i].m_Tangent = geom->pExtra[i].Tangent;
			vertices[i].m_Bitangent = geom->pExtra[i].Bitangent;
			vertices[i].m_Color = geom->pExtra[i].Color;
			vertices[i].m_UV = geom->pExtra[i].UV;
		}

		for (uint32_t i = 0; i < geom->nIndices; ++i)
		{
			indices[i] = geom->pIndices[i];
		}

		CreateVertexBuffer(vertices);
		CreateIndexBuffer(indices);
		CreateBoundingSphere(vertices);
	}

	RenderObject::~RenderObject()
	{
		vkDeviceWaitIdle(RendererContext::GetDevice()->GetLogicalDevice());
	}

	void RenderObject::Bind(VkCommandBuffer commandBuffer)
	{
		VkBuffer vertexBuffers[] = { m_VertexBuffer->GetBuffer() };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

		if (m_HasIndexBuffer)
		{
			vkCmdBindIndexBuffer(commandBuffer, m_IndexBuffer->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);
		}
	}

	void RenderObject::Draw(VkCommandBuffer commandBuffer)
	{
		if (m_HasIndexBuffer)
		{
			vkCmdDrawIndexed(commandBuffer, m_IndexCount, 1, 0, 0, 0);
		}
		else
		{
			vkCmdDraw(commandBuffer, m_VertexCount, 1, 0, 0);
		}
	}

	void RenderObject::CreateVertexBuffer(const std::vector<Vertex>& vertices)
	{
		m_VertexCount = static_cast<std::uint32_t>(vertices.size());
		assert(m_VertexCount >= 3 && "Vertex count must be at least 3");

		uint32_t vertexSize = sizeof(vertices[0]);
		Buffer stagingBuffer(vertexSize, m_VertexCount, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		//Create a staging buffer to copy the vertex data to
		stagingBuffer.Map();
		stagingBuffer.WriteToBuffer((void*)vertices.data());

		//Flush data from staging buffer to vertex buffer
		m_VertexBuffer = std::make_unique<Buffer>(vertexSize, m_VertexCount, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		VkDeviceSize bufferSize = vertexSize * m_VertexCount;
		vkUtils::CopyBuffer(stagingBuffer.GetBuffer(), m_VertexBuffer->GetBuffer(), bufferSize);
	}

	void RenderObject::CreateIndexBuffer(const std::vector<std::uint32_t>& indices)
	{
		m_IndexCount = static_cast<std::uint32_t>(indices.size());
		m_HasIndexBuffer = m_IndexCount > 0;

		if (!m_HasIndexBuffer)
			return;

		uint32_t indexSize = sizeof(indices[0]);
		Buffer stagingBuffer(indexSize, m_IndexCount, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		stagingBuffer.Map();
		stagingBuffer.WriteToBuffer((void*)indices.data());

		m_IndexBuffer = std::make_unique<Buffer>(indexSize, m_IndexCount, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		VkDeviceSize bufferSize = indexSize * m_IndexCount;
		vkUtils::CopyBuffer(stagingBuffer.GetBuffer(), m_IndexBuffer->GetBuffer(), bufferSize);
	}

	void RenderObject::CreateBoundingSphere(const std::vector<Vertex>& vertices)
	{
		std::vector<glm::vec3> positions(vertices.size());
		for (int i = 0; i < vertices.size(); ++i)
		{
			positions[i] = vertices[i].m_Position;
		}

		m_BoundingSphere.Create(positions);
	}

	std::vector<VkVertexInputBindingDescription> RenderObject::Vertex::GetBindingDescriptions()
	{
		std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
		bindingDescriptions[0].binding = 0;
		bindingDescriptions[0].stride = sizeof(Vertex);
		bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return bindingDescriptions;
	}

	std::vector<VkVertexInputAttributeDescription> RenderObject::Vertex::GetAttributeDescriptions()
	{
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};

		attributeDescriptions.push_back({ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, m_Position) });
		attributeDescriptions.push_back({ 1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, m_Color) });
		attributeDescriptions.push_back({ 2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, m_Normal) });
		attributeDescriptions.push_back({ 3, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, m_Tangent) });
		attributeDescriptions.push_back({ 4, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, m_Bitangent) });
		attributeDescriptions.push_back({ 5, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, m_UV) });

		return attributeDescriptions;
	}

	void RenderObject::Serialize()
	{
		
	}

	std::shared_ptr<RenderObject> RenderObject::Deserialize(const std::string& assetHexGUID)
	{
		std::string geomString = "../Resources/" + assetHexGUID + ".geom";
		std::unique_ptr<RenderObject> ro = std::make_unique<RenderObject>(geomString);
		ResourceHandle assetHandle = Resource::GetGUIDFromHex(assetHexGUID);
		ro->m_Handle = assetHandle;
		ResourceManager::Instance().AddResource(std::move(ro));

		return std::move(ResourceManager::Instance().GetResource<RenderObject>(assetHandle));
	}
}