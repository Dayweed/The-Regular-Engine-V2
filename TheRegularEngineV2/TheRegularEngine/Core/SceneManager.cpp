#include "pch.h"
#include "TREIncludes.h"
#include "Resource/ResourceManager.h"

namespace TRE
{
	void SceneManager::NewScene(std::string sceneName)
	{
		ECSManager::Instance().DestroyAll();

		Entity MainCamera = ECSManager::Instance().CreateEntity("Main Camera");
		MainCamera->AddComponent<Camera>();
		ECSSystemManager::Instance().GetSystem<CameraSystem>()->SetIsMainCamera(MainCamera, true);

		// Generate new Scene Name
		m_CurrentScene = sceneName;
		m_CurrentSceneFilePath = GETFOLDER(FILESYS_SCENE) + sceneName + GETFILE(FILESYS_SCENE);
	}

	void SceneManager::LoadScene(std::string scenePath)
	{
		if (std::filesystem::exists(scenePath))
		{
			ECSManager::Instance().LoadEntities(scenePath);
			m_CurrentScene = scenePath;
			m_CurrentSceneFilePath = scenePath;
		}
		else
		{
			TRE_CORE_CRITICAL("Scene file not found!");
			return;
		}
	}

	void SceneManager::SaveSceneAs(std::string scenePath)
	{
		ECSManager::Instance().SaveEntities(scenePath);
		ResourceManager::Instance().SerializeAll();
		m_CurrentScene = scenePath;
		m_CurrentSceneFilePath = scenePath;
	}

	void SceneManager::SaveScene()
	{
		SaveSceneAs(m_CurrentSceneFilePath);
	}

	std::string SceneManager::GetCurrentSceneName()
	{
		return m_CurrentScene;
	}
}