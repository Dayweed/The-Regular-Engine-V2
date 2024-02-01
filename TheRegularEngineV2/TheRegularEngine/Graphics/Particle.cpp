#include "pch.h"
#include "Particle.h"
#include "Random.h"
#include "Core/Transform.h"
#include "Core/Engine.h"

namespace TRE
{
	void ParticleComponent::GenerateParticles(const glm::vec3& emitterPos)
	{
		m_Particles.clear();
		m_Particles.resize(m_ParticleCount);
		std::random_device rd;
		std::mt19937 gen(rd());	
		const float multipler = 30.f;
		std::uniform_real_distribution<> dis(-1.0 * multipler, 1.0 * multipler);
		for (int i = 0; i < m_ParticleCount; ++i)
		{
			const glm::vec3 randomVec = glm::vec3(dis(gen), dis(gen), dis(gen));
			Particle& particle = m_Particles[i];
			particle.Position = emitterPos + randomVec;
			particle.Velocity = randomVec * m_Speed;
			particle.LifeTime = m_LifeTime;
		}
	}

	void ParticleComponent::UpdateParticle()
	{
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_real_distribution<> dis(m_Variation.x * m_Speed, m_Variation.y * m_Speed);
		for (auto& particle : m_Particles)
		{
			const float deltaTime = Engine::GetInstance().GetWindow()->GetDeltaTime();
			particle.LifeTime -= deltaTime;
			particle.Position += glm::vec3(dis(gen), dis(gen), dis(gen)) * m_Velocity * deltaTime;

			auto scale = glm::scale(glm::mat4(1.f), glm::vec3(100.f, 100.f, 0.f));
			auto translate = glm::translate(glm::mat4(1.f), particle.Position);
			particle.L2W = translate * scale;
		}
	}

	void ParticleSystem::LateUpdate()
	{
		for (auto& emitters : ECSManager::Instance().GetEntities<ParticleComponent>())
		{
			auto& particleComponent = emitters->GetComponent<ParticleComponent>();
			const auto& transform = emitters->GetComponent<Transform>();
			if (particleComponent.m_Particles.size() != particleComponent.m_ParticleCount)
			{
				particleComponent.GenerateParticles(transform.m_Position);
			}

			particleComponent.UpdateParticle();
		}
	}
}