#pragma once
#include "pch.h"

#include "entt.hpp"

#include "EventSystem/EventHandler/EventHandler.h"
#include "EventSystem/Events/EditorEvent.h"

namespace TRE
{
	class GameLoop
	{
	public:
		static GameLoop& Instance();

		void Init();
		void Shutdown();

		bool IsGameRunning();

		bool GetSceneReset();
		void SetSceneReset(bool reset);

		entt::registry& GetBackUpRegistry();

		void ToggleRun(bool isRunning);
		void ResetScene();
		void ToggleRun(ToggleRunEvent& event);
		void Reset(ResetSceneEvent& event);

	private:
		// Game Loop
		bool m_GameRunning{ false };

		// Game Got Reseted this scene
		bool m_SceneReset{ false };

		entt::registry m_BackUp;

		// Delete possible copy ctor and assignment to ensure singleton
		GameLoop() {};
		GameLoop(GameLoop const&) = delete;
		void operator=(GameLoop const&) = delete;
		void* operator new(size_t) = delete;
	};
}