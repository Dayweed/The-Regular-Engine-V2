#pragma once
#include "Core/System.h"
#include "FMOD/fmod.hpp"
#include "TREIncludes.h"
#include "Properties.h"
#include <map>

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

		bool m_isPlaying{};

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

	class AudioSystem : public ECSSystem
	{
	public:
		AudioSystem();
		~AudioSystem() override;

		void Init() override;
		void GameUpdate() override;
		void LateUpdate() override;
		void BeforeReset() override;
		void AfterReset() override;
		void OnDestroyEntities() override;
		void Shutdown() override;

		//void SetAudioData(Audio* file);
		void LoadFile(Entity& go);
		void Load3DFile(Entity& go); //-----
		//void CreateChildChannelGroup(FMOD::ChannelGroup* child, std::string name);
		void Play(Entity& go, const bool shouldPlay);
		void TogglePause(Entity& go);
		void ToggleMute(Entity& go);
		void Stop(Entity& go);
		void CompileAudio(Entity& go);

		int ErrorCheck(FMOD_RESULT result, std::string function);

		void SetFileName(Entity& go, const std::string filename);
		void SetChannelGroup(Entity& go, const std::string channel);
		void SetListenerPosition(Entity& go);
		void SetSourcePosition(Entity& go);
		void SetSourceRadius(Entity& go, const float min, const float max);
		bool GetIsPlaying(Entity& go) const;

		FMOD_VECTOR GetListenerPosition(Entity& go) const;
		FMOD_VECTOR GetSourcePosition(Entity& go) const;
		const std::pair<float, float> GetSourceRadius(Entity& go) const;


		FMOD::ChannelGroup* GetChannelGroup(Entity& go);
		std::string GetFileName(Entity& go) const;


		FMOD_VECTOR glmVec3ToFmodVector(const glm::vec3& glmVector)
		{
			FMOD_VECTOR fmodVector;
			fmodVector.x = glmVector.x;
			fmodVector.y = glmVector.y;
			fmodVector.z = glmVector.z;
			return fmodVector;
		}

	private:
		FMOD::System* m_System = nullptr;

		FMOD::ChannelGroup* m_SFXChannelGroup = nullptr;
		FMOD::ChannelGroup* m_MusicChannelGroup = nullptr;

		const int MAX_CHANNELS = 64;
		;
		std::unordered_set<Entity> audioMap;
		std::unordered_map<Entity, FMOD::Sound*> soundToRemove;

	};

}

property_begin(TRE::AudioListener)
{
	property_var(m_Position),
		property_var(m_Forward),
		property_var(m_Up),
		property_var(m_Velocity)
} property_vend_h(TRE::AudioListener)

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