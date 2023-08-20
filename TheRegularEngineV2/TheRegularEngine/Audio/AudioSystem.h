#pragma once
#include "FMOD/fmod.hpp"
#include "TREIncludes.h"

namespace TRE
{
	class AudioSystem
	{
	public:
		AudioSystem();
		~AudioSystem();

		void Initialize();
		void Shutdown();

		struct AudioData {

			float m_Volume{ 1.f };
			float m_Pitch{ 1.f };
			bool m_Pause{ true };
			bool m_Loop{ false };
			bool m_Mute{ false };

			std::string m_FileName{};
			std::string m_FilePath{};
			FMOD::ChannelGroup* m_ChannelGroup;

			FMOD_VECTOR m_Position{};
			FMOD_VECTOR m_Forward{};
			FMOD_VECTOR m_Up{};

			bool m_IsPlaying{ false };

		};

		void SetAudioData(AudioData* file);
		void LoadFile(AudioData* file);
		void CreateChildChannelGroup(FMOD::ChannelGroup* child, std::string name);
		void Play(AudioData* file);
		void TogglePause(AudioData* file);
		void StopAudio(AudioData* file);

	private:
		FMOD::System* m_System = nullptr;
		FMOD::Sound* m_Sound = nullptr;
		FMOD::Channel* m_Channel = nullptr;

		FMOD::ChannelGroup* m_SFXChannelGroup = nullptr;
		FMOD::ChannelGroup* m_MusicChannelGroup = nullptr;


		const int MAX_CHANNELS = 64;

	};
}
