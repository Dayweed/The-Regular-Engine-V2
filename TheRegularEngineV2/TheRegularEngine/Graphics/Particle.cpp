#include "pch.h"
#include "Particle.h"
#include "Random.h"
#include "Core/Transform.h"
#include "Core/Engine.h"
#include "Camera.h"

namespace TRE
{
	void ParticleComponent::GenerateParticles(const glm::vec3& emitterPos)
	{
		m_Particles.clear();
		m_Particles.resize(m_ParticleCount);
		ResetParticlesPosition(emitterPos);
	}

	void ParticleComponent::UpdateParticles()
	{
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_real_distribution<> disSpeed(m_VariationSpeed.x * m_Speed, m_VariationSpeed.y * m_Speed);
		std::uniform_real_distribution<> disSize(m_VariationSize.x * m_Size, m_VariationSize.y * m_Size);
		const float deltaTime = Engine::GetInstance().GetWindow()->GetDeltaTime();
		const Transform& mainCameraTransform = ECSSystemManager::Instance().GetSystem<CameraSystem>()->GetMainCamera()->GetComponent<Transform>();
		for (auto& particle : m_Particles)
		{
			const glm::vec3 randomSpeed = glm::vec3(disSpeed(gen), disSpeed(gen), disSpeed(gen));
			particle.Position += randomSpeed * m_Velocity * deltaTime;
			
			const glm::vec3 randomSize = glm::vec3(disSize(gen), disSize(gen), disSize(gen));

			glm::vec3 forward = glm::normalize(mainCameraTransform.m_Position - particle.Position);
			glm::vec3 right = glm::normalize(glm::cross(glm::vec3(0.f, 1.f, 0.f), forward));
			glm::vec3 up = glm::cross(forward, right);

			glm::mat4 billboardMatrix(1.0f);
			billboardMatrix[0] = glm::vec4(right * m_Size, 0.0f);
			billboardMatrix[1] = glm::vec4(up * m_Size, 0.0f);
			billboardMatrix[2] = glm::vec4(-forward * m_Size, 0.0f);
			billboardMatrix[3] = glm::vec4(particle.Position, 1.0f);

			particle.L2W = billboardMatrix;
		}

		m_ElapsedTime += Engine::GetInstance().GetWindow()->GetDeltaTime();
	}

	void ParticleComponent::ResetParticles(const glm::vec3& emitterPos)
	{
		m_ElapsedTime = 0.f;
		ResetParticlesPosition(emitterPos);
	}

	void ParticleComponent::ResetParticlesPosition(const glm::vec3 emitterPos)
	{
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_real_distribution<> dis(-1.0 * m_SpawnRadius, 1.0 * m_SpawnRadius);
		for (int i = 0; i < m_ParticleCount; ++i)
		{
			m_Particles[i].Position = emitterPos + glm::vec3(dis(gen), dis(gen), dis(gen));
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

			particleComponent.UpdateParticles();

			if (particleComponent.m_ElapsedTime >= particleComponent.m_LifeTime)
			{
				particleComponent.ResetParticles(transform.m_Position);
			}
		}
	}
}