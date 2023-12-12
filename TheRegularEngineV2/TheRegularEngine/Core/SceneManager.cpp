#include "pch.h"
#include "TREIncludes.h"
#include "Resource/ResourceManager.h"
#include "GameLoop.h"
#include "PostProcessing/Vignette.h"

#include "Scripting/ScriptEngine.h"

namespace TRE
{
	void SceneManager::NewScene(std::string sceneName)
	{
		ECSSystemManager::Instance().BeforeReset();
		ECSManager::Instance().DestroyAll();

		Entity MainCamera = ECSManager::Instance().CreateEntity("Main Camera");
		MainCamera->AddComponent<Camera>();
		ECSSystemManager::Instance().GetSystem<CameraSystem>()->SetIsMainCamera(MainCamera, true);

		Entity MainLight = ECSManager::Instance().CreateEntity("Directional Light");
		MainLight->AddComponent<DirectionalLight>();
		MainLight->GetComponent<Transform>().m_Rotation = glm::vec3(45.0f, 45.0f, 0.0f);
		MainLight->GetComponent<Transform>().m_IsDirty = true;

		// Generate new Scene Name
		m_CurrentScene = sceneName;
		m_CurrentSceneFilePath = GETFOLDER(FILESYS_SCENE) + sceneName + GETFILE(FILESYS_SCENE);

		ECSSystemManager::Instance().AfterReset();
	}

	void SceneManager::LoadScene(std::string scenePath)
	{
		if (GameLoop::Instance().GetDisplayingPrefab())
		{
			ECSSystemManager::Instance().GetSystem<PrefabSystem>()->ReturnToScene();
		}

		ECSSystemManager::Instance().BeforeReset();
		ECSManager::Instance().DestroyAll();

		if (std::filesystem::exists(scenePath))
		{
			ECSManager::Instance().LoadEntities(scenePath);

			// Update all Prefabs
			ECSSystemManager::Instance().GetSystem<PrefabSystem>()->CheckAndUpdateInstances();

			m_CurrentScene = GetSceneName(scenePath);
			m_CurrentSceneFilePath = scenePath;
		}
		else
		{
			TRE_CORE_CRITICAL("Scene file not found!");
			return;
		}

		ScriptEngine::ReloadAssembly();
		ECSSystemManager::Instance().AfterReset(); 

		//ResourceManager::Instance().UnloadUnusedResources();
	}

	void SceneManager::SaveSceneAs(std::string scenePath)
	{
		ECSManager::Instance().SaveEntities(scenePath);
		ResourceManager::Instance().SerializeAll();
		m_CurrentScene = GetSceneName(scenePath);
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

	std::string SceneManager::GetSceneName(std::string filePath)
	{
		std::string sceneName = filePath.substr(filePath.find_last_of('\\') + 1);
		sceneName.erase(sceneName.find(FILESYS_SCENE_TYPE));
		return sceneName;
	}

	void SceneTransitioner::Init()
	{
		m_Duration = 0.0f;
		m_HalfDuration = 0.0f;
		m_ElapsedTime = 0.0f;
		m_IsTransitioning = false;
		m_LoadedNewScene = false;

		m_TransitionSceneName = "";
	}

	void SceneTransitioner::Update()
	{
		if (m_IsTransitioning)
		{
			if (m_ElapsedTime < m_Duration)
			{
				auto vignette = PostProcessingManager::Instance().GetPostEffect<Vignette>("Vignette");
				//Close vignette
				if (m_ElapsedTime < m_HalfDuration)
				{
					vignette->SetRadius(1.0f - (m_ElapsedTime / m_HalfDuration));
				}
				//Open vignette
				else
				{
					if (m_LoadedNewScene == false)
					{
						SceneManager::Instance().LoadScene(m_TransitionSceneName);
						m_LoadedNewScene = true;
					}

					vignette->SetRadius((m_ElapsedTime - m_HalfDuration) / m_HalfDuration);
				}

				m_ElapsedTime += Engine::GetInstance().GetWindow()->GetDeltaTime();
			}
			else
			{
				PostProcessingManager::Instance().GetPostEffect<Vignette>("Vignette")->SetRadius(1.0f);
				m_ElapsedTime = 0.0f;
				m_Duration = 0.0f;
				m_IsTransitioning = false;
				m_LoadedNewScene = false;
				m_TransitionSceneName = "";
			}
		}
	}

	void SceneTransitioner::TransitionToScene(const std::string& sceneName, const float totalDuration)
	{
		m_TransitionSceneName = sceneName;
		m_Duration = totalDuration;
		m_HalfDuration = m_Duration / 2.0f;
		m_IsTransitioning = true;
	}
}