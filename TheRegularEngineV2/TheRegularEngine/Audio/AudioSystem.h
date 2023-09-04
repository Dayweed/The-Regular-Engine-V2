#pragma once
#include "Core/System.h"
#include "FMOD/fmod.hpp"
#include "TREIncludes.h"

namespace TRE
{
	struct Audio
	{
		std::string m_FileName{ "ViveLeFromageBGM1" };
		std::string m_FilePath{ "../Assets/Audio/ViveLeFromageBGM1.wav" };
		FMOD::ChannelGroup* m_ChannelGroup{};

		float m_Volume{ 1.f };
		float m_Pitch{ 1.f };
		int m_Priority{ 0 };
		bool m_Pause{ false };
		bool m_Loop{ false };
		bool m_Mute{ false };
		bool m_Play{ true };
		bool m_Spatialize{ false };
		
		//
		FMOD_VECTOR m_goPosition{ 0.0f, 0.0f, 0.0f };

	};
	
	struct AudioListener
	{
		FMOD_VECTOR m_Position{ 0.0f, 0.0f, 0.0f };
		FMOD_VECTOR m_Forward{ 0.0f, 0.0f, 0.0f };
		FMOD_VECTOR m_Up{ 0.0f, 0.0f, 0.0f };
	};

	class AudioSystem : public ECSSystem
	{
	public:
		AudioSystem();
		~AudioSystem() override;

		void Update() override;
		void OnDestroyGO() override;
		void Shutdown() override;

		//void SetAudioData(Audio* file);
		void LoadFile(Entity& go);
		void CreateChildChannelGroup(FMOD::ChannelGroup* child, std::string name);
		void Play(Entity& go);
		void TogglePause(Entity& go);
		void StopAudio(Entity& go);
		void SetUp3DMode(Entity& go);
		void CompileAudio(Entity& go);

		int ErrorCheck(FMOD_RESULT result, std::string function);

		void SetVolume(Entity& go, const float volume);
		void SetPitch(Entity& go, const float pitch);
		void SetPause(Entity& go, const bool pause);
		void SetLoop(Entity& go, const bool loop);
		void SetChannelGroup(Entity& go, FMOD::ChannelGroup* channelgroup);
		void SetPriority(Entity& go, const int priority);
		void SetMute(Entity& go, const bool mute);
		void SetPlay(Entity& go, const bool play);
		void SetSpatialize(Entity& go, const bool spatialize);

		float GetVolume(Entity& go);
		float GetPitch(Entity& go);
		bool GetPause(Entity& go);
		bool GetLoop(Entity& go);
		int GetPriority(Entity& go);
		bool GetMute(Entity& go);
		bool GetPlay(Entity& go);
		bool GetSpatialize(Entity& go);

		FMOD::ChannelGroup* GetChannelGroup(Entity& go);


	private:
		FMOD::System* m_System = nullptr;
		FMOD::Sound* m_Sound = nullptr;
		FMOD::Channel* m_Channel = nullptr;

		FMOD::ChannelGroup* m_SFXChannelGroup = nullptr;
		FMOD::ChannelGroup* m_MusicChannelGroup = nullptr;

		glm::vec3 m_ListenerPosition;

		const int MAX_CHANNELS = 64;

	};

}
