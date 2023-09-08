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

#define SCENE_DEFAULT_NAME "New Scene"

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
		void NewScene();

		/* !
		@function		LoadScene
		@author			Isaiah Lim (lim.i@digipen.edu)

		@params			sceneName	Name of the scene to be load as

		@brief			Load a new scene based on the scene name

		[Warning]		Will delete all the entities! Remember to save first!
		*//*__________________________________________________________________________*/
		void LoadScene(std::string sceneName);

		/* !
		@function		SaveSceneAs
		@author			Isaiah Lim (lim.i@digipen.edu)

		@params			sceneName	Name of the scene to be save as

		@brief			Save the scene based on the scene name
		*//*__________________________________________________________________________*/
		void SaveSceneAs(std::string sceneName);

		/* !
		@function		SaveScene
		@author			Isaiah Lim (lim.i@digipen.edu)

		@brief			Save the scene based on the current scene name
		*//*__________________________________________________________________________*/
		void SaveScene();

	private:
		// Delete possible copy ctor and assignment to ensure singleton
		SceneManager() {};
		SceneManager(SceneManager const&) = delete;
		void operator=(SceneManager const&) = delete;
		void* operator new(size_t) = delete;

		//std::pair<std::string, std::string> m_CurrentScene; // Filepath, Scene Name
		//std::map<std::string, std::string> m_Scenes; // Filepath, Scene Name

		std::string m_CurrentScene{ SCENE_DEFAULT_NAME };
		int m_DupDefaultName{};
	};
	static SceneManager* _scene_manager{ &SceneManager::Instance() };
}