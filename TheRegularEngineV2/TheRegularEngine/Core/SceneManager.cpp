#include "pch.h"
#include "TREIncludes.h"
#include "FileSystem.h"
#include "Resource/ResourceManager.h"

namespace TRE
{
	void SceneManager::NewScene()
	{
		ECSManager::Instance().DestroyAll();

		Entity MainCamera = ECSManager::Instance().CreateEntity("Main Camera");
		MainCamera->AddComponent<Camera>();
		ECSSystemManager::Instance().GetSystem<CameraSystem>()->SetIsMainCamera(MainCamera, true);

		// Generate new Scene Name
		m_CurrentScene = SCENE_DEFAULT_NAME;
		m_CurrentSceneFilePath = GETFOLDER(FILESYS_SCENE) + m_CurrentScene + GETFILE(FILESYS_SCENE);
	}

	void SceneManager::LoadScene(std::string scenePath)
	{
		ECSManager::Instance().LoadEntities(scenePath);
		m_CurrentScene = scenePath;
		m_CurrentSceneFilePath = scenePath;
	}

	void SceneManager::SaveSceneAs(std::string scenePath)
	{
		ECSManager::Instance().SaveEntities(scenePath);
		ResourceManager::Instance().Serialize();
		m_CurrentScene = scenePath;
		m_CurrentSceneFilePath = scenePath;
	}

	void SceneManager::SaveScene()
	{
		SaveSceneAs(m_CurrentSceneFilePath);
	}
}