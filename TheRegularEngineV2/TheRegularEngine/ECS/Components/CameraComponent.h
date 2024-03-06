#pragma once
#include "ECS/ECS.h"
#include "Graphics/Camera.h"

namespace TRE
{
	class Camera : property::base
	{
	public:
		BaseCamera m_BaseCamera;
		bool m_IsMainCamera{ false }; // SSSS
		//For transition
		glm::vec3 m_StartPosition{};
		glm::vec3 m_TransitionPosition{};
		glm::vec3 m_StartRotation{};
		glm::vec3 m_TransitionRotation{};
		float m_InterpolationValue{};
		float m_InterpolationSpeed{ 0.5f };
		bool m_IsTransitioning{ false };
		//End For transition
		bool m_IsDirty{ false };

		// MUST Use BOTH of this if have variables that are struct/class to serialize
		friend void to_json(nlohmann::json& j, const Camera& t) // Serialize
		{
			const float* vpsize = glm::value_ptr(t.m_BaseCamera.m_ViewportSize);
			std::vector<float> v_vpsize{ vpsize[0], vpsize[1] };
			const float* focal = glm::value_ptr(t.m_BaseCamera.m_FocalPoint);
			std::vector<float> v_focal{ focal[0], focal[1], focal[2] };

			j = nlohmann::json{
				{ "m_ViewportSize", v_vpsize },
				{ "m_Fov", t.m_BaseCamera.m_Fov },
				{ "m_Near", t.m_BaseCamera.m_Near },
				{ "m_Far", t.m_BaseCamera.m_Far },
				{ "m_FocalLength", t.m_BaseCamera.m_FocalLength },
				{ "m_FocalPoint", v_focal },
				{ "m_Left", t.m_BaseCamera.m_Left },
				{ "m_Right", t.m_BaseCamera.m_Right },
				{ "m_Bottom", t.m_BaseCamera.m_Bottom },
				{ "m_Top", t.m_BaseCamera.m_Top },
				{ "m_AspectRatio", t.m_BaseCamera.m_AspectRatio },
				{ "m_IsPerspective", t.m_BaseCamera.m_IsPerspective },
				{ "m_IsMainCamera", t.m_IsMainCamera },
			};
		}
		friend void from_json(const nlohmann::json& j, Camera& t) // Deserialize
		{
			if (j.contains("m_ViewportSize"))
			{
				std::vector<float> v_vpsize{ j.at("m_ViewportSize").get<std::vector<float>>() };
				float a_vpsize[2]{ v_vpsize[0], v_vpsize[1] };
				t.m_BaseCamera.m_ViewportSize = glm::make_vec2(a_vpsize);
			}
			if (j.contains("m_Fov"))
				t.m_BaseCamera.m_Fov = j.at("m_Fov").get<float>();
			if (j.contains("m_Near"))
				t.m_BaseCamera.m_Near = j.at("m_Near").get<float>();
			if (j.contains("m_Far"))
				t.m_BaseCamera.m_Far = j.at("m_Far").get<float>();
			if (j.contains("m_FocalLength"))
				t.m_BaseCamera.m_FocalLength = j.at("m_FocalLength").get<float>();
			if (j.contains("m_FocalPoint"))
			{
				std::vector<float> v_pnt{ j.at("m_FocalPoint").get<std::vector<float>>() };
				float a_pnt[3]{ v_pnt[0], v_pnt[1], v_pnt[2] };
				t.m_BaseCamera.m_FocalPoint = glm::make_vec3(a_pnt);
			}
			if (j.contains("m_Left"))
				t.m_BaseCamera.m_Left = j.at("m_Left").get<float>();
			if (j.contains("m_Right"))
				t.m_BaseCamera.m_Right = j.at("m_Right").get<float>();
			if (j.contains("m_Bottom"))
				t.m_BaseCamera.m_Bottom = j.at("m_Bottom").get<float>();
			if (j.contains("m_Top"))
				t.m_BaseCamera.m_Top = j.at("m_Top").get<float>();
			if (j.contains("m_AspectRatio"))
				t.m_BaseCamera.m_AspectRatio = j.at("m_AspectRatio").get<float>();
			if (j.contains("m_IsPerspective"))
				t.m_BaseCamera.m_IsPerspective = j.at("m_IsPerspective").get<bool>();
			if (j.contains("m_IsMainCamera"))
				t.m_IsMainCamera = j.at("m_IsMainCamera").get<bool>();

			t.m_IsDirty = true;
		}

		property_vtable()
	};
}

property_begin(TRE::Camera)
{
	//property_var(m_BaseCamera.m_ViewportSize).Name("Viewport Size"),
	property_var(m_BaseCamera.m_Fov).Name("FOV"),
		property_var(m_BaseCamera.m_FocalLength).Name("Focal Length"),
		property_var(m_BaseCamera.m_FocalPoint).Name("Focal Point"),
		property_var(m_BaseCamera.m_Near).Name("Near"),
		property_var(m_BaseCamera.m_Far).Name("Far"),
		//property_var(m_BaseCamera.m_Left).Name("Left"),
		//property_var(m_BaseCamera.m_Right).Name("Right"),
		//property_var(m_BaseCamera.m_Bottom).Name("Bottom"),
		//property_var(m_BaseCamera.m_Top).Name("Top"),
		//property_var(m_BaseCamera.m_AspectRatio).Name("Aspect Ratio"),
		property_var(m_BaseCamera.m_IsPerspective).Name("IsPerspective"),
		property_var(m_IsMainCamera).Name("IsMainCamera")
} property_vend_h(TRE::Camera)