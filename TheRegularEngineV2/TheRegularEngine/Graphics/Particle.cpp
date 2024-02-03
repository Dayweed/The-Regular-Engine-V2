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
		ResetParticles(emitterPos);
	}

	void ParticleComponent::UpdateParticles()
	{
		const float deltaTime = Engine::GetInstance().GetWindow()->GetDeltaTime();

		if (m_Fade)
		{
			const float fadeInCutOff = m_LifeTime * m_FadeDuration.x;
			const float fadeInIncrement = 1.f / fadeInCutOff;
			const float fadeOutCutOff = m_LifeTime * m_FadeDuration.y;
			const float fadeOutIncrement = 1.f / (m_LifeTime - fadeOutCutOff);

			if (m_ElapsedTime < fadeInCutOff)
			{
				m_Color.a += fadeInIncrement * deltaTime;
			}
			else if (m_ElapsedTime > fadeOutCutOff)
			{
				m_Color.a -= fadeOutIncrement * deltaTime;
			}
		}

		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_real_distribution<> disSpeed(m_VariationSpeed.x * m_Speed, m_VariationSpeed.y * m_Speed);
		
		const Transform& mainCameraTransform = ECSSystemManager::Instance().GetSystem<CameraSystem>()->GetMainCamera()->GetComponent<Transform>();
		for (auto& particle : m_Particles)
		{
			const glm::vec3 randomSpeed = glm::vec3(disSpeed(gen), disSpeed(gen), disSpeed(gen));
			particle.Position += randomSpeed * m_Velocity * deltaTime;

			const glm::mat4 rot = glm::mat3(glm::lookAt(particle.Position, mainCameraTransform.m_Position, glm::vec3(0.f, 1.f, 0.f)));

			particle.L2W = glm::translate(glm::mat4(1.f), particle.Position) * glm::scale(glm::mat4(1.0f), particle.Scale);

			//Billboard
			/*glm::vec3 forward = glm::normalize(mainCameraTransform.m_Position - particle.Position);
			glm::vec3 right = glm::normalize(glm::cross(glm::vec3(0.f, 1.f, 0.f), forward));
			glm::vec3 up = glm::cross(forward, right);

			glm::mat4 billboardMatrix(1.0f);
			billboardMatrix[0] = glm::vec4(right * m_Size, 0.0f);
			billboardMatrix[1] = glm::vec4(up * m_Size, 0.0f);
			billboardMatrix[2] = glm::vec4(-forward * m_Size, 0.0f);
			billboardMatrix[3] = glm::vec4(particle.Position, 1.0f);

			particle.L2W = billboardMatrix;*/
		}

		m_ElapsedTime += Engine::GetInstance().GetWindow()->GetDeltaTime();
	}

	void ParticleComponent::ResetParticles(const glm::vec3& emitterPos)
	{
		m_ElapsedTime = 0.f;
		ResetParticlesData(emitterPos);
		if (m_Fade)
			m_Color.a = 0.f;
	}

	void ParticleComponent::ResetParticlesData(const glm::vec3 emitterPos)
	{
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_real_distribution<> disPos(-1.0 * m_SpawnRadius, 1.0 * m_SpawnRadius);
		std::uniform_real_distribution<> disSize(m_VariationSize.x * m_Size, m_VariationSize.y * m_Size);
		for (int i = 0; i < m_ParticleCount; ++i)
		{
			m_Particles[i].Position = emitterPos + glm::vec3(disPos(gen), disPos(gen), disPos(gen));
			const float scale = static_cast<float>(disSize(gen));
			m_Particles[i].Scale = glm::vec3(scale, scale, scale);
		}
	}

	void ParticleSystem::LateUpdate()
	{
		for (auto& emitters : ECSManager::Instance().GetEntities<ParticleComponent>())
		{
			auto& particleComponent = emitters->GetComponent<ParticleComponent>();
			if(particleComponent.m_Running == false)
				continue;

			const auto& transform = emitters->GetComponent<Transform>();
			if (particleComponent.m_Particles.size() != particleComponent.m_ParticleCount)
			{
				particleComponent.GenerateParticles(transform.m_Position);
			}

			particleComponent.UpdateParticles();

			if (particleComponent.m_ElapsedTime >= particleComponent.m_LifeTime)
			{
				if(particleComponent.m_Loop)
					particleComponent.ResetParticles(transform.m_Position);
				else
					particleComponent.m_Running = false;
			}
		}
	}
}