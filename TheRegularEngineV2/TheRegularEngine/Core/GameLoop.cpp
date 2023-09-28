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

	void GameLoop::ToggleRun(bool isRunning)
	{
		// If toggle to run and was not running, save scene temporarily
		if (isRunning && !m_GameRunning)
		{
			// Destroys all undeployed entities
			MemoryManager::Instance().ClearUndeployed();

			// Save the registry
			m_BackUp.clear();
			ECSManager::Instance().SaveRegistry(m_BackUp);
		}

		m_GameRunning = isRunning;
	}

	void GameLoop::ResetScene()
	{
		if (m_GameRunning)
		{
			Profiler::Instance().StartTimer("BeforeReset");
			ECSSystemManager::Instance().BeforeReset();
			Profiler::Instance().EndTimer("BeforeReset");

			// Copy registry and components
			ECSManager::Instance().CopyRegistry(m_BackUp);
			// Clear Backup
			m_BackUp.clear();

			Profiler::Instance().StartTimer("OnReset");
			ECSSystemManager::Instance().OnReset();
			Profiler::Instance().EndTimer("OnReset");

			m_GameRunning = false;

			ScriptEngine::CreatedScriptObject = false;

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