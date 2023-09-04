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
		
	}

	AudioSystem::~AudioSystem()
	{
		Shutdown();
		m_System->release();
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

		m_ListenerPosition = player.get()->GetComponent<Transform>().m_Position;

	}

	void AudioSystem::OnDestroyGO()
	{

	}

	void AudioSystem::Shutdown()
	{
		m_System->release();
		m_Sound->release();
	}

	void AudioSystem::LoadFile(Entity& go) //(Entity& go, filepath)
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

	void AudioSystem::Play(Entity& go)
	{
		Audio& audio = go.get()->GetComponent<Audio>();
		if (audio.m_Play == true)
		{
			ErrorCheck(m_System->playSound(m_Sound, audio.m_ChannelGroup, audio.m_Pause, &m_Channel), "playSound()");
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
		ErrorCheck(audio.m_ChannelGroup->setPaused(&audio.m_Pause), "TogglePause()");
	}

	void AudioSystem::StopAudio(Entity& go)
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
		/*else
		{
			std::cout << "FMOD : " << function << " Success" << std::endl;
		}*/
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

	//void AudioSystem::SetAudioData(Audio* file)
	//{
	//	//placeholder data
	//	file->m_FileName = "ViveLeFromageBGM1";

	//	std::string file_path_{ "../Assets/Audio/" };
	//	std::string full_path = file_path_ + file->m_FileName;
	//	std::size_t fs = file->m_FileName.find_last_of(".");
	//	std::string filetype = file->m_File
	//Name.substr(fs);

	//	if (filetype != ".wav" || filetype != ".ogg")
	//	{
	//		std::cout << "[SetChannelName:] Invalid file type! " + file->m_FileName + " is not a .wav or .ogg file. Sound file not loaded." << std::endl;
	//		return;
	//	}

	//	file->m_FilePath = full_path;
	//	file->m_ChannelGroup = m_MusicChannelGroup;

	//}

}
