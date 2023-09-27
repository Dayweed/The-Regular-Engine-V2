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
		m_GameRunning = isRunning;

		// If toggle to run, save scene temporarily
		if (m_GameRunning)
		{
			ECSManager::Instance().SaveEntities(FILESYS_GAMELOOP_TEMPSAVE);
		}
	}

	void GameLoop::ResetScene()
	{
		if (std::filesystem::exists(FILESYS_GAMELOOP_TEMPSAVE))
		{
			//ECSManager::Instance().LoadEntities(FILESYS_GAMELOOP_TEMPSAVE);
			std::filesystem::remove(FILESYS_GAMELOOP_TEMPSAVE);
			m_IsResetted = true;
		}
	}

	void GameLoop::ToggleRun(ToggleRunEvent& event)
	{
		ToggleRun(event.m_Playing);
	}

	void GameLoop::Reset(ResetSceneEvent& event)
	{
		ToggleRun(false);
		ResetScene();
	}
}