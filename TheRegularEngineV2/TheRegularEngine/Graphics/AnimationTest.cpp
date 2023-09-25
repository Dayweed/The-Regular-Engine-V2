#include "pch.h"
#include "AnimationTest.h"
#include "Resource/ResourceManager.h"
#include "AnimationImporter.h"

namespace TRE
{
	AnimationTest::AnimationTest(const std::shared_ptr<RenderPass>& TargetPass)
	{
		auto AnimationVertShader = ResourceManager::Instance().GetResource<Shader>(5);
		auto AnimationFragShader = ResourceManager::Instance().GetResource<Shader>(6);

		PipelineConfigurations PipelineConfig{};
		PipelineConfig.Primitive = PrimitiveType::Triangles;
		PipelineConfig.VertexShader = AnimationVertShader;
		PipelineConfig.FragmentShader = AnimationFragShader;

		m_AnimationPipeline = std::make_unique<Pipeline>(PipelineConfig, TargetPass);

		AnimationImporter Importer;
		m_AnimationCharacter = std::make_unique<AnimationGeom>();
		auto ImportResult = Importer.Import("../Assets/GirlAnimationWalkingTextures/GirlAnimationWalking.fbx", &m_AnimationCharacter->m_SkinGeom, &m_AnimationCharacter->m_Skeleton, &m_AnimationCharacter->m_AnimPackage);
		
		if (ImportResult == false)
		{
			assert(ImportResult == true && "Failed to load animated model");
		}


	}

	AnimationTest::~AnimationTest()
	{

	}

	void AnimationTest::BindPipeline(VkCommandBuffer CmdBuffer)
	{
		vkCmdBindPipeline(CmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_AnimationPipeline->GetPipeline());
	}
}