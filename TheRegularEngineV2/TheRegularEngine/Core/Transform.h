#pragma once
#include "pch.h"
#include "ECS.h"
#include "System.h"

#include <glm/gtc/type_ptr.hpp>

namespace TRE
{
	class Transform : property::base
	{
	public:
		glm::vec3	m_Position{ 0.f,0.f,0.f };
		glm::vec3	m_Rotation{ 0.f,0.f,0.f };
		glm::vec3	m_Scale{ 1.f,1.f,1.f };
		bool		m_IsDirty{ false };
		const glm::mat4 GetModelMatrix() const;
		const glm::mat4 GetNormalMatrix() const;

		property_vtable()           // Allows the base class to get these properties  

		// MUST Use BOTH of this if have variables that are struct/class to serialize
		friend void to_json(nlohmann::json& j, const Transform& t) // Serialize
		{
			// Converting to vectors...
			const float* pos = glm::value_ptr(t.m_Position);
			std::vector<float> v_pos{ pos[0], pos[1], pos[2]};
			const float* rot = glm::value_ptr(t.m_Rotation);
			std::vector<float> v_rot{ rot[0], rot[1], rot[2]};
			const float* sca = glm::value_ptr(t.m_Scale);
			std::vector<float> v_sca{ sca[0], sca[1], sca[2]};

			j = nlohmann::json{
				{ "m_Position", v_pos },
				{ "m_Rotation", v_rot },
				{ "m_Scale", v_sca }
			};
		}
		friend void from_json(const nlohmann::json& j, Transform& t) // Deserialize
		{
			std::vector<float> v_pos{ j.at("m_Position").get<std::vector<float>>() };
			float a_pos[3]{ v_pos[0], v_pos[1], v_pos[2] };
			t.m_Position = glm::make_vec3(a_pos);
			std::vector<float> v_rot{ j.at("m_Rotation").get<std::vector<float>>() };
			float a_rot[3]{ v_rot[0], v_rot[1], v_rot[2] };
			t.m_Rotation = glm::make_vec3(a_rot);
			std::vector<float> v_sca{ j.at("m_Scale").get<std::vector<float>>() };
			float a_sca[3]{ v_sca[0], v_sca[1], v_sca[2] };
			t.m_Scale = glm::make_vec3(a_sca);
		}
	};

	class TransformSystem : public ECSSystem
	{
	public:
		void Update() override;
		void OnDestroyGO() override;
		void Shutdown() override;

		void SetPosition(Entity& go, const glm::vec3& position);
		void SetRotation(Entity& go, const glm::vec3& rotation);
		void SetScale(Entity& go, const glm::vec3& scale);

		const glm::vec3& GetPosition(const Entity& go) const;
		const glm::vec3& GetRotation(const Entity& go) const;
		const glm::vec3& GetScale(const Entity& go) const;
		const glm::mat4 GetModelMatrix(const Entity& go) const;
	private:
		bool m_IsDirty{ false };
	};
}

property_begin(TRE::Transform)
{
		property_var(m_Position)
		, property_var(m_Rotation)
		, property_var(m_Scale)

} property_vend_h(TRE::Transform)