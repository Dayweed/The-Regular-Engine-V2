#pragma once
#include "pch.h"
#include "Core/ECS.h"
#include "Core/System.h"

#include <glm/gtc/type_ptr.hpp>

namespace TRE
{
	enum TransformDirtyFlags
	{
		TRE_DIRTY_NONE = 0,	// binary 0000
		TRE_DIRTY_POSITION = 1 << 0, // binary 0001
		TRE_DIRTY_ROTATION = 1 << 1, // binary 0010
		TRE_DIRTY_SCALE = 1 << 2, // binary 0100
		TRE_DIRTY_ALL = 1 << 3 // binary 0101
	};

	class Transform : property::base
	{
	public:
		glm::mat4	m_WorldXform{ 1.f };
		//glm::mat4	m_LocalXform{ 1.f };
		// When changing the bottom three, it must be changed to a specific name for gizmo to play nice with prefab and reflection
		glm::vec3	m_Position{ 0.f,0.f,0.f };		// [m_Position] becomes [TRE::Trasnform/Position] for ImGui::Gizmo in ViewportPanel.cpp
		glm::vec3	m_Rotation{ 0.f,0.f,0.f };		// [m_Rotation] becomes [TRE::Trasnform/Rotation] for ImGui::Gizmo in ViewportPanel.cpp
		glm::vec3	m_Scale{ 1.f,1.f,1.f };			// [m_Scale] becomes [TRE::Trasnform/Scale] for ImGui::Gizmo in ViewportPanel.cpp

		glm::vec3	m_LocalPosition{ 0.f,0.f,0.f };
		glm::vec3	m_LocalRotation{ 0.f,0.f,0.f };
		glm::vec3	m_LocalScale{ 1.f,1.f,1.f };

		int			m_DirtyFlags{};
		bool		m_IsDirty{ true };
	private:
		glm::vec3	m_RotationOld{ 0,0,0 };
		glm::quat	m_OldRotation{ 0,0,0,1 };
	public:
		void CalculateWorldMatrix();
		const glm::mat4 CalculateLocalMatrix();
		void DecomposeWorldMatrix(const glm::mat4 newWorld);
		void UpdateLocalData(Transform& parent);
	public:
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

			const float* locpos = glm::value_ptr(t.m_LocalPosition);
			std::vector<float> v_lpos{ locpos[0], locpos[1], locpos[2]};
			const float* locrot = glm::value_ptr(t.m_LocalRotation);
			std::vector<float> v_lrot{ locrot[0], locrot[1], locrot[2]};
			const float* locsca = glm::value_ptr(t.m_LocalScale);
			std::vector<float> v_lsca{ locsca[0], locsca[1], locsca[2]};

			j = nlohmann::json{
				{ "m_Position", v_pos },
				{ "m_Rotation", v_rot },
				{ "m_Scale", v_sca },
				/*{ "m_LocalPosition", v_lpos },
				{ "m_LocalRotation", v_lrot },
				{ "m_LocalScale", v_lsca }*/
			};
		}
		friend void from_json(const nlohmann::json& j, Transform& t) // Deserialize
		{
			if (j.contains("m_Position"))
			{
				std::vector<float> v_pos{ j.at("m_Position").get<std::vector<float>>() };
				float a_pos[3]{ v_pos[0], v_pos[1], v_pos[2] };
				t.m_Position = glm::make_vec3(a_pos);
			}
			if (j.contains("m_Rotation"))
			{
				std::vector<float> v_rot{ j.at("m_Rotation").get<std::vector<float>>() };
				float a_rot[3]{ v_rot[0], v_rot[1], v_rot[2] };
				t.m_Rotation = glm::make_vec3(a_rot);
			}
			if (j.contains("m_Scale"))
			{
				std::vector<float> v_sca{ j.at("m_Scale").get<std::vector<float>>() };
				float a_sca[3]{ v_sca[0], v_sca[1], v_sca[2] };
				t.m_Scale = glm::make_vec3(a_sca);
			}
			/*if (j.contains("m_LocalPosition"))
			{
				std::vector<float> v_lpos{ j.at("m_LocalPosition").get<std::vector<float>>() };
				float a_lpos[3]{ v_lpos[0], v_lpos[1], v_lpos[2] };
				t.m_LocalPosition = glm::make_vec3(a_lpos);
			}
			if (j.contains("m_LocalRotation"))
			{
				std::vector<float> v_lrot{ j.at("m_LocalRotation").get<std::vector<float>>() };
				float a_lrot[3]{ v_lrot[0], v_lrot[1], v_lrot[2] };
				t.m_LocalRotation = glm::make_vec3(a_lrot);
			}
			if (j.contains("m_LocalScale"))
			{
				std::vector<float> v_lsca{ j.at("m_LocalScale").get<std::vector<float>>() };
				float a_lsca[3]{ v_lsca[0], v_lsca[1], v_lsca[2] };
				t.m_LocalScale = glm::make_vec3(a_lsca);
			}

			if (t.m_LocalPosition == glm::vec3() && t.m_LocalRotation == glm::vec3() && t.m_Scale == glm::vec3(1,1,1))
				t.m_IsDirty = true;*/
			t.m_IsDirty = true;
		}
	};

	class TransformSystem : public ECSSystem
	{
	public:
		void LateUpdate() override;

		glm::vec3 RotateMatrix(glm::vec3 vector, glm::vec3 rotation);
	};
}

property_begin(TRE::Transform)
{
		property_var(m_Position)
		, property_var(m_Rotation)
		, property_var(m_Scale)
		, property_var_fnbegin("Local Position", glm::vec3)
		{
			if (isRead)
			{
				InOut = Self.m_LocalPosition;
			}
		} property_var_fnend()
		, property_var_fnbegin("Local Rotation", glm::vec3)
		{
			if (isRead)
			{
				InOut = Self.m_LocalRotation;
			}
		} property_var_fnend()
		, property_var_fnbegin("Local Scale", glm::vec3)
		{
			if (isRead)
			{
				InOut = Self.m_LocalScale;
			}
		} property_var_fnend()

} property_vend_h(TRE::Transform)