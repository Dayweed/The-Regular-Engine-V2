#pragma once
#include "ECS/ECS.h"

namespace TRE
{
	class AudioListener : property::base
	{
	public:
		FMOD_VECTOR m_Position{ 0.0f, 0.0f, 0.0f };
		FMOD_VECTOR m_Forward{ 0.0f, 0.0f, 0.0f };
		FMOD_VECTOR m_Up{ 0.0f, 0.0f, 0.0f };

		FMOD_VECTOR m_Velocity{};

		friend void to_json(nlohmann::json& j, const AudioListener& t) //serialize
		{
			std::vector<float> v_pos{ t.m_Position.x, t.m_Position.y, t.m_Position.z };
			std::vector<float> v_fwd{ t.m_Forward.x, t.m_Forward.y, t.m_Forward.z };
			std::vector<float> v_up{ t.m_Up.x, t.m_Up.y, t.m_Up.z };
			std::vector<float> v_vel{ t.m_Velocity.x, t.m_Velocity.y, t.m_Velocity.z };

			j = nlohmann::json{
				{ "m_Position", v_pos },
				{ "m_Forward", v_fwd },
				{ "m_Up", v_up },
				{ "m_Velocity", v_vel}
			};
		}

		friend void from_json(const nlohmann::json& j, AudioListener& t) //deserialize
		{
			std::vector<float> v_pos{ j.at("m_Position").get<std::vector<float>>() };
			float a_pos[3]{ v_pos[0], v_pos[1], v_pos[2] };
			t.m_Position.x = a_pos[0];
			t.m_Position.y = a_pos[1];
			t.m_Position.z = a_pos[2];

			std::vector<float> v_fwd{ j.at("m_Forward").get<std::vector<float>>() };
			float a_fwd[3]{ v_fwd[0], v_fwd[1], v_fwd[2] };
			t.m_Forward.x = a_fwd[0];
			t.m_Forward.y = a_fwd[1];
			t.m_Forward.z = a_fwd[2];

			std::vector<float> v_up{ j.at("m_Up").get<std::vector<float>>() };
			float a_up[3]{ v_up[0], v_up[1], v_up[2] };
			t.m_Up.x = a_up[0];
			t.m_Up.y = a_up[1];
			t.m_Up.z = a_up[2];

			std::vector<float> v_vel{ j.at("m_Velocity").get<std::vector<float>>() };
			float a_vel[3]{ v_vel[0], v_vel[1], v_vel[2] };
			t.m_Velocity.x = a_vel[0];
			t.m_Velocity.y = a_vel[1];
			t.m_Velocity.z = a_vel[2];
		};

		//NLOHMANN_DEFINE_TYPE_INTRUSIVE(AudioListener, m_Position, m_Forward, m_Up, m_Velocity);
		property_vtable()
	};
}

property_begin(TRE::AudioListener)
{
	property_var(m_Position),
		property_var(m_Forward),
		property_var(m_Up),
		property_var(m_Velocity)
} property_vend_h(TRE::AudioListener)