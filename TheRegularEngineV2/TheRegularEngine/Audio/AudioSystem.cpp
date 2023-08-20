#include "pch.h"
#include "Audio/AudioSystem.h"

namespace TRE
{
	AudioSystem::AudioSystem()
	{
	}

	AudioSystem::~AudioSystem()
	{
	}

	void AudioSystem::Initialize()
	{
		FMOD::System_Create(&m_System);
		m_System->init(MAX_CHANNELS, FMOD_INIT_NORMAL, nullptr);

		m_System->createChannelGroup("SFX", &m_SFXChannelGroup);
		m_System->createChannelGroup("Music", &m_MusicChannelGroup);
	}

	void AudioSystem::Shutdown()
	{
		m_System->release();
		m_Sound->release();
	}

	void AudioSystem::SetAudioData(AudioData* file)
	{
		//placeholder data
		file->m_FileName = "ViveLeFromageBGM1";

		std::string file_path_{ "../Assets/Audio/" };
		std::string full_path = file_path_ + file->m_FileName;
		std::size_t fs = file->m_FileName.find_last_of(".");
		std::string filetype = file->m_FileName.substr(fs);

		file->m_FilePath = full_path;
		file->m_ChannelGroup = m_MusicChannelGroup;

	}

	void AudioSystem::LoadFile(AudioData* file)
	{
		m_System->createSound(file->m_FilePath.c_str(), FMOD_DEFAULT, nullptr, &m_Sound);
	}

	void AudioSystem::CreateChildChannelGroup(FMOD::ChannelGroup* child, std::string channelname)
	{
		child = nullptr;
		m_System->createChannelGroup(channelname.c_str(), &child);
	}

	void AudioSystem::Play(AudioData* file)
	{
		file->m_Pause = false;
		m_System->playSound(m_Sound, file->m_ChannelGroup, file->m_Pause, &m_Channel);
	}

	void AudioSystem::TogglePause(AudioData* file)
	{
		file->m_Pause = !file->m_Pause;
		file->m_ChannelGroup->setPaused(&file->m_Pause);
	}

	void AudioSystem::StopAudio(AudioData* file)
	{
		file->m_ChannelGroup->stop();
	}

}
