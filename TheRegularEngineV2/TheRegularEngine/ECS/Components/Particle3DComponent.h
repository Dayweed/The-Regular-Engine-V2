#pragma once
#include "Core/ECS.h"
#include "Resource/ResourceManager.h"
#include "Graphics/Material.h"
#include "Graphics/UniformBuffer.h"
#include "Graphics/VulkanUtilities.h"
#include "glm/gtc/type_ptr.hpp"

namespace TRE
{
	struct Particle3DUBO
	{
		glm::mat4 L2W [500];
	};

	class Particle3DComponent : property::base
	{
	public:	
		Particle3DComponent()
		{
			m_Data = std::make_shared<UniformBuffer>(UINT32_T_CAST(sizeof(Particle3DUBO)), 8);
		}

		std::shared_ptr<RenderObject> m_Mesh;
		std::shared_ptr<Material> m_Material;
		std::shared_ptr<UniformBuffer> m_Data;

		glm::vec4 m_Color{ glm::vec4(1.f, 1.f, 1.f, 1.f) };
		glm::vec3 m_Velocity = glm::vec3(0.f, 1.f, 0.f);
		glm::vec2 m_VariationSpeed = glm::vec2(0.15f, 1.f);	//Variation in the speed of the particles
		glm::vec2 m_VariationSize = glm::vec2(0.8f, 1.f);	//Variation in the size of the particles
		glm::vec2 m_FadeDuration = glm::vec2(0.25f, 0.75f);	//Percentage for fade in and fade out
		
		float m_SpawnRadius = 1.f;
		float m_Speed = 5.f;
		float m_LifeTime = 10.f;
		float m_ElapsedTime = 0.f;
		float m_Size = 1.f;
		int m_ParticleCount = 2;
		bool m_Running = true;
		bool m_Loop = true;
		bool m_PlayOnStart = true;
		bool m_Fade = true;
		bool m_Show = true;

		std::vector<glm::vec3> m_Position;
		std::vector<glm::vec3> m_Scale;

		Particle3DUBO m_ParticleData;

	public:
		void GenerateParticles(const glm::vec3& emitterPos);
		void UpdateParticles();
		void ResetParticles(const glm::vec3& emitterPos);

	private:
		void ResetParticlesData(const glm::vec3 emitterPos);

		property_vtable()

			friend void to_json(nlohmann::json& j, const Particle3DComponent& t)
		{
			const float* color = glm::value_ptr(t.m_Color);
			std::vector<float> storedColor{ color[0], color[1], color[2], color[3] };
			const float* velocity = glm::value_ptr(t.m_Velocity);
			std::vector<float> storedVelocity{ velocity[0], velocity[1], velocity[2] };
			const float* varSpeed = glm::value_ptr(t.m_VariationSpeed);
			std::vector<float> storedVarSpeed{ varSpeed[0], varSpeed[1] };
			const float* varSize = glm::value_ptr(t.m_VariationSize);
			std::vector<float> storedVarSize{ varSize[0], varSize[1] };
			const float* fadeDuration = glm::value_ptr(t.m_FadeDuration);
			std::vector<float> storedFadeDuration{ fadeDuration[0], fadeDuration[1] };

			j = nlohmann::json
			{
				{ "RenderObject", t.m_Mesh ? t.m_Mesh->GetHandleHex() : "0" },
				{ "MaterialInstance", t.m_Material ? t.m_Material->GetHandleHex() : "0" },
				{ "Color", storedColor },
				{ "Velocity", storedVelocity },
				{ "VariationSpeed", storedVarSpeed },
				{ "VariationSize", storedVarSize },
				{ "FadeDuration", storedFadeDuration },
				{ "SpawnRadius", t.m_SpawnRadius },
				{ "Speed", t.m_Speed },
				{ "LifeTime", t.m_LifeTime },
				{ "Size", t.m_Size },
				{ "ParticleCount", t.m_ParticleCount },
				{ "Loop", t.m_Loop },
				{ "PlayOnStart", t.m_PlayOnStart },
				{ "Fade", t.m_Fade }
			};
		}

		friend void from_json(const nlohmann::json& j, Particle3DComponent& t)
		{
			if (j.contains("RenderObject"))
			{
				std::string roString = j.at("RenderObject").get<std::string>();
				ResourceHandle roHandle = Resource::GetGUIDFromHex(roString);

				if (roHandle != 0)
				{
					if (auto renderObject = ResourceManager::Instance().GetResource<RenderObject>(roHandle); renderObject)
					{
						t.m_Mesh = renderObject;
					}
					else
					{
						t.m_Mesh = RenderObject::Deserialize(roString);

						if (t.m_Mesh == nullptr)
							TRE_CORE_CRITICAL(roString + ".geom not found!");
					}
				}
				else
				{
					t.m_Mesh = nullptr;
					TRE_CORE_WARN("Entity does not contain a mesh");
				}
			}
			if (j.contains("MaterialInstance"))
			{
				std::string matString = j.at("MaterialInstance").get<std::string>();
				ResourceHandle matHandle = Resource::GetGUIDFromHex(matString);

				if (matHandle != 0)
				{
					if (auto material = ResourceManager::Instance().GetResource<Material>(matHandle); material)
					{
						t.m_Material = material;
					}
					else
					{
						t.m_Material = Material::Deserialize(matString);

						if (t.m_Material == nullptr)
							TRE_CORE_CRITICAL(matString + ".mat not found!");
					}
				}
				//Else most likely default material
				else
				{
					t.m_Material = nullptr;
					TRE_CORE_WARN("Entity does not contain material instance, mimght cause error");
				}
			}
			if (j.contains("Color"))
			{
				std::vector<float> color = j.at("Color").get<std::vector<float>>();
				t.m_Color = glm::vec4(color[0], color[1], color[2], color[3]);
			}
			if (j.contains("Velocity"))
			{
				std::vector<float> velocity = j.at("Velocity").get<std::vector<float>>();
				t.m_Velocity = glm::vec3(velocity[0], velocity[1], velocity[2]);
			}
			if (j.contains("VariationSpeed"))
			{
				std::vector<float> varSpeed = j.at("VariationSpeed").get<std::vector<float>>();
				t.m_VariationSpeed = glm::vec2(varSpeed[0], varSpeed[1]);
			}
			if (j.contains("VariationSize"))
			{
				std::vector<float> varSize = j.at("VariationSize").get<std::vector<float>>();
				t.m_VariationSize = glm::vec2(varSize[0], varSize[1]);
			}
			if (j.contains("FadeDuration"))
			{
				std::vector<float> fadeDuration = j.at("FadeDuration").get<std::vector<float>>();
				t.m_FadeDuration = glm::vec2(fadeDuration[0], fadeDuration[1]);
			}
			if (j.contains("SpawnRadius"))
				t.m_SpawnRadius = j.at("SpawnRadius").get<float>();
			if (j.contains("Speed"))
				t.m_Speed = j.at("Speed").get<float>();
			if (j.contains("LifeTime"))
				t.m_LifeTime = j.at("LifeTime").get<float>();
			if (j.contains("Size"))
				t.m_Size = j.at("Size").get<float>();
			if (j.contains("ParticleCount"))
				t.m_ParticleCount = j.at("ParticleCount").get<int>();
			if (j.contains("Loop"))
				t.m_Loop = j.at("Loop").get<bool>();
			if (j.contains("PlayOnStart"))
			{
				t.m_PlayOnStart = j.at("PlayOnStart").get<bool>();
				if (t.m_PlayOnStart)
					t.m_Running = true;
				else
					t.m_Running = false;
			}
			if (j.contains("Fade"))
			{
				t.m_Fade = j.at("Fade").get<bool>();
				if (t.m_Fade)
					t.m_Color.a = 0.0f;
			}
		}

	};
}

property_begin(TRE::Particle3DComponent)
{
	property_var_fnbegin("Mesh", resource_list)
	{
		InOut.m_Type = "MESH";

		if (isRead)
		{
			if (Self.m_Mesh)
				InOut.m_Value = Self.m_Mesh->GetHandle();
			else
				InOut.m_Value = 0;
		}
		else
		{
			if (InOut.m_Value)
				Self.m_Mesh = TRE::ResourceManager::Instance().GetResource<TRE::RenderObject>(InOut.m_Value);
			else
				Self.m_Mesh = nullptr;
		}

	} property_var_fnend(),
	property_var_fnbegin("Material Instance", resource_list)
	{
		InOut.m_Type = "MATERIAL";

		if (isRead)
		{
			if (Self.m_Material)
				InOut.m_Value = Self.m_Material->GetHandle();
			else
				InOut.m_Value = 0;
		}
		else
		{
			if (InOut.m_Value)
				Self.m_Material = TRE::ResourceManager::Instance().GetResource<TRE::Material>(InOut.m_Value);
			else
				Self.m_Material = nullptr;
		}

	} property_var_fnend(),
	property_var_fnbegin("Color", Color)
	{
		if (isRead)
		{
			InOut.m_Value = Self.m_Color;
		}
		else
		{
			Self.m_Color = InOut.m_Value;
		}
	} property_var_fnend(),
		property_var(m_Running).Name("Running"),
		property_var(m_ParticleCount).Name("ParticleCount"),
		property_var(m_Velocity).Name("Velocity"),
		property_var(m_Speed).Name("Speed"),
		property_var(m_SpawnRadius).Name("SpawnRadius"),
		property_var(m_LifeTime).Name("LifeTime"),
		property_var(m_Size).Name("Size"),
		property_var(m_Loop).Name("Loop"),
		property_var(m_PlayOnStart).Name("PlayOnStart"),
		property_var(m_VariationSpeed).Name("VariationSpeed"),
		property_var(m_VariationSize).Name("VariationSize"),
		property_var(m_Fade).Name("Fade"),
		property_var(m_FadeDuration).Name("FadeDuration"),
		property_var(m_Show).Name("Show Spawn Point")
} property_vend_h(TRE::Particle3DComponent)