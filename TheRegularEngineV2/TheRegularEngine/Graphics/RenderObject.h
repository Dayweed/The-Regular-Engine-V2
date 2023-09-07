#pragma once
#include "Buffer.h"
#include "Geom.h"

namespace TRE
{
	class RenderObject
	{
	public:
		struct Vertex
		{
			glm::vec3 m_Position{};
			glm::vec3 m_Color{};
			glm::vec3 m_Normal{};
			glm::vec2 m_UV{};

			static std::vector<VkVertexInputBindingDescription> GetBindingDescriptions();
			static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions();

			bool operator==(const Vertex& _Other) const
			{
				return m_Position == _Other.m_Position && m_Color == _Other.m_Color && m_Normal == _Other.m_Normal && m_UV == _Other.m_UV;
			}
		};

		struct Builder
		{
			std::vector<Vertex> m_Vertices{};
			std::vector<std::uint32_t> m_Indices{};
		};

		RenderObject(const Builder& builder);
		~RenderObject();

		RenderObject(RenderObject&) = delete;
		void operator=(const RenderObject&) = delete;

		static std::unique_ptr<RenderObject> CreateFromFile(const std::string& filePath);
		static std::unique_ptr<RenderObject> CreateFromGeom(std::unique_ptr<Geom> geom);
		void Bind(VkCommandBuffer commandBuffer);
		void Draw(VkCommandBuffer commandBuffer);
	private:
		void CreateVertexBuffer(const std::vector<Vertex>& vertices);
		void CreateIndexBuffer(const std::vector<std::uint32_t>& indices);
	private:
		std::unique_ptr<Buffer> m_VertexBuffer;
		std::uint32_t m_VertexCount;

		bool m_HasIndexBuffer{ false };
		std::unique_ptr<Buffer> m_IndexBuffer;
		std::uint32_t m_IndexCount;
	};
}