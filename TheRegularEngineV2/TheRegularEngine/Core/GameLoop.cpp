#include "pch.h"

#include "SceneManager.h"
#include "ECS.h"
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
		}

		m_GameRunning = isRunning;
	}

	void GameLoop::ResetScene()
	{
		if (!m_BackUp.empty())
		{
			m_GameRunning = false;
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
		EventHandler::getEventHandlerInstance().Publish(ConsoleDebugEvent{ "Reseting scene..." });
		ResetScene();
	}
}