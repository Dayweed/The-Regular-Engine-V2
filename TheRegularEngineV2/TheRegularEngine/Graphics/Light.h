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
		glm::vec4 DirectionalColor{ 1,1,1,1 }; ///RGB, A = Intensity
		glm::vec4 AmbientColor{ 1,1,1,0.05 }; ///RGB, A = Intensity

		glm::vec3 GetUpVec() const;

		property_vtable()

		friend void to_json(nlohmann::json& j, const DirectionalLight& t)
		{
			const float* direction = glm::value_ptr(t.Direction);
			std::vector<float> v_dir{ direction[0], direction[1], direction[2] };
			const float* directionalColor = glm::value_ptr(t.DirectionalColor);
			std::vector<float> v_directionalColor{ directionalColor[0], directionalColor[1], directionalColor[2], directionalColor[3] };
			const float* ambientColor = glm::value_ptr(t.AmbientColor);
			std::vector<float> v_ambientColor{ ambientColor[0], ambientColor[1], ambientColor[2], ambientColor[3] };

			j = nlohmann::json{
				{ "m_Direction", v_dir },
				{ "m_DirectionalColor", v_directionalColor },
				{ "m_AmbientColor", v_ambientColor }
			};
		}
		friend void from_json(const nlohmann::json& j, DirectionalLight& t)
		{
			if (j.contains("m_Direction"))
			{
				std::vector<float> v_dir{ j.at("m_Direction").get<std::vector<float>>() };
				float a_dir[3]{ v_dir[0], v_dir[1], v_dir[2] };
				t.Direction = glm::make_vec3(a_dir);
			}

			if (j.contains("m_DirectionalColor"))
			{
				std::vector<float> v_directionalColor{ j.at("m_DirectionalColor").get<std::vector<float>>() };
				float a_directionalColor[4]{ v_directionalColor[0], v_directionalColor[1], v_directionalColor[2], v_directionalColor[3] };
				t.DirectionalColor = glm::make_vec4(a_directionalColor);
			}

			if (j.contains("m_AmbientColor"))
			{
				std::vector<float> v_ambientColor{ j.at("m_AmbientColor").get<std::vector<float>>() };
				float a_ambientColor[4]{ v_ambientColor[0], v_ambientColor[1], v_ambientColor[2], v_ambientColor[3] };
				t.AmbientColor = glm::make_vec4(a_ambientColor);
			}
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
	property_var_fnbegin("Directional Light Color", Color3)
	{
		if (isRead)
		{
			glm::vec3 color = glm::vec3(Self.DirectionalColor.x, Self.DirectionalColor.y, Self.DirectionalColor.z);
			InOut.m_Value = color;
		}
		else
		{
			glm::vec3 color = InOut.m_Value;
			Self.DirectionalColor = glm::vec4(color.x, color.y, color.z, Self.DirectionalColor.w);
		}
	}property_var_fnend()
	, property_var_fnbegin("Directional Light Intensity", float)
	{
		if (isRead)
			InOut = Self.DirectionalColor.w;
		else
			Self.DirectionalColor.w = InOut;
	}property_var_fnend()
	, property_var_fnbegin("Ambient Light Color", Color3)
	{
		if (isRead)
		{
			glm::vec3 color = glm::vec3(Self.AmbientColor.x, Self.AmbientColor.y, Self.AmbientColor.z);
			InOut.m_Value = Self.AmbientColor;
		}
		else
		{
			glm::vec3 color = InOut.m_Value;
			Self.AmbientColor = glm::vec4(color.x, color.y, color.z, Self.AmbientColor.w);
		}
	}property_var_fnend()
	, property_var_fnbegin("Ambient Light Intensity", float)
	{
		if (isRead)
			InOut = Self.AmbientColor.w;
		else
			Self.AmbientColor.w = InOut;
	}property_var_fnend()

} property_vend_h(TRE::DirectionalLight)
