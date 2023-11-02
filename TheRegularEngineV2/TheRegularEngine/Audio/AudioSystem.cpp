#include "pch.h"
#include "Core/Transform.h"
#include "AudioSystem.h"
#include "Core/GameLoop.h"
#include "InputHandler/InputHandler.h"

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
	}

	void AudioSystem::GameUpdate()
	{
		for (Entity& go : ECSManager::Instance().GetEntities<AudioListener>())
		{
			SetListenerPosition(go);
		}

		std::vector<Entity> entitiesToRemove;

		for (Entity& go : ECSManager::Instance().GetEntities<Audio>())
		{
			Audio& source = go->GetComponent<Audio>();
			//GetFileName(go);

			if (!source.m_HasCompiled)
			{
				CompileAudio(go);
				source.m_HasCompiled = true;
			}
			source.m_Channel->isPlaying(&source.m_isPlaying);

			if (source.m_PlayOnStart && source.m_Play)
			{
				Play(go, true);

			}
			else if (source.m_Loop)
			{
				if (!source.m_isPlaying)
				{
					Play(go, true);
				}
			}
			else
			{

			}

			if (!source.m_Play)
			{
				{
					source.m_Channel->stop();
				}

			}
			else
			{
				entitiesToRemove.push_back(go);
				auto it = soundToRemove.find(go);
				if (it != soundToRemove.end())
				{
					it->second->release();
					soundToRemove.erase(it);
				}
			}

			TogglePause(go);
			ToggleMute(go);
			source.m_Channel->setVolume(source.m_Volume);
			source.m_Channel->setPitch(source.m_Pitch);
			source.m_Channel->setPriority(source.m_Priority);

			/*for (auto go : audioMap)
			{
				if (go->HasComponent<Audio>())
				{
				}
			}*/
		}

		for (const Entity go : entitiesToRemove)
		{
			audioMap.erase(go);
		}

		m_System->update();

	}

	void AudioSystem::LateUpdate()
	{
		for (Entity& go : ECSManager::Instance().GetEntities<Audio>())
		{
			if (!GameLoop::Instance().IsGameRunning())
			{
				Audio& audio = go.get()->GetComponent<Audio>();
				audio.m_Channel->setPaused(true);
			}
		}
	}

	void AudioSystem::BeforeReset()
	{
		for (Entity& go : ECSManager::Instance().GetEntities<Audio>())
		{
			Audio& source = go->GetComponent<Audio>();
			Play(go, false);
			source.m_Channel->stop();
			m_SFXChannelGroup->stop();
			m_MusicChannelGroup->stop();
		}
		m_System->update();
	}

	void AudioSystem::AfterReset()
	{
		for (Entity& go : ECSManager::Instance().GetEntities<Audio>())
		{
			Audio& source = go->GetComponent<Audio>();
			Play(go, false);
			source.m_Channel->stop();
			m_SFXChannelGroup->stop();
			m_MusicChannelGroup->stop();
		}
		m_System->update();


		for (Entity& go : ECSManager::Instance().GetEntities<Audio>())
		{
			Audio& source = go->GetComponent<Audio>();
			source.m_HasCompiled = false;
		}
	}

	void AudioSystem::Init()
	{
		/*EventHandler::getEventHandlerInstance().subscribe(this, &GameLoop::ToggleRun);
		EventHandler::getEventHandlerInstance().subscribe(this, &GameLoop::Reset);*/

	}

	void AudioSystem::OnDestroyEntities()
	{

	}

	void AudioSystem::Shutdown()
	{

	}

	void AudioSystem::LoadFile(Entity& go) //(Entity& go, filepath)
	{
		Audio& audio = go.get()->GetComponent<Audio>();

		std::string file_path_{ "../Resources/Audio/" };
		std::string m_FilePath = file_path_ + audio.m_FileName;
		std::size_t fs = audio.m_FileName.find_last_of(".");
		std::string filetype = audio.m_FileName.substr(fs);

		if (filetype != ".wav")
		{
			TRE_CORE_ERROR("FMOD: Invalid File Type! Audio file is not a .wav file. File not loaded");
			return;
		}

		std::ifstream ifs(m_FilePath);

		if (!(ifs.is_open()))
		{
			TRE_CORE_ERROR("Unable to open audio file");
		}

		audio.m_Channel->setChannelGroup(m_MusicChannelGroup);

		ErrorCheck(m_System->createSound(m_FilePath.c_str(), FMOD_DEFAULT, nullptr, &audio.m_Sound), "FMOD: LoadFile()");
	}

	void AudioSystem::Load3DFile(Entity& go)
	{
		Audio& audio = go.get()->GetComponent<Audio>();

		std::string file_path_{ "../Resources/Audio/" };
		std::string m_FilePath = file_path_ + audio.m_FileName;
		std::size_t fs = audio.m_FileName.find_last_of(".");
		std::string filetype = audio.m_FileName.substr(fs);

		if (filetype != ".wav")
		{
			TRE_CORE_ERROR("FMOD: Invalid File Type! Audio file is not a .wav file. File not loaded");
			return;
		}

		std::ifstream ifs(m_FilePath);

		if (!ifs.is_open())
		{
			TRE_CORE_ERROR("Unable to open audio file");
		}

		audio.m_Channel->setChannelGroup(m_MusicChannelGroup);

		ErrorCheck(m_System->createSound(m_FilePath.c_str(), FMOD_3D, nullptr, &audio.m_Sound), "FMOD: LoadFile()");
	}

	void AudioSystem::Play(Entity& go, const bool shouldPlay)
	{
		Audio& audio = go.get()->GetComponent<Audio>();
		if (shouldPlay)
		{
			audio.m_Pause = false;
			audio.m_PlayOnStart = false;
			ErrorCheck(m_System->playSound(audio.m_Sound, audio.m_ChannelGroup, audio.m_Pause, &audio.m_Channel), "FMOD: playSound()");
		}
		else
		{
			audio.m_Pause = true;
		}
		SetSourcePosition(go);

		if (audio.m_Loop == false)
		{
			audio.m_Sound->setMode(FMOD_LOOP_OFF);
		}
		else
		{
			audio.m_Sound->setMode(FMOD_LOOP_NORMAL);
			audio.m_Sound->setLoopCount(-1);
		};

	}

	void AudioSystem::TogglePause(Entity& go)
	{
		Audio& audio = go.get()->GetComponent<Audio>();

		if (!audio.m_Pause)
		{
			audio.m_Channel->setPaused(false);
			audio.m_Pause = false;
		}
		else
		{
			audio.m_Channel->setPaused(true);
			audio.m_Pause = true;
		}
	}

	void AudioSystem::ToggleMute(Entity& go)
	{
		Audio& audio = go.get()->GetComponent<Audio>();
		if (!audio.m_Mute)
		{
			audio.m_Channel->setMute(false);
			audio.m_Mute = false;
		}
		else
		{
			audio.m_Channel->setMute(true);
			audio.m_Mute = true;
		}
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

	void AudioSystem::CompileAudio(Entity& go)
	{
		Audio& audio = go.get()->GetComponent<Audio>();

		if (audio.m_FileName == "") {
			TRE_CORE_WARN("Audio filename is not set. Skipping audio compilation.");
			return;
		}
		else if (audio.m_Spatialize)
		{
			Load3DFile(go);
		}
		else
		{
			LoadFile(go);
		}

		//audioMap.insert(go);
		//soundToRemove.insert({ go, audio.m_Sound });
	}

	void AudioSystem::SetFileName(Entity& go, const std::string filename)
	{
		Audio& audio = go.get()->GetComponent<Audio>();
		audio.m_FileName = filename;
	}

	void AudioSystem::SetChannelGroup(Entity& go, const std::string channel)
	{
		Audio& audio = go.get()->GetComponent<Audio>();
		if (channel == "Music")
		{
			audio.m_ChannelGroup = m_MusicChannelGroup;
		}
		else if (channel == "SFX")
		{
			audio.m_ChannelGroup = m_SFXChannelGroup;
		}

	}

	void AudioSystem::SetListenerPosition(Entity& go)
	{
		AudioListener& listener = go.get()->GetComponent<AudioListener>();

		const glm::vec3 pos = go->GetComponent<Transform>().m_Position;
		const glm::vec3 viewVec = go->GetComponent<Camera>().m_BaseCamera.GetViewDirection();
		const glm::vec3 upVec = go->GetComponent<Camera>().m_BaseCamera.GetUpVec();

		listener.m_Position = glmVec3ToFmodVector(pos);
		listener.m_Forward = glmVec3ToFmodVector(viewVec);
		listener.m_Up = glmVec3ToFmodVector(upVec);

		m_System->set3DListenerAttributes(0, &listener.m_Position, nullptr, nullptr, nullptr);
	}

	void AudioSystem::SetSourcePosition(Entity& go)
	{
		Transform& sourceposition = go.get()->GetComponent<Transform>();
		Audio& audiosource = go.get()->GetComponent<Audio>();
		audiosource.m_goPosition = glmVec3ToFmodVector(sourceposition.m_Position);

		audiosource.m_Channel->set3DMinMaxDistance(audiosource.m_MinDistance, audiosource.m_MaxDistance);
		audiosource.m_Channel->setMode(FMOD_3D);
		audiosource.m_Channel->set3DAttributes(&audiosource.m_goPosition, nullptr); //2nd param -> for doppler pitch shift
	}

	void AudioSystem::SetSourceRadius(Entity& go, const float min, const float max)
	{
		Audio& audiosource = go.get()->GetComponent<Audio>();
		audiosource.m_Channel->set3DMinMaxDistance(min, max);
		audiosource.m_MinDistance = min;
		audiosource.m_MaxDistance = max;
	}

	FMOD::ChannelGroup* AudioSystem::GetChannelGroup(Entity& go)
	{
		return go.get()->GetComponent<Audio>().m_ChannelGroup;
	}

	std::string AudioSystem::GetFileName(Entity& go) const
	{
		Audio& audio = go.get()->GetComponent<Audio>();

		for (const auto& entry : std::filesystem::directory_iterator("../Resources/Audio/")) {
			std::string filename = entry.path().filename().string();
			audio.m_audioFiles.push_back(filename);
			return audio.m_FileName = filename;
		}

		return {};
	}

	FMOD_VECTOR AudioSystem::GetListenerPosition(Entity& go) const
	{
		return go.get()->GetComponent<AudioListener>().m_Position;
	}

	FMOD_VECTOR AudioSystem::GetSourcePosition(Entity& go) const
	{
		return go.get()->GetComponent<Audio>().m_goPosition;
	}

	const std::pair<float, float> AudioSystem::GetSourceRadius(Entity& go) const
	{
		const Audio& audioSource = go.get()->GetComponent<Audio>();
		std::pair<float, float> radius;
		radius.first = audioSource.m_MinDistance;
		radius.second = audioSource.m_MaxDistance;
		return radius;
	}

	bool AudioSystem::GetIsPlaying(Entity& go) const
	{
		Audio& source = go.get()->GetComponent<Audio>();
		return source.m_Channel->isPlaying(&source.m_isPlaying);
	}

}