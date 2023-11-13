#pragma once
#include "Geom.h"
#include "Sphere3D.h"
#include "Resource/Resource.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"

namespace TRE
{
	class RenderObject : public Resource
	{
	public:
		struct Vertex
		{
			glm::vec3 m_Position{};
			glm::vec3 m_Normal{};
			glm::vec3 m_Tangent{};
			glm::vec3 m_Bitangent{};
			glm::vec3 m_Color{};
			glm::vec2 m_UV{};

			static std::vector<VkVertexInputBindingDescription> GetBindingDescriptions();
			static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions();

			bool operator==(const Vertex& _Other) const
			{
				return m_Position == _Other.m_Position && m_Color == _Other.m_Color && m_Normal == _Other.m_Normal && m_UV == _Other.m_UV;
			}
		};

		struct BoneVertex
		{
			glm::vec4         m_BoneWeights{};
			glm::uvec4        m_BoneIndex{};
		};

		RenderObject(const std::string& geomAsset);
		~RenderObject();

		const Collision::Sphere3D& GetBoundingSphere() const { return m_BoundingSphere; }

		void Bind(VkCommandBuffer commandBuffer);
		void Draw(VkCommandBuffer commandBuffer);

		static ResourceType GetType() { return ResourceType::Mesh; }

		void Serialize() override;
		static std::shared_ptr<RenderObject> Deserialize(const std::string& assetHexGUID);

	private:
		void CreateBoundingSphere(const std::vector<Vertex>& vertices);

		RenderObject(RenderObject&) = delete;
		void operator=(const RenderObject&) = delete;
	private:
		std::unique_ptr<VertexBuffer> m_VertexBuffer;
		std::uint32_t m_VertexCount;

		std::unique_ptr<VertexBuffer> m_BoneVertexBuffer;

		//AnimationPlayer m_AnimationPlayer;
		std::vector<Animation> m_Animations;
		Skeleton m_Skeleton;

		bool m_HasIndexBuffer{ false };
		std::unique_ptr<IndexBuffer> m_IndexBuffer;

		//Ill change this next time to account for non uniform scaling
		Collision::Sphere3D m_BoundingSphere; //Local space bounding sphere
	};
}