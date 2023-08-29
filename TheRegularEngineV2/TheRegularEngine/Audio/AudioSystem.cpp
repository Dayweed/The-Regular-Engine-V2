#include "pch.h"
#include "Audio/AudioSystem.h"

namespace TRE
{
	AudioSystem::AudioSystem()
	{
		ErrorCheck(FMOD::System_Create(&m_System), "System_Create()");

		m_System->init(MAX_CHANNELS, FMOD_INIT_NORMAL, nullptr);

		m_System->createChannelGroup("SFX", &m_SFXChannelGroup);
		m_System->createChannelGroup("Music", &m_MusicChannelGroup);

		printf("initializing audio\n");
	}

	AudioSystem::~AudioSystem()
	{
		printf("audio system shutdown\n");
		Shutdown();
	}

	void AudioSystem::Update()
	{

	}

	void AudioSystem::Shutdown()
	{
		ErrorCheck(m_System->release(), "System->Release()");
		ErrorCheck(m_Sound->release(), "Sound->Release()");
	}

	//void AudioSystem::SetAudioData(Audio* file)
	//{
	//	//placeholder data
	//	file->m_FileName = "ViveLeFromageBGM1";

	//	std::string file_path_{ "../Assets/Audio/" };
	//	std::string full_path = file_path_ + file->m_FileName;
	//	std::size_t fs = file->m_FileName.find_last_of(".");
	//	std::string filetype = file->m_FileName.substr(fs);

	//	if (filetype != ".wav" || filetype != ".ogg")
	//	{
	//		std::cout << "[SetChannelName:] Invalid file type! " + file->m_FileName + " is not a .wav or .ogg file. Sound file not loaded." << std::endl;
	//		return;
	//	}

	//	file->m_FilePath = full_path;
	//	file->m_ChannelGroup = m_MusicChannelGroup;

	//}

	void AudioSystem::LoadFile(GO& go) //(GO& go, filepath)
	{
		Audio& audio = go.get()->GetComponent<Audio>();

		std::ifstream ifs(audio.m_FilePath);

		if (!ifs.is_open())
		{
			printf("Unable to load audio file\n\n");
		}

		ErrorCheck(m_System->createSound(audio.m_FilePath.c_str(), FMOD_DEFAULT, nullptr, &m_Sound), "LoadFile()");
	}

	void AudioSystem::CreateChildChannelGroup(FMOD::ChannelGroup* child, std::string channelname)
	{
		child = nullptr;
		ErrorCheck(m_System->createChannelGroup(channelname.c_str(), &child), "createChannelGroup()");
	}

	void AudioSystem::Play(GO& go, bool play)
	{
		Audio& audio = go.get()->GetComponent<Audio>();
		if (audio.m_IsPlaying = true)
		{
			ErrorCheck(m_System->playSound(m_Sound, audio.m_ChannelGroup, audio.m_Pause, &m_Channel), "playSound()");
		}
	}

	void AudioSystem::TogglePause(GO& go)
	{
		Audio& audio = go.get()->GetComponent<Audio>();
		audio.m_Pause = !audio.m_Pause;
		ErrorCheck(audio.m_ChannelGroup->setPaused(&audio.m_Pause), "TogglePause()");
	}

	void AudioSystem::StopAudio(GO& go)
	{
		Audio& audio = go.get()->GetComponent<Audio>();
		ErrorCheck(audio.m_ChannelGroup->stop(), "StopAudio()");
	}

	int AudioSystem::ErrorCheck(FMOD_RESULT result, std::string function)
	{
		if (result != FMOD_OK) {
			std::cout << "FMOD ERROR: " << result << " " << function << std::endl;
			return 1;
		}
		else
		{
			std::cout << "FMOD : " << function << " Success" << std::endl;
		}
		return 0;
	}

	void AudioSystem::SetVolume(GO& go, const float volume)
	{
		Audio& audio = go.get()->GetComponent<Audio>();
		audio.m_Volume = volume;
	}

	void AudioSystem::SetPitch(GO& go, const float pitch)
	{
		Audio& audio = go.get()->GetComponent<Audio>();
		audio.m_Pitch = pitch;
	}

	void AudioSystem::SetPause(GO& go, const bool pause)
	{
		Audio& audio = go.get()->GetComponent<Audio>();
		audio.m_Pause = pause;
	}

	void AudioSystem::SetLoop(GO& go, const bool loop)
	{
		Audio& audio = go.get()->GetComponent<Audio>();
		audio.m_Loop = loop;
	}

	void AudioSystem::SetChannelGroup(GO& go, FMOD::ChannelGroup* channelgroup)
	{
		Audio& audio = go.get()->GetComponent<Audio>();
		audio.m_ChannelGroup = channelgroup;
	}

	float AudioSystem::GetVolume(GO& go)
	{
		return go.get()->GetComponent<Audio>().m_Volume;
	}

	float AudioSystem::GetPitch(GO& go)
	{
		return go.get()->GetComponent<Audio>().m_Pause;
	}

	bool AudioSystem::GetPause(GO& go)
	{
		return go.get()->GetComponent<Audio>().m_Pause;
	}

	bool AudioSystem::GetLoop(GO& go)
	{
		return go.get()->GetComponent<Audio>().m_Loop;
	}

	FMOD::ChannelGroup* AudioSystem::GetChannelGroup(GO& go)
	{
		return go.get()->GetComponent<Audio>().m_ChannelGroup;
	}

}
