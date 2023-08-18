#pragma once
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE //Forces depth values to be [0,1] instead of openGL [-1,1]
#include "pch.h"
#include "Buffer.h"

namespace TRE
{
	class RenderObject
	{
	public:
		struct Vertex
		{
			glm::vec4 m_Position{};
			glm::vec4 m_Color{};
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

			//void LoadModel(const std::string& filePath);
		};

		RenderObject(const Builder& builder);
		~RenderObject();

		RenderObject(RenderObject&) = delete;
		void operator=(const RenderObject&) = delete;

		//static std::unique_ptr<RenderObject> CreateModelFromFile(const std::string& filePath);
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