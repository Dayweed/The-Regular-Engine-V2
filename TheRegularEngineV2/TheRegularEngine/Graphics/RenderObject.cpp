#include "pch.h"
#include "RenderObject.h"
#include "RendererContext.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tinyobjloader/tiny_obj_loader.h"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/hash.hpp"

namespace
{
	template<typename T, typename... Rest>
	void HashCombine(std::size_t& seed, const T& v, Rest&... rest)
	{
		seed ^= std::hash<T>{}(v)+0x9e3779b9 + (seed << 6) + (seed >> 2);
		(HashCombine(seed, rest), ...);
	}
}

namespace std
{
	template <>
	struct hash<TRE::RenderObject::Vertex>
	{
		size_t operator()(TRE::RenderObject::Vertex const& vertex) const
		{
			size_t seed = 0;
			HashCombine(seed, vertex.m_Position, vertex.m_Color, vertex.m_Normal, vertex.m_UV);
			return seed;
		}
	};
}

namespace TRE
{
	void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) 
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
}

namespace TRE
{
	RenderObject::RenderObject(const Builder& _Builder)
	{
		CreateVertexBuffer(_Builder.m_Vertices);
		CreateIndexBuffer(_Builder.m_Indices);
	}

	RenderObject::~RenderObject()
	{

	}

	std::unique_ptr<RenderObject> RenderObject::CreateFromFile(const std::string& filePath)
	{
		Builder builder{};
		builder.LoadRenderObject(filePath);

		return std::make_unique<RenderObject>(builder);
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
		CopyBuffer(stagingBuffer.GetBuffer(), m_VertexBuffer->GetBuffer(), bufferSize);
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
		CopyBuffer(stagingBuffer.GetBuffer(), m_IndexBuffer->GetBuffer(), bufferSize);
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
		attributeDescriptions.push_back({ 3, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, m_UV) });

		return attributeDescriptions;
	}

	void RenderObject::Builder::LoadRenderObject(const std::string& filePath)
	{
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string warn, error;

		if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &error, filePath.c_str()))
		{
			throw std::runtime_error(warn + error);
		}

		m_Vertices.clear();
		m_Indices.clear();

		std::unordered_map<Vertex, std::uint32_t> uniqueVertices{};
		for (const auto& shape : shapes)
		{
			for (const auto& index : shape.mesh.indices)
			{
				Vertex vertex{};

				if (index.vertex_index >= 0)
				{
					vertex.m_Position = { attrib.vertices[3 * index.vertex_index + 0],attrib.vertices[3 * index.vertex_index + 1], attrib.vertices[3 * index.vertex_index + 2] };

					vertex.m_Color = { attrib.colors[3 * index.vertex_index + 0],attrib.colors[3 * index.vertex_index + 1], attrib.colors[3 * index.vertex_index + 2] };

				}

				if (index.normal_index >= 0)
				{
					vertex.m_Normal = { attrib.normals[3 * index.normal_index + 0], attrib.normals[3 * index.normal_index + 1], attrib.normals[3 * index.normal_index + 2] };
				}

				if (index.texcoord_index >= 0)
				{
					vertex.m_UV = { attrib.texcoords[2 * index.texcoord_index + 0], attrib.texcoords[2 * index.texcoord_index + 1] };
				}

				if (uniqueVertices.count(vertex) == 0)
				{
					uniqueVertices[vertex] = static_cast<std::uint32_t>(m_Vertices.size());
					m_Vertices.push_back(vertex);
				}
				m_Indices.push_back(uniqueVertices[vertex]);
			}
		}
	}
}