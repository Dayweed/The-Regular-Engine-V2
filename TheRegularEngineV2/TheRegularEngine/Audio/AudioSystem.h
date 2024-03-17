#pragma once
#include "ECS/System.h"
#include "FMOD/fmod.hpp"
#include "TREIncludes.h"
#include "Properties.h"
#include <map>

namespace TRE
{
	class AudioSystem : public ECSSystem
	{
	public:
		AudioSystem();
		~AudioSystem() override;

		static AudioSystem& Instance()
		{
			static AudioSystem instance;
			return instance;
		}

		void Init() override;
		void GameUpdate() override;
		void LateUpdate() override;
		void BeforeReset() override;
		void AfterReset() override;
		void OnDestroyEntities() override;
		void Shutdown() override;

		void ForcePause(); // Force pause all objects with audio

		//void SetAudioData(Audio* file);
		void LoadFile(Entity& go);
		void Load3DFile(Entity& go); //-----
		//void CreateChildChannelGroup(FMOD::ChannelGroup* child, std::string name);
		void Play(Entity& go, const bool shouldPlay);
		void TogglePause(Entity& go);
		void ToggleMute(Entity& go);
		void Stop(Entity& go);
		void CompileAudio(Entity& go);
		void MuteAll();
		void CompileFootstepsSounds(Entity& go);
		void PlayFootsteps(Entity& go);
		void PlayFootsteps(Entity& go, int i);

		int ErrorCheck(FMOD_RESULT result, std::string function);

		void SetFileName(Entity& go, const std::string filename);
		void SetChannelGroup(Entity& go, const std::string channel);
		void SetListenerPosition(Entity& go);
		void SetSourcePosition(Entity& go);
		float Calculate3DVolume(float distance, float minDistance, float maxDistance, float maxVolume);
		void SetSourceRadius(Entity& go, const float min, const float max);
		bool GetIsPlaying(Entity& go) const;

		void SetVolume(Entity& go, const float volume);
		float GetVolume(Entity& go) const;
		void SetMasterVolume(const float volume);
		float GetMasterVolume() const;
		void SetBGMVolume(const float volume);
		float GetBGMVolume() const;
		void SetSFXVolume(const float volume);
		float GetSFXVolume() const;

		FMOD_VECTOR GetListenerPosition(Entity& go) const;
		FMOD_VECTOR GetSourcePosition(Entity& go) const;

		const std::pair<float, float> GetSourceRadius(Entity& go) const;


		FMOD::ChannelGroup* GetChannelGroup(Entity& go);
		std::string GetFileName(Entity& go) const;


		FMOD_VECTOR glmVec3ToFmodVector(const glm::vec3& glmVector)
		{
			FMOD_VECTOR fmodVector;
			fmodVector.x = glmVector.x;
			fmodVector.y = glmVector.y;
			fmodVector.z = glmVector.z;
			return fmodVector;
		}

		bool m_MuteAll{ true };

	private:
		FMOD::System* m_System = nullptr;

		FMOD::ChannelGroup* m_SFXChannelGroup = nullptr;
		FMOD::ChannelGroup* m_MusicChannelGroup = nullptr;

		const int MAX_CHANNELS = 64;
		
		std::unordered_set<Entity> audioMap;
		std::unordered_map<Entity, FMOD::Sound*> soundToRemove;

		float m_MasterVolume = 1.f;
		float m_BGMVolume = 1.f;
		float m_SFXVolume = 1.f;
	};

}