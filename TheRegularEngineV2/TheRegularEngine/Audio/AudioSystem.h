#pragma once
#include "Core/System.h"
#include "FMOD/fmod.hpp"
#include "TREIncludes.h"

namespace TRE
{
	struct Audio
	{
		float m_Volume{ 1.f };
		float m_Pitch{ 1.f };
		bool m_Pause{ false };
		bool m_Loop{ false };
		//bool m_Mute{ false };

		std::string m_FileName{ "ViveLeFromageBGM1" };
		std::string m_FilePath{ "../Assets/Audio/ViveLeFromageBGM1.wav" };
		FMOD::ChannelGroup* m_ChannelGroup{};

		FMOD_VECTOR m_Position{ 0.0f, 0.0f, 0.0f };
		FMOD_VECTOR m_Forward{ 0.0f, 0.0f, 0.0f };
		FMOD_VECTOR m_Up{ 0.0f, 0.0f, 0.0f };

		bool m_IsPlaying{ false };
	};

	class AudioSystem : public System
	{
	public:
		AudioSystem();
		~AudioSystem() override;

		void Update() override;
		void Shutdown() override;

		//void SetAudioData(Audio* file);
		void LoadFile(GO& go);
		void CreateChildChannelGroup(FMOD::ChannelGroup* child, std::string name);
		void Play(GO& go, bool play);
		void TogglePause(GO& go);
		void StopAudio(GO& go);

		int ErrorCheck(FMOD_RESULT result, std::string function);

		void SetVolume(GO& go, const float volume);
		void SetPitch(GO& go, const float pitch);
		void SetPause(GO& go, const bool pause);
		void SetLoop(GO& go, const bool loop);
		void SetChannelGroup(GO& go, FMOD::ChannelGroup* channelgroup);

		float GetVolume(GO& go);
		float GetPitch(GO& go);
		bool GetPause(GO& go);
		bool GetLoop(GO& go);
		FMOD::ChannelGroup* GetChannelGroup(GO& go);


	private:
		FMOD::System* m_System = nullptr;
		FMOD::Sound* m_Sound = nullptr;
		FMOD::Channel* m_Channel = nullptr;

		FMOD::ChannelGroup* m_SFXChannelGroup = nullptr;
		FMOD::ChannelGroup* m_MusicChannelGroup = nullptr;


		const int MAX_CHANNELS = 64;

	};

}
