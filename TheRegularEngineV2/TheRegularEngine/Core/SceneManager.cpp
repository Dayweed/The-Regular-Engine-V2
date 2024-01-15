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

	void ScenePostEffectsSystem::Init()
	{
		for (Transition& transition : m_Transitions)
		{
			transition.m_Duration = 0.0f;
			transition.m_HalfDuration = 0.0f;
			transition.m_ElapsedTime = 0.0f;
			transition.m_IsTransitioning = false;
			transition.m_LoadedNewScene = false;
			transition.m_TransitionSceneName = "";
		}
	}

	void ScenePostEffectsSystem::Update()
	{
		for (size_t i{}; i < TYPE_SIZE; ++i)
		{
			Transition& transition = m_Transitions[i];
			if (transition.m_IsTransitioning)
			{
				if (transition.m_ElapsedTime < transition.m_Duration)
				{
					// Finish transition
					switch ((TransitionTypeIndex)i)
					{
					case TYPE_VIGNETTE:
						VignetteCalc();
						break;
					case TYPE_FADE:
						break;
					default:
						break;
					}

					transition.m_ElapsedTime += Engine::GetInstance().GetWindow()->GetDeltaTime();
				}
				else
				{
					transition.m_ElapsedTime = 0.0f;
					transition.m_Duration = 0.0f;
					transition.m_IsTransitioning = false;
					transition.m_LoadedNewScene = false;
					transition.m_TransitionSceneName = "";
					transition.m_State = STATE_NONE;
				}
			}
		}
	}

	ScenePostEffectsSystem::TransitionState ScenePostEffectsSystem::GetTransitionState(ScenePostEffectsSystem::TransitionTypeIndex index)
	{
		return m_Transitions[index].m_State;
	}

	void ScenePostEffectsSystem::VignetteCalc()
	{
		Transition& vignetteTransition = m_Transitions[TYPE_VIGNETTE];
		auto vignette = PostProcessingManager::Instance().GetPostEffect<Vignette>("Vignette");
		// Close vignette
		if (vignetteTransition.m_ElapsedTime < vignetteTransition.m_HalfDuration)
		{	
			vignette->SetRadius(1.0f - (vignetteTransition.m_ElapsedTime / vignetteTransition.m_HalfDuration));

			// Transition State
			vignetteTransition.m_State = STATE_GOINGIN;

			// Check if complete closing state after this
			if (Engine::GetInstance().GetWindow()->GetDeltaTime() + vignetteTransition.m_ElapsedTime >= vignetteTransition.m_HalfDuration)
			{
				vignette->SetRadius(0.0f);
				vignetteTransition.m_State = STATE_IN;
			}
		}
		// Open vignette
		else
		{
			// Transition State
			if (vignetteTransition.m_State == STATE_IN)
			{
				vignetteTransition.m_State = STATE_GOINGOUT;
			}

			vignette->SetRadius((vignetteTransition.m_ElapsedTime - vignetteTransition.m_HalfDuration) / vignetteTransition.m_HalfDuration);


			if (vignetteTransition.m_LoadedNewScene == false && vignetteTransition.m_TransitionSceneName != "")
			{
				SceneManager::Instance().LoadScene(vignetteTransition.m_TransitionSceneName);
				vignetteTransition.m_LoadedNewScene = true;
			}

			// Check if complete opening state after this
			if (Engine::GetInstance().GetWindow()->GetDeltaTime() + vignetteTransition.m_ElapsedTime >= vignetteTransition.m_Duration)
			{
				vignette->SetRadius(1.0f);
				vignetteTransition.m_State = STATE_OUT;
			}
		}
	}

	void ScenePostEffectsSystem::TransitionToScene(const std::string& sceneName, const float totalDuration)
	{
		// Auto use vignette
		Transition& vignetteTransition = m_Transitions[TYPE_VIGNETTE];

		// Ignores if is already transitioning
		if (vignetteTransition.m_IsTransitioning) return;

		vignetteTransition.m_TransitionSceneName = sceneName;
		vignetteTransition.m_Duration = totalDuration;
		vignetteTransition.m_HalfDuration = vignetteTransition.m_Duration / 2.0f;
		vignetteTransition.m_IsTransitioning = true;
	}

	void ScenePostEffectsSystem::VignetteShrink(const float totalDuration)
	{
		// Auto use vignette
		Transition& vignetteTransition = m_Transitions[TYPE_VIGNETTE];

		// Ignores if is already transitioning
		if (vignetteTransition.m_IsTransitioning) return;

		vignetteTransition.m_Duration = totalDuration;
		vignetteTransition.m_HalfDuration = vignetteTransition.m_Duration / 2.0f;
		vignetteTransition.m_IsTransitioning = true;
	}
}