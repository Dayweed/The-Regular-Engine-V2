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

	}

	bool GameLoop::IsGameRunning()
	{
		return m_GameRunning;
	}

	void GameLoop::ToggleRun(bool isRunning)
	{
		m_GameRunning = isRunning;

		// If true, save scene registry scene stuff
		entt::registry& ecsRegistry{ ECSManager::Instance().GetRegistry() };
		ECSManager::Instance().SaveEntities(FILESYS_GAMELOOP_TEMPSAVE);
	}

	void GameLoop::ResetScene()
	{
		if (std::filesystem::exists(FILESYS_GAMELOOP_TEMPSAVE))
		{
			ECSManager::Instance().LoadEntities(FILESYS_GAMELOOP_TEMPSAVE);
		}
	}
}