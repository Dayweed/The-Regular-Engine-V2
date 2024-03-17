#include "pch.h"
#include "ECS/Components/Particle2DComponent.h"
#include "ECS/Components/Particle3DComponent.h"
#include "Random.h"
#include "ECS/Components/Transform.h"
#include "Core/Engine.h"
#include "Camera.h"
#include "ECS/Components/CameraComponent.h"
#include "ParticleSystem.h"

namespace TRE
{
	void Particle2DComponent::GenerateParticles(const glm::vec3& emitterPos)
	{
		m_Particles.clear();
		m_Particles.resize(m_ParticleCount);
		ResetParticles(emitterPos);
	}

	void Particle2DComponent::UpdateParticles(const bool is3D)
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
		
		// const Transform& mainCameraTransform = ECSSystemManager::Instance().GetSystem<CameraSystem>()->GetMainCamera()->GetComponent<Transform>();
		const BaseCamera& camera = ECSSystemManager::Instance().GetSystem<CameraSystem>()->GetMainCamera()->GetComponent<Camera>().m_BaseCamera;
		const auto mainCamPos = camera.m_FocalPoint - camera.GetViewDirection() * camera.m_FocalLength;
		for (auto& particle : m_Particles)
		{
			const glm::vec3 randomSpeed = glm::vec3(disSpeed(gen), disSpeed(gen), disSpeed(gen));
			particle.Position += randomSpeed * m_Velocity * deltaTime;

			if (is3D)
			{
				//Billboard
				glm::vec3 forward = glm::normalize(mainCamPos - particle.Position);
				glm::vec3 right = glm::normalize(glm::cross(glm::vec3(0.f, 1.f, 0.f), forward));
				glm::vec3 up = glm::cross(forward, right);

				glm::mat4 billboardMatrix(1.0f);
				billboardMatrix[0] = glm::vec4(right, 0.0f);
				billboardMatrix[1] = glm::vec4(up, 0.0f);
				billboardMatrix[2] = glm::vec4(-forward, 0.0f);
				particle.L2W = glm::translate(glm::mat4(1.f), particle.Position) * billboardMatrix * glm::scale(glm::mat4(1.0f), particle.Scale);
			}
			else
			{
				const glm::vec3 postiion2D = glm::vec3(particle.Position.x, particle.Position.y, 0.f);
				particle.L2W = glm::translate(glm::mat4(1.f), postiion2D) * glm::scale(glm::mat4(1.0f), particle.Scale);
			}
		}

		m_ElapsedTime += deltaTime;
	}

	void Particle2DComponent::ResetParticles(const glm::vec3& emitterPos)
	{
		if(m_Particles.size() != m_ParticleCount)
			GenerateParticles(emitterPos);
		m_ElapsedTime = 0.f;
		ResetParticlesData(emitterPos);
		if (m_Fade)
			m_Color.a = 0.f;
	}

	void Particle2DComponent::ResetParticlesData(const glm::vec3 emitterPos)
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
		for (auto& emitters : ECSManager::Instance().GetEntities<Particle2DComponent>())
		{
			auto& particleComponent = emitters->GetComponent<Particle2DComponent>();
			const auto& transform = emitters->GetComponent<Transform>();

			if (particleComponent.m_Particles.size() != particleComponent.m_ParticleCount)
			{
				particleComponent.GenerateParticles(transform.m_Position);
			}

			if(particleComponent.m_Running == false)
				continue;

			particleComponent.UpdateParticles(particleComponent.m_3DWorld);

			if (particleComponent.m_ElapsedTime >= particleComponent.m_LifeTime)
			{
				if(particleComponent.m_Loop)
					particleComponent.ResetParticles(transform.m_Position);
				else
					particleComponent.m_Running = false;
			}
		}

		for (auto& emitters : ECSManager::Instance().GetEntities<Particle3DComponent>())
		{
			auto& particleComponent = emitters->GetComponent<Particle3DComponent>();
			
			if (particleComponent.m_Mesh == nullptr)
				continue;

			const auto& transform = emitters->GetComponent<Transform>();

			if (particleComponent.m_Position.size() != particleComponent.m_ParticleCount)
			{
				particleComponent.GenerateParticles(transform.m_Position);
			}

			if (particleComponent.m_Running == false)
				continue;

			particleComponent.UpdateParticles();

			if (particleComponent.m_ElapsedTime >= particleComponent.m_LifeTime)
			{
				if (particleComponent.m_Loop)
					particleComponent.ResetParticles(transform.m_Position);
				else
					particleComponent.m_Running = false;
			}
		}
	}

	//3D
	void Particle3DComponent::GenerateParticles(const glm::vec3& emitterPos)
	{
		m_Position.clear();
		m_Scale.clear();

		m_Position.resize(m_ParticleCount);
		m_Scale.resize(m_ParticleCount);

		ResetParticles(emitterPos);
	}

	void Particle3DComponent::UpdateParticles()
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

		// const Transform& mainCameraTransform = ECSSystemManager::Instance().GetSystem<CameraSystem>()->GetMainCamera()->GetComponent<Transform>();
		const BaseCamera& camera = ECSSystemManager::Instance().GetSystem<CameraSystem>()->GetMainCamera()->GetComponent<Camera>().m_BaseCamera;
		const auto mainCamPos = camera.m_FocalPoint - camera.GetViewDirection() * camera.m_FocalLength;

		for (int x = 0; x < m_ParticleCount; x++)
		{
			const glm::vec3 randomSpeed = glm::vec3(disSpeed(gen), disSpeed(gen), disSpeed(gen));
			m_Position[x] += randomSpeed * m_Velocity * deltaTime;

			//Billboard
			glm::vec3 forward = glm::normalize(mainCamPos - m_Position[x]);
			glm::vec3 right = glm::normalize(glm::cross(glm::vec3(0.f, 1.f, 0.f), forward));
			glm::vec3 up = glm::cross(forward, right);

			glm::mat4 billboardMatrix(1.0f);
			billboardMatrix[0] = glm::vec4(right, 0.0f);
			billboardMatrix[1] = glm::vec4(up, 0.0f);
			billboardMatrix[2] = glm::vec4(-forward, 0.0f);
			m_ParticleData.L2W[x] = glm::translate(glm::mat4(1.f), m_Position[x]) * billboardMatrix * glm::scale(glm::mat4(1.0f), m_Scale[x]);
		}

		m_ElapsedTime += deltaTime;
	}

	void Particle3DComponent::ResetParticles(const glm::vec3& emitterPos)
	{
		if (m_Position.size() != m_ParticleCount || m_Scale.size() != m_ParticleCount)
			GenerateParticles(emitterPos);
		m_ElapsedTime = 0.f;
		ResetParticlesData(emitterPos);
		if (m_Fade)
			m_Color.a = 0.f;
	}

	void Particle3DComponent::ResetParticlesData(const glm::vec3 emitterPos)
	{
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_real_distribution<> disPos(-1.0 * m_SpawnRadius, 1.0 * m_SpawnRadius);
		std::uniform_real_distribution<> disSize(m_VariationSize.x * m_Size, m_VariationSize.y * m_Size);
		for (int i = 0; i < m_ParticleCount; ++i)
		{
			m_Position[i]  = emitterPos + glm::vec3(disPos(gen), disPos(gen), disPos(gen));
			const float scale = static_cast<float>(disSize(gen));
			m_Scale[i] = glm::vec3(scale, scale, scale);
		}
	}
}