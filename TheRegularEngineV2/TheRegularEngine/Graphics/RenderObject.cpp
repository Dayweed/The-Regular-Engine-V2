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
		m_HasIndexBuffer = false;
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

		if (geom->m_IsAnimated)
		{
			std::vector<BoneVertex> BoneVertices(geom->nBones);
			for (uint32_t x = 0; x < geom->nBones; x++)
			{
				BoneVertices[x].m_BoneIndex = geom->pBone[x].m_BoneIndex;
				BoneVertices[x].m_BoneWeights = geom->pBone[x].m_BoneWeights;
			}
			m_BoneVertexBuffer = std::make_unique<VertexBuffer>(static_cast<void*>(BoneVertices.data()),
				UINT32_T_CAST(BoneVertices.size() * sizeof(BoneVertex)));

			m_Animations = geom->m_Animation;
			m_Skeleton = geom->m_Skeleton;
			m_AnimationPlayer = AnimationPlayer(m_Skeleton, m_Animations);

			m_IsRigged = true;
		}
		else
		{
			m_IsRigged = false;
		}

		m_VertexBuffer = std::make_unique<VertexBuffer>(static_cast<void*>(vertices.data()), UINT32_T_CAST(vertices.size() * sizeof(Vertex)));

		if (!indices.empty())
		{
			m_IndexBuffer = std::make_unique<IndexBuffer>(static_cast<void*>(indices.data()),
				UINT32_T_CAST(indices.size() * sizeof(uint32_t)),
				UINT32_T_CAST(indices.size()));
			m_HasIndexBuffer = true;
		}

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

	void RenderObject::BindAnimation(VkCommandBuffer commandBuffer)
	{
		VkBuffer vertexBuffers[] = { m_VertexBuffer->GetBuffer() };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

		VkBuffer bonevertexBuffers[] = { m_BoneVertexBuffer->GetBuffer() };
		VkDeviceSize offsets2[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 1, 1, bonevertexBuffers, offsets2);

		vkCmdBindIndexBuffer(commandBuffer, m_IndexBuffer->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);

		vkCmdDrawIndexed(commandBuffer, m_IndexBuffer->GetIndexCount(), 1, 0, 0, 0);
	}

	void RenderObject::Draw(VkCommandBuffer commandBuffer)
	{
		if (m_HasIndexBuffer)
		{
			vkCmdDrawIndexed(commandBuffer, m_IndexBuffer->GetIndexCount(), 1, 0, 0, 0);
		}
		else
		{
			vkCmdDraw(commandBuffer, m_VertexCount, 1, 0, 0);
		}
	}

	void RenderObject::CreateBoundingSphere(const std::vector<Vertex>& vertices)
	{
		std::vector<glm::vec3> positions(vertices.size());
		for (unsigned i = 0; i < vertices.size(); ++i)
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

	void RenderObject::UpdateAnimation(std::span<glm::mat4> FinalL2W, const glm::mat4& L2W)
	{
		if (m_AnimationPlayer.m_Animations.size() != 0)
		{
			m_AnimationPlayer.Update(Engine::GetInstance().GetWindow()->GetDeltaTime());
			m_AnimationPlayer.ComputeMatrices(FinalL2W, L2W);
		}
	}

	void AnimationPlayer::Update(float DT)
	{
		auto& Anim = m_Animations[m_iCurAnim];
		// advance time
		m_Time += DT;
		while (m_Time >= Anim.m_TimeLength) m_Time -= Anim.m_TimeLength;
	}
	void AnimationPlayer::ComputeMatrices(std::span<glm::mat4> FinalL2W, const glm::mat4& L2W) const
	{
		auto& Anim = m_Animations[m_iCurAnim];
		const float FrameTime = m_Time * Anim.m_FPS;
		const int   iFrameT0 = static_cast<int>(FrameTime);
		const int   iFrameT1 = static_cast<int>((iFrameT0 + 1) % Anim.m_BoneKeyFrames[0].m_Scale.size());
		// compute hierarchy matrices
		for (int i = 0; i < Anim.m_BoneKeyFrames.size(); ++i)
		{
			auto& KeyFrame = Anim.m_BoneKeyFrames[i];
			glm::vec3 BlendedScale = KeyFrame.m_Scale[iFrameT0] + ((FrameTime - iFrameT0) * (KeyFrame.m_Scale[iFrameT1] - KeyFrame.m_Scale[iFrameT0]));
			glm::quat BlendedRotate = vkUtils::Blend(KeyFrame.m_Rotate[iFrameT0], (FrameTime - iFrameT0), KeyFrame.m_Rotate[iFrameT1]);
			glm::vec3 BlendedTranslate = KeyFrame.m_Translate[iFrameT0] + ((FrameTime - iFrameT0) * (KeyFrame.m_Translate[iFrameT1] - KeyFrame.m_Translate[iFrameT0]));
			glm::mat4 Scale(1.f); glm::mat4 Translate(1.f);
			Scale = glm::scale(Scale, BlendedScale);
			glm::mat4 Rotate = glm::mat4_cast(BlendedRotate);
			Translate = glm::translate(Translate, BlendedTranslate);
			FinalL2W[i] = Translate * Rotate * Scale;
			if (-1 != m_Skeleton.m_Bones[i].m_iParent)
			{
				assert(m_Skeleton.m_Bones[i].m_iParent <= i);
				FinalL2W[i] = FinalL2W[m_Skeleton.m_Bones[i].m_iParent] * FinalL2W[i];
			}
			else
			{
				assert(i == 0);
				FinalL2W[i] = L2W * FinalL2W[i];
			}
		}
		// add the bind matrices into the final hierarchy
		for (int i = 0; i < Anim.m_BoneKeyFrames.size(); ++i)
		{
			FinalL2W[i] *= m_Skeleton.m_Bones[i].m_InvBind;
		}
	}
}