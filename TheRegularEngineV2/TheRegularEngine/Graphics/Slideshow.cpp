#include "pch.h"
#include "ECS/Components/Slideshow.h"
#include "SwapChain.h"
#include "Core/Engine.h"
#include "RendererContext.h"
#include "ECS/Components//Sprite3DComponent.h"

namespace TRE
{
	SlideshowComponent::SlideshowComponent()
	{

	}

	void SlideshowComponent::GenerateVertexBuffer()
	{
		auto tempVertexBuffers = std::vector<std::shared_ptr<VertexBuffer>>();
		const float increment = 1.f / m_Slices;
		for (int i = 0; i < m_Slices; i++)
		{
			float x = -1.f; float y = -1.f;
			float width = 2, height = 2;
			std::vector<QuadVertex> data(4);

			data[0].Position = glm::vec3(x, y, 0.0f);
			data[0].TexCoord = glm::vec2(0.f / m_Slices + increment * i, 0.f);

			data[1].Position = glm::vec3(x + width, y, 0.0f);
			data[1].TexCoord = glm::vec2(1.f / m_Slices + increment * i, 0.f);

			data[2].Position = glm::vec3(x + width, y + height, 0.0f);
			data[2].TexCoord = glm::vec2(1.f / m_Slices + increment * i, 1.f);

			data[3].Position = glm::vec3(x, y + height, 0.0f);
			data[3].TexCoord = glm::vec2(0.f / m_Slices + increment * i, 1.f);

			auto vertexBuffer = std::make_shared<VertexBuffer>(static_cast<void*>(data.data()),
				UINT32_T_CAST(data.size() * sizeof(QuadVertex)));

			tempVertexBuffers.push_back(vertexBuffer);
		}
		m_VertexBuffers = tempVertexBuffers;
	}

	std::shared_ptr<VertexBuffer> SlideshowComponent::GetCurrentVertexBuffer()
	{
		return m_VertexBuffers[m_CurrentSlice];
	}

	int SlideshowComponent::VertexContainerSize()
	{
		return (int)m_VertexBuffers.size();
	}

	void SlideshowSystem::LateUpdate()
	{
		for (const auto& entity : ECSManager::Instance().GetEntities<SlideshowComponent>())
		{
			auto& slideshowComp = entity->GetComponent<SlideshowComponent>();

			const auto animationSpeed = slideshowComp.m_AnimationDuration / slideshowComp.m_Slices;

			if(slideshowComp.VertexContainerSize() != slideshowComp.m_Slices)
				slideshowComp.GenerateVertexBuffer();

			if (slideshowComp.m_ElapsedTime >= animationSpeed)
			{
				++slideshowComp.m_CurrentSlice;
				if (slideshowComp.m_CurrentSlice >= slideshowComp.m_Slices)
				{
					slideshowComp.m_CurrentSlice = 0;
				}
				slideshowComp.m_ElapsedTime = 0.f;
			}

			slideshowComp.m_ElapsedTime += Engine::GetInstance().GetWindow()->GetDeltaTime();
		}

		for (const auto& entity : ECSManager::Instance().GetEntities<Sprite3DComponent>())
		{
			auto& spritecomp = entity->GetComponent<Sprite3DComponent>();
			if (!spritecomp.m_IsSpriteSheet) continue; //Ignore if its not spritesheet

			const auto animationSpeed = spritecomp.m_AnimationDuration / spritecomp.m_Slices;

			if (spritecomp.m_VertexBuffers.size() != spritecomp.m_Slices)
				spritecomp.GenerateVertexBuffer();

			if (spritecomp.m_ElapsedTime >= animationSpeed)
			{
				++spritecomp.m_CurrentSlice;
				if (spritecomp.m_CurrentSlice >= spritecomp.m_Slices)
				{
					spritecomp.m_CurrentSlice = 0;
				}
				spritecomp.m_ElapsedTime = 0.f;
			}

			spritecomp.m_ElapsedTime += Engine::GetInstance().GetWindow()->GetDeltaTime();
		}
	}

	void Sprite3DComponent::GenerateVertexBuffer()
	{
		auto tempVertexBuffers = std::vector<std::shared_ptr<VertexBuffer>>();
		const float increment = 1.f / m_Slices;
		for (int i = 0; i < m_Slices; i++)
		{
			float x = -1.f; float y = -1.f;
			float width = 2, height = 2;
			std::vector<QuadVertex> data(4);

			data[0].Position = glm::vec3(x, y, 0.0f);
			data[0].TexCoord = glm::vec2(0.f / m_Slices + increment * i, 0.f);

			data[1].Position = glm::vec3(x + width, y, 0.0f);
			data[1].TexCoord = glm::vec2(1.f / m_Slices + increment * i, 0.f);

			data[2].Position = glm::vec3(x + width, y + height, 0.0f);
			data[2].TexCoord = glm::vec2(1.f / m_Slices + increment * i, 1.f);

			data[3].Position = glm::vec3(x, y + height, 0.0f);
			data[3].TexCoord = glm::vec2(0.f / m_Slices + increment * i, 1.f);

			auto vertexBuffer = std::make_shared<VertexBuffer>(static_cast<void*>(data.data()), UINT32_T_CAST(data.size() * sizeof(QuadVertex)));

			tempVertexBuffers.push_back(vertexBuffer);
		}
		m_VertexBuffers = tempVertexBuffers;
	}
}