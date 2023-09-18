#include "pch.h"
#include "Core/Transform.h"
#include "AudioSystem.h"

namespace TRE
{
	AudioSystem::AudioSystem()
	{
		if (ErrorCheck(FMOD::System_Create(&m_System), "System_Create()"))
		{
			TRE_CORE_INFO("FMOD: Audio System Initialized");
		}

		m_System->init(MAX_CHANNELS, FMOD_INIT_NORMAL, nullptr);

		m_System->createChannelGroup("SFX", &m_SFXChannelGroup);
		m_System->createChannelGroup("Music", &m_MusicChannelGroup);
		
	}

	AudioSystem::~AudioSystem()
	{
		ErrorCheck(m_System->release(), "FMOD: m_System->release()");
		ErrorCheck(m_Sound->release(), "FMOD: m_Sound->release()");
	}

	void AudioSystem::Update()
	{
		Entity player;

		for (Entity& go : ECSManager::Instance().GetEntities<Properties>())
		{
			if (go.get()->GetComponent<Properties>().m_Name == "Player") //temporary code
			{
				player = go;
			}
			else
			{
				return;
			}
		}

		if (player)
		{
			m_ListenerPosition = player.get()->GetComponent<Transform>().m_Position;
		}

	}

	void AudioSystem::OnDestroyGO()
	{

	}

	void AudioSystem::Shutdown()
	{
	}

	void AudioSystem::LoadFile(Entity& go) //(Entity& go, filepath)
	{
		Audio& audio = go.get()->GetComponent<Audio>();

		std::string file_path_{ "../Assets/Audio/" };
		std::string m_FilePath = file_path_ + audio.m_FileName;
		std::size_t fs = audio.m_FileName.find_last_of(".");
		std::string filetype = audio.m_FileName.substr(fs);

		if (filetype != ".wav" || filetype != ".ogg")
		{
			TRE_CORE_ERROR("FMOD: Invalid File Type! Audio file is not a .wav or .ogg file. File not loaded");
			return;
		}

		std::ifstream ifs(m_FilePath);

		audio.m_ChannelGroup = m_MusicChannelGroup;

		if (!ifs.is_open())
		{
			TRE_CORE_ERROR("Unable to open audio file");
		}

		ErrorCheck(m_System->createSound(m_FilePath.c_str(), FMOD_DEFAULT, nullptr, &m_Sound), "FMOD: LoadFile()");
	}

	void AudioSystem::CreateChildChannelGroup(FMOD::ChannelGroup* child, std::string channelname)
	{
		child = nullptr;
		ErrorCheck(m_System->createChannelGroup(channelname.c_str(), &child), "FMOD: createChannelGroup()");
	}

	void AudioSystem::Play(Entity& go)
	{
		Audio& audio = go.get()->GetComponent<Audio>();
		if (audio.m_Play == true)
		{
			if (audio.m_Loop == false)
			{
				m_Channel->setMode(FMOD_LOOP_OFF);
			}
			else
			{
				m_Channel->setMode(FMOD_LOOP_NORMAL);
				m_Channel->setLoopCount(-1);
			};

			ErrorCheck(m_System->playSound(m_Sound, audio.m_ChannelGroup, audio.m_Pause, &m_Channel), "FMOD: playSound()");
		}
		else
		{
			TogglePause(go);
		}
	}

	void AudioSystem::TogglePause(Entity& go)
	{
		Audio& audio = go.get()->GetComponent<Audio>();
		audio.m_Pause = !audio.m_Pause;
		ErrorCheck(audio.m_ChannelGroup->setPaused(&audio.m_Pause), "FMOD: TogglePause()");
	}

	void AudioSystem::StopAudio(Entity& go)
	{
		Audio& audio = go.get()->GetComponent<Audio>();
		ErrorCheck(audio.m_ChannelGroup->stop(), "FMOD: StopAudio()");
	}

	int AudioSystem::ErrorCheck(FMOD_RESULT result, std::string function)
	{
		if (result != FMOD_OK) {
			TRE_CORE_ERROR(function);
			return 1;
		}
		return 0;
	}

	void AudioSystem::SetUp3DMode(Entity& go)
	{
		Audio& audio = go.get()->GetComponent<Audio>();
		AudioListener& listener = go->AddComponent<AudioListener>();
		Entity player;

		m_Channel->setMode(FMOD_3D);
		m_Channel->set3DMinMaxDistance(0.0f, 0.0f);
		m_Channel->set3DAttributes(&audio.m_goPosition, nullptr); //2nd param -> for doppler pitch shift

		listener.m_Position.x = m_ListenerPosition.x;
		listener.m_Position.y = m_ListenerPosition.y;
		listener.m_Position.z = m_ListenerPosition.z;

		m_System->set3DListenerAttributes(0, &listener.m_Position, nullptr, &listener.m_Forward, &listener.m_Up);
		Play(go);
	}

	void AudioSystem::CompileAudio(Entity& go)
	{
		Audio& audio = go.get()->GetComponent<Audio>();
		LoadFile(go);
		m_Channel->setVolume(audio.m_Volume);
		m_Channel->setPitch(audio.m_Pitch);
		m_Channel->setPriority(audio.m_Priority);

		if (audio.m_Spatialize)
		{
			SetUp3DMode(go);
		}
		else
		{
			Play(go);
		}

	}

	void AudioSystem::SetVolume(Entity& go, const float volume)
	{
		Audio& audio = go.get()->GetComponent<Audio>();
		audio.m_Volume = volume;
	}

	void AudioSystem::SetPitch(Entity& go, const float pitch)
	{
		Audio& audio = go.get()->GetComponent<Audio>();
		audio.m_Pitch = pitch;
	}

	void AudioSystem::SetPause(Entity& go, const bool pause)
	{
		Audio& audio = go.get()->GetComponent<Audio>();
		audio.m_Pause = pause;
	}

	void AudioSystem::SetLoop(Entity& go, const bool loop)
	{
		Audio& audio = go.get()->GetComponent<Audio>();
		audio.m_Loop = loop;
	}

	void AudioSystem::SetChannelGroup(Entity& go, FMOD::ChannelGroup* channelgroup)
	{
		Audio& audio = go.get()->GetComponent<Audio>();
		audio.m_ChannelGroup = channelgroup;
	}

	void AudioSystem::SetPriority(Entity& go, const int priority)
	{
		Audio& audio = go.get()->GetComponent<Audio>();
		audio.m_Priority = priority;
	}

	void AudioSystem::SetMute(Entity& go, const bool mute)
	{
		Audio& audio = go.get()->GetComponent<Audio>();
		audio.m_Mute = mute;
	}

	void AudioSystem::SetPlay(Entity& go, const bool play)
	{
		Audio& audio = go.get()->GetComponent<Audio>();
		audio.m_Play = play;
	}

	void AudioSystem::SetSpatialize(Entity& go, const bool spatialize)
	{
		Audio& audio = go.get()->GetComponent<Audio>();
		audio.m_Spatialize = spatialize;
	}

	float AudioSystem::GetVolume(Entity& go)
	{
		return go.get()->GetComponent<Audio>().m_Volume;
	}

	float AudioSystem::GetPitch(Entity& go)
	{
		return go.get()->GetComponent<Audio>().m_Pause;
	}

	bool AudioSystem::GetPause(Entity& go)
	{
		return go.get()->GetComponent<Audio>().m_Pause;
	}

	bool AudioSystem::GetLoop(Entity& go)
	{
		return go.get()->GetComponent<Audio>().m_Loop;
	}

	FMOD::ChannelGroup* AudioSystem::GetChannelGroup(Entity& go)
	{
		return go.get()->GetComponent<Audio>().m_ChannelGroup;
	}

	int AudioSystem::GetPriority(Entity& go)
	{
		return go.get()->GetComponent<Audio>().m_Priority;
	}

	bool AudioSystem::GetMute(Entity& go)
	{
		return go.get()->GetComponent<Audio>().m_Mute;
	}

	bool AudioSystem::GetPlay(Entity& go)
	{
		return go.get()->GetComponent<Audio>().m_Play;
	}

	bool AudioSystem::GetSpatialize(Entity& go)
	{
		return go.get()->GetComponent<Audio>().m_Spatialize;
	}


}
