#pragma once
#include "Core/ECS.h"
#include "Resource/ResourceManager.h"
#include "Material.h"

namespace TRE
{
	struct Particle
	{
		glm::mat4 L2W;
		glm::vec3 Position;
	};

	class ParticleComponent : property::base
	{
	public:
		ParticleComponent()
		{
			m_Material = std::make_shared<Material>(ResourceManager::Instance().GetResource<Shader>(12));
			m_Material->Invalidate();
		}
		std::shared_ptr<Material> m_Material;
		std::shared_ptr<VulkanTexture> m_Texture;
		glm::vec3 m_Velocity = glm::vec3(0.f, 1.f, 0.f);
		glm::vec2 m_VariationSpeed = glm::vec2(0.05f, 1.f);	//Variation in the speed of the particles
		glm::vec2 m_VariationSize = glm::vec2(0.2f, 1.f);	//Variation in the size of the particles
		float m_SpawnRadius = 1.f;
		float m_Speed = 5.f;
		float m_LifeTime = 10.f;
		float m_ElapsedTime = 0.f;
		float m_Size = 1.f;
		int m_ParticleCount = 2;
		bool m_Running = true;
		bool m_Loop = true;
		bool m_PlayOnStart = true;

		std::vector<Particle> m_Particles;
	public:
		void GenerateParticles(const glm::vec3& emitterPos);
		void UpdateParticles();
		void ResetParticles(const glm::vec3& emitterPos);

		property_vtable()

		friend void to_json(nlohmann::json& j, const ParticleComponent& t)
		{
			static_cast<void>(t); // because t is currently unused
			j = nlohmann::json
			{
				//{ "m_IsVisible", t.m_IsVisible },
			};
		}

		friend void from_json(const nlohmann::json& j, ParticleComponent& t)
		{
			static_cast<void>(t); // because t is currently unused
			if (j.contains("m_IsVisible"))
			{
				//t.m_IsVisible = j.at("m_IsVisible").get<bool>();
			}
		}
	private:
		void ResetParticlesPosition(const glm::vec3 emitterPos);
	};

	class ParticleSystem : public ECSSystem
	{
		void LateUpdate() override;
	};
}

property_begin(TRE::ParticleComponent)
{
	property_var_fnbegin("Texture", resource_list)
	{
		InOut.m_Type = "TEXTURE";
		if (isRead)
		{
			if (Self.m_Texture)
				InOut.m_Value = Self.m_Texture->GetHandle();
			else
				InOut.m_Value = 0;
		}
		else
		{
			if (InOut.m_Value)
				Self.m_Texture = TRE::ResourceManager::Instance().GetResource<TRE::VulkanTexture>(InOut.m_Value);
			else
				Self.m_Texture = nullptr;
		}

	} property_var_fnend(),
	property_var(m_Running).Name("Running"),
	property_var(m_ParticleCount).Name("ParticleCount"),
	property_var(m_Velocity).Name("Velocity"),
	property_var(m_Speed).Name("Speed"),
	property_var(m_SpawnRadius).Name("SpawnRadius"),
	property_var(m_VariationSpeed).Name("VariationSpeed"),
	property_var(m_VariationSize).Name("VariationSize"),
	property_var(m_LifeTime).Name("LifeTime"),
	property_var(m_Size).Name("Size"),
	property_var(m_Loop).Name("Loop"),
	property_var(m_PlayOnStart).Name("PlayOnStart"),	

} property_vend_h(TRE::ParticleComponent)