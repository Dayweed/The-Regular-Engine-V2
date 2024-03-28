#include "pch.h"
#include "ECS/Components/Transform.h"
#include "AudioSystem.h"
#include "Core/GameLoop.h"
#include "InputHandler/InputHandler.h"
#include "ECS/Components/AudioComponent.h"
#include "ECS/Components/AudioListenerComponent.h"

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

			SetSourcePosition(go);

			if (!source.m_HasCompiled)
			{
				CompileAudio(go);
				CompileFootstepsSounds(go);
				for (auto name : source.footstepsSounds)
				{
					std::cout << name.m_File << std::endl;
				}
				source.m_HasCompiled = true;
			}
		}

		for (auto go : audioMap)
		{
			if (go->HasComponent<Audio>())
			{
				Audio& source = go->GetComponent<Audio>();

				if (source.m_PlayOnStart && source.m_Play)
				{
					Play(go, true);
				}
				else if (!source.m_Play && !source.m_Loop)
				{
					source.m_Channel->stop();
				}

				source.m_Channel->isPlaying(&source.m_isPlaying);
				TogglePause(go);
				ToggleMute(go);
				source.m_Channel->setPitch(source.m_Pitch);
				source.m_Channel->setPriority(source.m_Priority);

				if (!source.m_Spatialize)
				{
					if (source.m_ChannelGroup == m_MusicChannelGroup)
					{
						source.m_Channel->setVolume(source.m_Volume * m_BGMVolume * m_MasterVolume);
					}
					else if (source.m_ChannelGroup == m_SFXChannelGroup)
					{
						source.m_Channel->setVolume(source.m_Volume * m_SFXVolume * m_MasterVolume);
					}
					else
					{
						source.m_Channel->setVolume(source.m_Volume * m_MasterVolume);
					}
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
				Audio& audio = go->GetComponent<Audio>();
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

	}

	void AudioSystem::OnDestroyEntities()
	{

	}

	void AudioSystem::Shutdown()
	{

	}

	void AudioSystem::ForcePause()
	{
		for (Entity& go : ECSManager::Instance().GetEntities<Audio>())
		{
			Audio& audio = go->GetComponent<Audio>();
			audio.m_Channel->setPaused(true);
		}
	}

	void AudioSystem::LoadFile(Entity& go) //(Entity& go, filepath)
	{
		Audio& audio = go->GetComponent<Audio>();

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

		//audio.m_Channel->setChannelGroup(m_MusicChannelGroup);

		ErrorCheck(m_System->createSound(m_FilePath.c_str(), FMOD_DEFAULT, nullptr, &audio.m_Sound), "FMOD: LoadFile()");
	}

	void AudioSystem::Load3DFile(Entity& go)
	{
		Audio& audio = go->GetComponent<Audio>();

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

		//audio.m_Channel->setChannelGroup(m_MusicChannelGroup);

		ErrorCheck(m_System->createSound(m_FilePath.c_str(), FMOD_3D, nullptr, &audio.m_Sound), "FMOD: LoadFile()");
	}

	void AudioSystem::Play(Entity& go, const bool shouldPlay)
	{
		Audio& audio = go->GetComponent<Audio>();

		//std::cout << "Playing: " << audio.m_FileName << std::endl;

		if (audio.m_isPlaying || !shouldPlay)
			return;

		if (audio.m_Loop == false)
		{
			audio.m_Sound->setMode(FMOD_LOOP_OFF);
			ErrorCheck(m_System->playSound(audio.m_Sound, audio.m_ChannelGroup, audio.m_Pause, &audio.m_Channel), "FMOD: playSound()" + audio.m_FileName);
			audio.m_Play = false;
		}
		else
		{
			audio.m_Sound->setMode(FMOD_LOOP_NORMAL);
			audio.m_Sound->setLoopCount(-1);
			ErrorCheck(m_System->playSound(audio.m_Sound, audio.m_ChannelGroup, audio.m_Pause, &audio.m_Channel), "FMOD: playSound()" + audio.m_FileName);
		};

		audio.m_Channel->setPaused(false);
		audio.m_isPlaying = true;
	}

	void AudioSystem::TogglePause(Entity& go)
	{
		Audio& audio = go->GetComponent<Audio>();

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
		Audio& audio = go->GetComponent<Audio>();
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

	void AudioSystem::Stop(Entity& go)
	{
		Audio& audio = go->GetComponent<Audio>();
		audio.m_Channel->stop();
		audio.m_isPlaying = false;
		audio.m_Play = false;
	}

	int AudioSystem::ErrorCheck(FMOD_RESULT result, std::string function)
	{
		if (result != FMOD_OK)
		{
			TRE_CORE_ERROR(function);

			return 1;
		}
		return 0;
	}

	void AudioSystem::CompileAudio(Entity& go)
	{
		Audio& audio = go->GetComponent<Audio>();

		if (audio.m_FileName == "") {
			TRE_CORE_WARN("Audio filename is not set. Skipping audio compilation.");
			return;
		}
		else if (audio.m_Spatialize)
		{
			Load3DFile(go);
			SetSourcePosition(go);
		}
		else
		{
			LoadFile(go);
		}
		audio.m_Channel->setVolume(audio.m_Volume);
		if (audio.m_FileName.find("sfx_") != std::string::npos || audio.m_FileName.find("SFX_") != std::string::npos)
		{
			audio.m_Channel->setChannelGroup(m_SFXChannelGroup);
			audio.m_ChannelGroup = m_SFXChannelGroup;
		}
		else if (audio.m_FileName.find("bgm_") != std::string::npos || audio.m_FileName.find("BGM_") != std::string::npos)
		{
			audio.m_Channel->setChannelGroup(m_MusicChannelGroup);
			audio.m_ChannelGroup = m_MusicChannelGroup;
		}
		audioMap.insert(go);
		soundToRemove.insert({ go, audio.m_Sound });
	}

	void AudioSystem::MuteAll()
	{
		m_MuteAll = !m_MuteAll;

		for (Entity& go : ECSManager::Instance().GetEntities<Audio>())
		{ 
			Audio& audio = go->GetComponent<Audio>();
			if (m_MuteAll)
			{
				audio.m_Channel->setMute(true);
				audio.m_Mute = true;
			}
			else
			{
				audio.m_Channel->setMute(false);
				audio.m_Mute = false;
			}
		}
	}

	void AudioSystem::CompileFootstepsSounds(Entity& go)
	{
		if (!go->HasComponent<Audio>()) return;

		Audio& audio{ go->GetComponent<Audio>() };

		audio_file_dropdown dropdown;

		for(auto fs : audio.footstepsSounds)
		{
			if (fs.m_File == audio.m_FileName)
			{
				TRE_CORE_WARN("Footsteps sound already exists in the list");
				return;
			}
			else
			{
				dropdown.m_File = audio.m_FileName;
				audio.footstepsSounds.push_back(dropdown);

			}
		}
	}

	//void AudioSystem::PlayFootsteps(Entity& go)
	//{
	//	if (!go->HasComponent<Audio>()) return;

	//	Audio& audio{ go->GetComponent<Audio>() };

	//	if (audio.footstepsSounds.empty()) return;

	//	int i = rand() % audio.footstepsSounds.size();

	//	std::cout << i << std::endl;

	//	std::cout << "Playing: " << audio.footstepsSounds[i].m_File << std::endl;

	//	const std::string& soundFilepath = "../Resources/Audio/" + audio.footstepsSounds[i].m_File;

	//	FMOD::Sound* sound;
	//	ErrorCheck(m_System->createSound(soundFilepath.c_str(), FMOD_DEFAULT, nullptr, &sound), "FMOD: createSound()");

	//	FMOD::Channel* channel;

	//	ErrorCheck(m_System->playSound(sound, m_SFXChannelGroup, false, &channel), "FMOD: playSound()");
	//}

	void AudioSystem::PlayFootsteps(Entity& go, int i)
	{
		if (!go->HasComponent<Audio>()) return;

		Audio& audio{ go->GetComponent<Audio>() };

		if (audio.footstepsSounds.empty()) return;

		std::cout << "Playing: " << audio.footstepsSounds[i].m_File << std::endl;

		audio.m_FileName = audio.footstepsSounds[i].m_File;
		const std::string& soundFilepath = "../Resources/Audio/" + audio.m_FileName;

		FMOD::Sound* sound;
		ErrorCheck(m_System->createSound(soundFilepath.c_str(), FMOD_DEFAULT, nullptr, &sound), "FMOD: createSound()");

		FMOD::Channel* channel;

		ErrorCheck(m_System->playSound(sound, m_SFXChannelGroup, false, &channel), "FMOD: playSound()");

	}

	void AudioSystem::SetFileName(Entity& go, const std::string filename)
	{
		Audio& audio = go->GetComponent<Audio>();
		audio.m_FileName = filename;
	}

	void AudioSystem::SetChannelGroup(Entity& go, const std::string channel)
	{
		Audio& audio = go->GetComponent<Audio>();
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
		AudioListener& listener = go->GetComponent<AudioListener>();

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
		Transform& sourceposition = go->GetComponent<Transform>();
		Audio& audiosource = go->GetComponent<Audio>();
		audiosource.m_goPosition = glmVec3ToFmodVector(sourceposition.m_Position);
		
		if (audiosource.m_Spatialize)
		{
			for (Entity& other_go : ECSManager::Instance().GetEntities<AudioListener>())
			{
				const AudioListener& listener = other_go->GetComponent<AudioListener>();

				FMOD_VECTOR listenerPos = listener.m_Position;
				const FMOD_VECTOR sourcePos = glmVec3ToFmodVector(sourceposition.m_Position);

				m_System->get3DListenerAttributes(0, &listenerPos, nullptr, nullptr, nullptr);
				const float distance = sqrtf(powf(listenerPos.x - sourcePos.x, 2) + powf(listenerPos.y - sourcePos.y, 2) + powf(listenerPos.z - sourcePos.z, 2));

				const float volume = Calculate3DVolume(distance, audiosource.m_MinDistance, audiosource.m_MaxDistance, audiosource.m_Volume);

				audiosource.m_Channel->setVolume(volume * m_MasterVolume);

				audiosource.m_goPosition = glmVec3ToFmodVector(sourceposition.m_Position);
				audiosource.m_Channel->set3DMinMaxDistance(audiosource.m_MinDistance, audiosource.m_MaxDistance);
				audiosource.m_Channel->setMode(FMOD_3D);
				audiosource.m_Channel->set3DAttributes(&audiosource.m_goPosition, nullptr);
			}
		}
		//audiosource.m_Channel->set3DMinMaxDistance(audiosource.m_MinDistance, audiosource.m_MaxDistance);
		//audiosource.m_Channel->setMode(FMOD_3D);
		//audiosource.m_Channel->set3DAttributes(&audiosource.m_goPosition, nullptr); //2nd param -> for doppler pitch shift

		//std::cout << "Source Position: " << audiosource.m_goPosition.x << ", " << audiosource.m_goPosition.y << ", " << audiosource.m_goPosition.z << std::endl;
	}

	float AudioSystem::Calculate3DVolume(float distance, float minDistance, float maxDistance, float maxVolume)
	{
		distance = std::clamp(distance, minDistance, maxDistance);

		float volume = maxVolume * (1.0f - (distance - minDistance) / (maxDistance - minDistance));

		// Ensure volume is within the range [0, maxVolume]
		return std::max(0.0f, std::min(maxVolume, volume));
	}

	void AudioSystem::SetSourceRadius(Entity& go, const float min, const float max)
	{
		Audio& audiosource = go->GetComponent<Audio>();
		audiosource.m_Channel->set3DMinMaxDistance(min, max);
		audiosource.m_MinDistance = min;
		audiosource.m_MaxDistance = max;

		//std::cout << "Source Radius: " << audiosource.m_MinDistance << ", " << audiosource.m_MaxDistance << std::endl;
	}

	void AudioSystem::SetVolume(Entity& go, const float volume)
	{
		Audio& audio = go->GetComponent<Audio>();
		audio.m_Volume = std::clamp(volume, 0.0f, 1.0f);
		audio.m_Channel->setVolume(audio.m_Volume);
	}

	void AudioSystem::SetMasterVolume(const float volume)
	{
		m_MasterVolume = volume;
		m_MasterVolume = std::clamp(m_MasterVolume, 0.0f, 1.0f);
	}

	void AudioSystem::SetBGMVolume(const float volume)
	{
		m_BGMVolume = volume;
		m_BGMVolume = std::clamp(m_BGMVolume, 0.0f, 1.0f);
	}

	void AudioSystem::SetSFXVolume(const float volume)
	{
		m_SFXVolume = volume;
		m_SFXVolume = std::clamp(m_SFXVolume, 0.0f, 1.0f);
	}

	FMOD::ChannelGroup* AudioSystem::GetChannelGroup(Entity& go)
	{
		return go->GetComponent<Audio>().m_ChannelGroup;
	}

	std::string AudioSystem::GetFileName(Entity& go) const
	{
		// The code below was producing C4702: "unreachable code" due to the fact that the function was returning
		// immediately on its first iteration, causing the increment part of the range-for loop to never execute.
#if 0
		Audio& audio = go->GetComponent<Audio>();

		for (const auto& entry : std::filesystem::directory_iterator("../Resources/Audio/")) {
			std::string filename = entry.path().filename().string();
			audio.m_audioFiles.push_back(filename);
			return audio.m_FileName = filename;
		}

		return {};
#else
		Audio& audio = go->GetComponent<Audio>();

		const std::filesystem::directory_iterator directory("../Resources/Audio/");

		// return an empty string immediately if the directory is empty
		if (std::filesystem::begin(directory) == std::filesystem::end(directory))
		{
			audio.m_audioFiles.emplace_back();
			return audio.m_FileName = std::string{};
		}

		// otherwise, obtain the first entry's filename
		const std::string filename = directory->path().filename().string();
		audio.m_audioFiles.push_back(filename);
		return audio.m_FileName = filename;
#endif
		}

	FMOD_VECTOR AudioSystem::GetListenerPosition(Entity& go) const
	{
		return go->GetComponent<AudioListener>().m_Position;
	}

	FMOD_VECTOR AudioSystem::GetSourcePosition(Entity& go) const
	{
		return go->GetComponent<Audio>().m_goPosition;
	}

	const std::pair<float, float> AudioSystem::GetSourceRadius(Entity& go) const
	{
		const Audio& audioSource = go->GetComponent<Audio>();
		std::pair<float, float> radius;
		radius.first = audioSource.m_MinDistance;
		radius.second = audioSource.m_MaxDistance;
		return radius;
	}

	bool AudioSystem::GetIsPlaying(Entity& go) const
	{
		Audio& source = go->GetComponent<Audio>();
		return source.m_Channel->isPlaying(&source.m_isPlaying);

		//std::cout << "IsPlaying: " << source.m_isPlaying << source.m_FileName << std::endl;
	}

	float AudioSystem::GetVolume(Entity& go) const
	{
		return go->GetComponent<Audio>().m_Volume;
	}

	float AudioSystem::GetMasterVolume() const
	{
		return m_MasterVolume;
	}

	float AudioSystem::GetBGMVolume() const
	{
		return m_BGMVolume;
	}

	float AudioSystem::GetSFXVolume() const
	{
		return m_SFXVolume;
	}
}