#include "pch.h"

#include "SceneManager.h"
#include "ECS.h"
#include "GameLoop.h"


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

	void GameLoop::ClearReset()
	{
		m_IsResetted = false;
	}

	void GameLoop::Shutdown()
	{
		if (std::filesystem::exists(FILESYS_GAMELOOP_TEMPSAVE))
		{
			std::filesystem::remove(FILESYS_GAMELOOP_TEMPSAVE);
		}

		m_BackUp.clear();
	}

	bool GameLoop::IsGameRunning()
	{
		return m_GameRunning;
	}

	bool GameLoop::IsResetted()
	{
		return m_IsResetted;
	}

	void GameLoop::ToggleRun(bool isRunning)
	{
		// If toggle to run and was not running, save scene temporarily
		if (isRunning && !m_GameRunning)
		{
			ECSManager::Instance().SaveEntities(FILESYS_GAMELOOP_TEMPSAVE);

			// Save the registry
			ECSManager::Instance().SaveRegistry(m_BackUp);
		}

		m_GameRunning = isRunning;
	}

	void GameLoop::ResetScene()
	{
		if (m_GameRunning)
		{
			// Copy registry and components
			ECSManager::Instance().CopyRegistry(m_BackUp);
			// Clear Backup
			m_BackUp.clear();

			//ECSManager::Instance().LoadEntities(FILESYS_GAMELOOP_TEMPSAVE);
			//std::filesystem::remove(FILESYS_GAMELOOP_TEMPSAVE);
			m_GameRunning = false;
			m_IsResetted = true;
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
			EventHandler::getEventHandlerInstance().Publish(ConsoleDebugEvent{ "Reseting scene..." });
			ResetScene();
	}
}