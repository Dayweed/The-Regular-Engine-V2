#pragma once

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

		void NewScene();
		void LoadScene(std::string sceneName);
		void SaveSceneAs(std::string sceneName);
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