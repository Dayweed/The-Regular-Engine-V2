#include "pch.h"

#include "SceneManager.h"
#include "ECS/ECS.h"
#include "GameLoop.h"
#include "Scripting/ScriptEngine.h"


namespace TRE
{
	GameLoop& GameLoop::Instance()
	{
		static GameLoop instance;
		return instance;
	}

	void GameLoop::Init()
	{
		m_GameRunning = false;
		EventHandler::getEventHandlerInstance().subscribe(this, &GameLoop::ToggleRun);
		EventHandler::getEventHandlerInstance().subscribe(this, &GameLoop::Reset);
	}

	void GameLoop::Shutdown()
	{
		m_BackUp.clear();
	}

	bool GameLoop::IsGameRunning()
	{
		return m_GameRunning;
	}

	bool GameLoop::GetGameSimulating()
	{
		return m_GameSimulating;
	}

	bool GameLoop::GetSceneReset()
	{
		return m_SceneReset;
	}

	void GameLoop::SetSceneReset(bool reset)
	{
		m_SceneReset = reset;
	}

	bool GameLoop::GetDisplayingPrefab()
	{
		return m_DisplayingPrefab;
	}

	void GameLoop::SetDisplayingPrefab(bool isDisplaying)
	{
		m_DisplayingPrefab = isDisplaying;
	}

	entt::registry& GameLoop::GetBackUpRegistry()
	{
		return m_BackUp;
	}

	void GameLoop::ToggleRun(bool isRunning)
	{
		// If toggle to run and was not running, save scene temporarily
		if (isRunning && !m_GameRunning && m_BackUp.empty())
		{
			// Restore back the scene if it was displaying prefab (Assuming if m_BackUp saved the scene before displaying prefab)
			if (m_DisplayingPrefab)
			{
				ECSSystemManager::Instance().GetSystem<PrefabSystem>()->ReturnToScene();
			}

			// Destroys all undeployed entities
			//MemoryManager::Instance().ClearUndeployed();

			// Save the registry
			ECSManager::Instance().SaveRegistry(m_BackUp);

			// Remember current scene and scenepath
			m_BackUpSceneName = SceneManager::Instance().m_CurrentScene;
			m_BackUpSceneFilePath = SceneManager::Instance().m_CurrentSceneFilePath;
		}

		m_GameRunning = isRunning;

		// Start Simulating if it just started
		if (!m_GameSimulating && isRunning)
		{
			m_GameSimulating = true;
		}
	}

	void GameLoop::ResetScene()
	{
		if (!m_BackUp.empty())
		{
			m_GameRunning = false;
			m_GameSimulating = false;
			SetSceneReset(true);
		}
	}

	void GameLoop::ToggleRun(ToggleRunEvent& event)
	{
		if (m_GameRunning == false && event.m_Playing == true)
		{
			EventHandler::getEventHandlerInstance().Publish(ConsoleDebugEvent{ "Playing Scene..." });
			ToggleRun(event.m_Playing);
		}
		else if(m_GameRunning == true && event.m_Playing == true)
		{
			EventHandler::getEventHandlerInstance().Publish(ConsoleDebugEvent{ "Scene is already playing" });
		}
		else if (m_GameRunning == true && event.m_Playing == false)
		{
			EventHandler::getEventHandlerInstance().Publish(ConsoleDebugEvent{ "Pausing Scene..." });
			ToggleRun(event.m_Playing);
		}
		else
		{
			EventHandler::getEventHandlerInstance().Publish(ConsoleDebugEvent{ "Scene is already paused" });
		}
	}

	void GameLoop::Reset(ResetSceneEvent& event)
	{
		(void)event;
		EventHandler::getEventHandlerInstance().Publish(ConsoleDebugEvent{ "Resetting scene..." });
		ResetScene();
	}

	void GameLoop::InstantReset()
	{
		// Copy registry and components
		ECSManager::Instance().CopyRegistry(GetBackUpRegistry());
		// Clear Backup
		GetBackUpRegistry().clear();

		// Restore current scene and scenepath
		SceneManager::Instance().m_CurrentScene = m_BackUpSceneName;
		SceneManager::Instance().m_CurrentSceneFilePath = m_BackUpSceneFilePath;
	}
}