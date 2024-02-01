#pragma once
#include "Core/ECS.h"
#include "Resource/ResourceManager.h"

namespace TRE
{
	struct Particle
	{
		glm::mat4 L2W;
		glm::vec3 Position;
		glm::vec3 Velocity;
		float LifeTime;
	};

	class ParticleComponent : property::base
	{
	public:
		std::vector<Particle> m_Particles;
		glm::vec3 m_Velocity = glm::vec3(0.f, 1.f, 0.f);
		glm::vec2 m_Variation = glm::vec2(0.8f, 1.f);
		float m_Speed = 1.f;
		float m_LifeTime = 1.f;
		int m_ParticleCount = 100;
		bool m_IsVisible = true;

	public:
		void GenerateParticles(const glm::vec3& emitterPos);
		void UpdateParticle();

		property_vtable()

		friend void to_json(nlohmann::json& j, const ParticleComponent& t)
		{
			j = nlohmann::json
			{
				{ "m_IsVisible", t.m_IsVisible },
			};
		}

		friend void from_json(const nlohmann::json& j, ParticleComponent& t)
		{
			if (j.contains("m_IsVisible"))
			{
				t.m_IsVisible = j.at("m_IsVisible").get<bool>();
			}
		}
	};

	class ParticleSystem : public ECSSystem
	{
		void LateUpdate() override;
	};
}

property_begin(TRE::ParticleComponent)
{
	property_var(m_IsVisible),

} property_vend_h(TRE::ParticleComponent)