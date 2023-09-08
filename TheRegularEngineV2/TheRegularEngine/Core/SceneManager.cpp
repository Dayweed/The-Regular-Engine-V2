#include "pch.h"
#include "TREIncludes.h"
#include "FileSystem.h"

namespace TRE
{
	void SceneManager::NewScene()
	{
		ECSManager::Instance().DestroyAll();

		// Generate new Scene Name
		m_CurrentScene = SCENE_DEFAULT_NAME;
	}

	void SceneManager::LoadScene(std::string sceneName)
	{
		ECSManager::Instance().LoadEntities(GETFOLDER(FILESYS_SCENE) + sceneName + GETFILE(FILESYS_SCENE));
		m_CurrentScene = sceneName;
	}

	void SceneManager::SaveSceneAs(std::string sceneName)
	{
		ECSManager::Instance().SaveEntities(GETFOLDER(FILESYS_SCENE) + sceneName + GETFILE(FILESYS_SCENE));
		m_CurrentScene = sceneName;
	}

	void SceneManager::SaveScene()
	{
		ECSManager::Instance().SaveEntities(m_CurrentScene);
	}
}