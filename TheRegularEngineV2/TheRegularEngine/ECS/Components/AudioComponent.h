#pragma once
#include "ECS/ECS.h"

namespace TRE
{
	class Audio : property::base
	{
	public:

		FMOD::ChannelGroup* m_ChannelGroup{};
		FMOD::Channel* m_Channel{};
		FMOD::Sound* m_Sound{};

		std::string m_FileName{ "" };
		std::vector<std::string> m_audioFiles;
		float m_Volume{ 1.f };
		float m_Pitch{ 1.f };
		int m_Priority{ 0 };
		bool m_Pause{ false };
		bool m_Loop{ false };
		bool m_Mute{ false };
		bool m_Play{ false };
		bool m_PlayOnStart{ true };
		bool m_Spatialize{ false };
		float m_MinDistance{ 1.f };
		float m_MaxDistance{ 300.f };

		bool m_HasCompiled{ false };
		FMOD_VECTOR m_goPosition{ 0.0f, 0.0f, 0.0f };
		property_vtable()
			//std::map<int, std::string> channelIndex{};

			bool m_isPlaying{ false };

		friend void to_json(nlohmann::json& j, const Audio& t) //serialize
		{
			std::vector<float> v_pos{ t.m_goPosition.x, t.m_goPosition.y, t.m_goPosition.z };

			j = nlohmann::json{
				{"m_FileName", t.m_FileName},
				{ "m_Play", t.m_Play },
				{ "m_Volume", t.m_Volume },
				{ "m_Pitch", t.m_Pitch },
				{ "m_Priority", t.m_Priority },
				{ "m_Pause", t.m_Pause},
				{ "m_Loop", t.m_Loop},
				{ "m_Mute", t.m_Mute},
				{ "m_PlayOnStart", t.m_PlayOnStart},
				{ "m_Spatialize", t.m_Spatialize},
				{ "m_MinDistance", t.m_MinDistance},
				{ "m_MaxDistance", t.m_MaxDistance},
				{ "m_goPosition", v_pos }
			};
		}

		friend void from_json(const nlohmann::json& j, Audio& t) //deserialize
		{
			t.m_FileName = j.at("m_FileName").get<std::string>();
			t.m_Play = j.at("m_Play").get<bool>();
			t.m_Volume = j.at("m_Volume").get<float>();
			t.m_Pitch = j.at("m_Pitch").get<float>();
			t.m_Priority = j.at("m_Priority").get<int>();
			t.m_Pause = j.at("m_Pause").get<bool>();
			t.m_Loop = j.at("m_Loop").get<bool>();
			t.m_Mute = j.at("m_Mute").get<bool>();
			t.m_PlayOnStart = j.at("m_PlayOnStart").get<bool>();
			t.m_Spatialize = j.at("m_Spatialize").get<bool>();
			t.m_MinDistance = j.at("m_MinDistance").get<float>();
			t.m_MaxDistance = j.at("m_MaxDistance").get<float>();

			std::vector<float> v_pos{ j.at("m_goPosition").get<std::vector<float>>() };
			float a_pos[3]{ v_pos[0], v_pos[1], v_pos[2] };
			t.m_goPosition.x = a_pos[0];
			t.m_goPosition.y = a_pos[1];
			t.m_goPosition.z = a_pos[2];

		}
	};
}

property_begin(TRE::Audio)
{
	property_var(m_FileName),
		property_var(m_Play),
		property_var(m_Volume),
		property_var(m_Pitch),
		property_var(m_Priority),
		property_var(m_Pause),
		property_var(m_Loop),
		property_var(m_Mute),
		property_var(m_PlayOnStart),
		property_var(m_Spatialize),
		property_var(m_MinDistance),
		property_var(m_MaxDistance),
		property_var(m_goPosition)
} property_vend_h(TRE::Audio)