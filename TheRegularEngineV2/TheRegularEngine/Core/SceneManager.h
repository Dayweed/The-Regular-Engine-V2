#pragma once
#include "pch.h"

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

		void NewScene();
		void LoadScene(std::string filepath);
		void SaveScene(std::string scenename);

	private:
		// Delete possible copy ctor and assignment to ensure singleton
		SceneManager() {};
		SceneManager(SceneManager const&) = delete;
		void operator=(SceneManager const&) = delete;
		void* operator new(size_t) = delete;

		std::pair<std::string, std::string> m_CurrentScene; // Filepath, Scene Name
		std::map<std::string, std::string> m_Scenes; // Filepath, Scene Name
	};
	static SceneManager* _scene_manager{ &SceneManager::Instance() };
}