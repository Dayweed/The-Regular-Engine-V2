#pragma once
#include "pch.h"
#include "Core/ECS.h"
#include "Core/System.h"
#include <glm/gtc/type_ptr.hpp>

namespace TRE
{
	class DirectionalLight : property::base
	{
	public:
		glm::vec3 Direction{ 1,-1,1 };
		glm::vec4 AmbientColor{ 1,1,1,1 }; ///RGB, A = Intensity

		property_vtable()

		friend void to_json(nlohmann::json& j, const DirectionalLight& t)
		{
			const float* direction = glm::value_ptr(t.Direction);
			std::vector<float> v_dir{ direction[0], direction[1], direction[2] };
			const float* ambientColor = glm::value_ptr(t.AmbientColor);
			std::vector<float> v_ambientColor{ ambientColor[0], ambientColor[1], ambientColor[2], ambientColor[3] };

			j = nlohmann::json{
				{ "m_Direction", v_dir },
				{ "m_AmbientColor", v_ambientColor }
			};
		}
		friend void from_json(const nlohmann::json& j, DirectionalLight& t)
		{
			std::vector<float> v_dir{ j.at("m_Direction").get<std::vector<float>>() };
			float a_dir[3]{ v_dir[0], v_dir[1], v_dir[2] };
			t.Direction = glm::make_vec3(a_dir);

			std::vector<float> v_ambientColor{ j.at("m_AmbientColor").get<std::vector<float>>() };
			float a_ambientColor[4]{ v_ambientColor[0], v_ambientColor[1], v_ambientColor[2], v_ambientColor[3] };
			t.AmbientColor = glm::make_vec4(a_ambientColor);
		}
	};

	class PointLight : property::base
	{
	public:
		glm::vec4 Position{ 0,0,0,1 }; ///XYZ, W = Attenuation
		glm::vec4 AmbientColor{ 1,1,1,1 }; ///RGB, A = Intensity
	};

	class LightSystem : public ECSSystem
	{
	public: 
		void LateUpdate() override;
	};
}

property_begin(TRE::DirectionalLight)
{
	/*property_var(Direction)
	, */property_var_fnbegin("Ambient Color", Color)
	{
		if(isRead)
			InOut.m_Value = Self.AmbientColor;
		else
			Self.AmbientColor = InOut.m_Value;
	}property_var_fnend()

} property_vend_h(TRE::DirectionalLight)
