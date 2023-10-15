#include "pch.h"
#include "AnimationTest.h"
#include "Resource/ResourceManager.h"
#include "AnimationImporter.h"
#include "RendererContext.h"
#include "Renderer.h"
#include "Core/Engine.h"
#include "VulkanUtilities.h"

namespace TRE
{
	VkPipelineLayout AnimationTest::GetPipelineLayout()
	{
		return m_AnimationPipeline->GetPipelineLayout();
	}

	const VkDescriptorSet& AnimationTest::GetDescriptorSet(uint32_t Index)
	{
		return m_MaterialInstace->GetDescriptor(Index);
	}

	AnimationTest::AnimationTest(const std::shared_ptr<RenderPass>& TargetPass)
	{
		auto AnimationShader = ResourceManager::Instance().GetResource<Shader>(5);
		auto AnimationtextureHandle1 = Resource::GetGUIDFromHex("9c6509635ee2d750");
		auto AnimationtextureHandle2 = Resource::GetGUIDFromHex("52ba56f854e86f56");
		auto AnimationtextureHandle3 = Resource::GetGUIDFromHex("547865c1f61ef1f9");
		auto AnimationtextureHandle4 = Resource::GetGUIDFromHex("c076cd64a7491d7");
		auto AnimationtextureHandle5 = Resource::GetGUIDFromHex("6b2822ce3972f53");
		auto Texture1 = ResourceManager::Instance().GetResource<VulkanTexture>(AnimationtextureHandle1);
		auto Texture2 = ResourceManager::Instance().GetResource<VulkanTexture>(AnimationtextureHandle2);
		auto Texture3 = ResourceManager::Instance().GetResource<VulkanTexture>(AnimationtextureHandle3);
		auto Texture4 = ResourceManager::Instance().GetResource<VulkanTexture>(AnimationtextureHandle4);
		auto Texture5 = ResourceManager::Instance().GetResource<VulkanTexture>(AnimationtextureHandle5);

		m_MaterialInstace = std::make_unique<Material>(AnimationShader);
		m_MaterialInstace->Invalidate();
		m_MaterialInstace->SetTextures(Texture1);
		m_MaterialInstace->SetTextures(Texture2);
		m_MaterialInstace->SetTextures(Texture3);
		m_MaterialInstace->SetTextures(Texture4);
		m_MaterialInstace->SetTextures(Texture5);

		PipelineConfigurations PipelineConfig{};
		PipelineConfig.Primitive = PrimitiveType::Triangles;
		PipelineConfig.Shader = AnimationShader;
		PipelineConfig.VertexStride = sizeof(vertex);
		m_AnimationPipeline = std::make_unique<Pipeline>(PipelineConfig, TargetPass);

		AnimationImporter Importer;
		m_AnimationCharacter = std::make_unique<AnimationGeom>();
		auto ImportResult = Importer.Import(*m_AnimationCharacter, "../Assets/GirlAnimationWalkingTextures/GirlAnimationWalking.fbx");
		
		if (ImportResult == false)
		{
			assert(ImportResult == true && "Failed to load animated model");
		}

		CreateVertexBuffer(m_AnimationCharacter->m_SkinGeom.m_Mesh[0].m_Submeshes[0].m_Vertices);
		CreateIndexBuffer(m_AnimationCharacter->m_SkinGeom.m_Mesh[0].m_Submeshes[0].m_Indices);
	}

	void AnimationTest::CreateVertexBuffer(const std::vector<vertex>& vertices)
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

	void AnimationTest::CreateIndexBuffer(const std::vector<int>& indices)
	{
		m_IndexCount = static_cast<std::uint32_t>(indices.size());

		uint32_t indexSize = sizeof(indices[0]);
		Buffer stagingBuffer(indexSize, m_IndexCount, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		stagingBuffer.Map();
		stagingBuffer.WriteToBuffer((void*)indices.data());

		m_IndexBuffer = std::make_unique<Buffer>(indexSize, m_IndexCount, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		VkDeviceSize bufferSize = indexSize * m_IndexCount;
		vkUtils::CopyBuffer(stagingBuffer.GetBuffer(), m_IndexBuffer->GetBuffer(), bufferSize);
	}

	AnimationTest::~AnimationTest()
	{

	}

	void AnimationTest::UpdateAnimations(AnimationUBO& UBO, glm::mat4 L2W)
	{
		m_AnimationCharacter->m_AnimPlayer.Update(Engine::GetInstance().GetWindow()->GetDeltaTime());
		m_AnimationCharacter->m_AnimPlayer.ComputeMatrices(UBO.L2W, L2W);
	}
	
	void AnimationTest::UpdateMaterial(const std::shared_ptr<UniformBuffer>& UBO, uint32_t Index)
	{
		m_MaterialInstace->UpdateForRendering(UBO, Index);
	}

	void AnimationTest::BindPipeline(VkCommandBuffer CmdBuffer)
	{
		vkCmdBindPipeline(CmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_AnimationPipeline->GetPipeline());
	}

	void AnimationTest::BindBuffers(VkCommandBuffer CmdBuffer)
	{
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(CmdBuffer, 0, 1, &m_VertexBuffer->GetBuffer(), offsets);
		vkCmdBindIndexBuffer(CmdBuffer, m_IndexBuffer->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);
	}

	void AnimationTest::Draw(VkCommandBuffer CmdBuffer)
	{
		vkCmdDrawIndexed(CmdBuffer, m_IndexCount, 1, 0, 0, 0);
	}
}