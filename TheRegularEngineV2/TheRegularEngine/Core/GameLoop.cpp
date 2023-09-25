#include "pch.h"

#include "GameLoop.h"

namespace TRE
{
	GameLoop& GameLoop::Instance()
	{
		static GameLoop instance;
		return instance;
	}

	bool GameLoop::IsGameRunning()
	{
		return m_GameRunning;
	}

	void GameLoop::ToggleRun(bool isRunning)
	{
		m_GameRunning = isRunning;
	}

	void GameLoop::ResetScene()
	{

	}
}