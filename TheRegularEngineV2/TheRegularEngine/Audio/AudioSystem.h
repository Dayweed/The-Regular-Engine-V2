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
		void Play(Entity& go, bool play);
		void TogglePause(Entity& go);
		void StopAudio(Entity& go);

		int ErrorCheck(FMOD_RESULT result, std::string function);

		void SetVolume(Entity& go, const float volume);
		void SetPitch(Entity& go, const float pitch);
		void SetPause(Entity& go, const bool pause);
		void SetLoop(Entity& go, const bool loop);
		void SetChannelGroup(Entity& go, FMOD::ChannelGroup* channelgroup);

		float GetVolume(Entity& go);
		float GetPitch(Entity& go);
		bool GetPause(Entity& go);
		bool GetLoop(Entity& go);
		FMOD::ChannelGroup* GetChannelGroup(Entity& go);


	private:
		FMOD::System* m_System = nullptr;
		FMOD::Sound* m_Sound = nullptr;
		FMOD::Channel* m_Channel = nullptr;

		FMOD::ChannelGroup* m_SFXChannelGroup = nullptr;
		FMOD::ChannelGroup* m_MusicChannelGroup = nullptr;


		const int MAX_CHANNELS = 64;

	};

}
