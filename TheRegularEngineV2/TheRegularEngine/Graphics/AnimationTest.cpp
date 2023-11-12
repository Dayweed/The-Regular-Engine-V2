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
	const VkDescriptorSet& AnimationTest::GetDescriptorSet(uint32_t Index)
	{
		return m_MaterialInstace->GetDescriptor(Index);
	}

	AnimationTest::AnimationTest(const std::shared_ptr<RenderPass>& TargetPass)
	{
		auto AnimationShader = ResourceManager::Instance().GetResource<Shader>(9);
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
		m_MaterialInstace->SetTexture("Diffuse", Texture1);
		m_MaterialInstace->SetTexture("DiffuseAO", Texture2);
		m_MaterialInstace->SetTexture("NormalMap", Texture3);
		m_MaterialInstace->SetTexture("Specular", Texture4);
		m_MaterialInstace->SetTexture("Glossiness", Texture5);

		AnimationImporter Importer;
		m_AnimationCharacter = std::make_shared<AnimationGeom>();
		if (auto ImportResult = Importer.Import(*m_AnimationCharacter, "../Assets/GirlAnimationWalkingTextures/GirlAnimationWalking.fbx"); !ImportResult)
			assert(ImportResult == true && "Failed to load animated model");

		m_VertexBuffer = std::make_unique<VertexBuffer>((void*)m_AnimationCharacter->m_SkinGeom.m_Mesh[0].m_Submeshes[0].m_Vertices.data(),
			m_AnimationCharacter->m_SkinGeom.m_Mesh[0].m_Submeshes[0].m_Vertices.size() * sizeof(TRE::vertex));

		m_IndexBuffer = std::make_unique<IndexBuffer>((void*)m_AnimationCharacter->m_SkinGeom.m_Mesh[0].m_Submeshes[0].m_Indices.data(),
			m_AnimationCharacter->m_SkinGeom.m_Mesh[0].m_Submeshes[0].m_Indices.size() * sizeof(int), m_AnimationCharacter->m_SkinGeom.m_Mesh[0].m_Submeshes[0].m_Indices.size());
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

	void AnimationTest::BindBuffers(VkCommandBuffer CmdBuffer)
	{
		VkDeviceSize offsets[] = { 0 };
		auto VB = m_VertexBuffer->GetBuffer();
		vkCmdBindVertexBuffers(CmdBuffer, 0, 1, &VB, offsets);
		vkCmdBindIndexBuffer(CmdBuffer, m_IndexBuffer->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);
	}

	void AnimationTest::Draw(VkCommandBuffer CmdBuffer)
	{
		vkCmdDrawIndexed(CmdBuffer, m_IndexBuffer->GetIndexCount(), 1, 0, 0, 0);
	}
}