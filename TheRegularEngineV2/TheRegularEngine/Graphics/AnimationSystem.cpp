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
		for (auto& Entity : ECSManager::Instance().GetEntities<MeshRenderer, AnimationComponent>())
		{
			auto& MRComp = Entity->GetComponent<MeshRenderer>();
			auto& AnimComp = Entity->GetComponent<AnimationComponent>();
			if (AnimComp.InitialSet)
				continue;

			AnimComp.m_FPS = MRComp.m_RenderObject->GetAnimation().m_FPS;
			AnimComp.InitialFPS= MRComp.m_RenderObject->GetAnimation().m_FPS;
			AnimComp.InitialSet = true;
			//AnimComp.m_AnimationSpeed = MRComp.m_RenderObject->GetAnimation().m_TimeLength;
		}
	}

	void AnimationSystem::GameUpdate()
	{

	}

	void AnimationSystem::LateUpdate()
	{

	}
	
	void AnimationSystem::BeforeReset()
	{

	}

	void AnimationSystem::AfterReset()
	{

	}
}