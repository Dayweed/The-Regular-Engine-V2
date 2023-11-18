#include "pch.h"
#include "AnimationSystem.h"
#include "Core/ECS.h"

#include "AnimationComponent.h"
#include "Core/Transform.h"
#include "Graphics/MeshRenderer.h"

namespace TRE
{
	void AnimationSystem::Init()
	{

	}

	void AnimationSystem::Update()
	{

	}

	void AnimationSystem::GameUpdate()
	{

	}

	void AnimationSystem::LateUpdate()
	{
		for (auto& Entity : ECSManager::Instance().GetEntities<AnimationComponent>())
		{
			const auto& TransformComp = Entity->GetComponent<Transform>();
			auto& MRComp = Entity->GetComponent<MeshRenderer>();
			auto& AnimComp = Entity->GetComponent<AnimationComponent>();
			if (!MRComp.m_IsVisible)
				continue;

			//MRComp.m_RenderObject->UpdateAnimation(AnimComp.m_BufferData.L2W, glm::identity<glm::mat4>());
			//AnimComp.m_BufferData.ProjView = editorCamera.GetViewProjectionMatrix();
			//AnimComp.m_UBO->SetData(&AnimComp.m_BufferData, sizeof(AnimationUBO));
		}
	}
	
	void AnimationSystem::BeforeReset()
	{

	}

	void AnimationSystem::AfterReset()
	{

	}
}