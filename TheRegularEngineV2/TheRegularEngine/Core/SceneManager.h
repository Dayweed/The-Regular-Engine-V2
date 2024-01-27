#pragma once
/*!
	@file		SceneManager.h
	@author		Isaiah Lim (Code Contribution 100%)
	@email		lim.i@digipen.edu
	@date		08/09/2023
	@brief		Handles all the front end of handling scenes

	Copyright (C) 2023 DigiPen Institute of Technology.
	Reproduction or disclosure of this file or its contents without the
	prior written consent of DigiPen Institute of Technology is prohibited.
************************************************************************/

#include "System.h"
#include "FileSystem.h"

#define SCENE_DEFAULT_NAME "New Scene"
#define FILESYS_SCENE_TYPE ".json"

namespace TRE
{
	class SceneManager
	{
		friend class ECSManager;

	public:
		static SceneManager& Instance()
		{
			static SceneManager instance;
			return instance;
		}

		/* !
		@function		NewScene
		@author			Isaiah Lim (lim.i@digipen.edu)

		@brief			Creates a new scene

		[Warning]		Will delete all the entities! Remember to save first!
		*//*__________________________________________________________________________*/
		void NewScene(std::string sceneName = SCENE_DEFAULT_NAME);

		/* !
		@function		LoadScene
		@author			Isaiah Lim (lim.i@digipen.edu)

		@params			sceneName	Name of the filepath for the scene to be load at

		@brief			Load a new scene based on the filepath

		[Warning]		Will delete all the entities! Remember to save first!
		*//*__________________________________________________________________________*/
		void LoadScene(std::string scenePath);

		/* !
		@function		SaveSceneAs
		@author			Isaiah Lim (lim.i@digipen.edu)

		@params			sceneName	Name of the filepath for the scene to be save at

		@brief			Save the scene based on the filepath
		*//*__________________________________________________________________________*/
		void SaveSceneAs(std::string scenePath);

		/* !
		@function		SaveScene
		@author			Isaiah Lim (lim.i@digipen.edu)

		@brief			Save the scene based on the current scene name
		*//*__________________________________________________________________________*/
		void SaveScene();

		std::string GetCurrentSceneName();
		bool SceneExistInFile();

	private:
		friend class GameLoop;

		// Delete possible copy ctor and assignment to ensure singleton
		SceneManager() {};
		SceneManager(SceneManager const&) = delete;
		void operator=(SceneManager const&) = delete;
		void* operator new(size_t) = delete;

		std::string GetSceneName(std::string filePath);

		//std::pair<std::string, std::string> m_CurrentScene; // Filepath, Scene Name
		//std::map<std::string, std::string> m_Scenes; // Filepath, Scene Name

		std::string m_CurrentScene{ SCENE_DEFAULT_NAME };
		std::string m_CurrentSceneFilePath{ GETFOLDER(FILESYS_SCENE) + SCENE_DEFAULT_NAME + GETFILE(FILESYS_SCENE) };
		int m_DupDefaultName{};

		bool m_NewScene{ true }; // Checks if this scene has been saved before
	};

	class ScenePostEffectsSystem : public ECSSystem
	{
	public:
		// Handles index in m_TransitonStateArray
		enum TransitionTypeIndex
		{
			TYPE_VIGNETTE,
			TYPE_FADE,
			TYPE_SIZE
		};

		enum TransitionState
		{
			STATE_NONE,
			STATE_IN,
			STATE_GOINGIN,
			STATE_OUT,
			STATE_GOINGOUT,
			STATE_SIZE
		};

		ScenePostEffectsSystem() = default;
		~ScenePostEffectsSystem() = default;

		void Init() override;
		void Update() override;

		TransitionState GetTransitionState(TransitionTypeIndex index);

		void TransitionToScene(const std::string& sceneName, const float totalDuration);
		void VignetteShrink(const float totalDuration);

	private:
		// Vignette Effect
		void VignetteCalc();	// Returns if it is closing or opening

		struct Transition
		{
			TransitionState m_State;

			float m_Duration;
			float m_HalfDuration;
			float m_ElapsedTime;

			bool m_IsTransitioning;

			bool m_LoadedNewScene;
			std::string m_TransitionSceneName;
		};

		Transition m_Transitions[2];
	};
}