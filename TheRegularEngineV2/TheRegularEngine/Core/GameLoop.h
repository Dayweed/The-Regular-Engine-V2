#pragma once
#include "pch.h"

#include "entt.hpp"

#include "EventSystem/EventHandler/EventHandler.h"
#include "EventSystem/Events/EditorEvent.h"

#define FILESYS_GAMELOOP_TEMPSAVE "../tmp/SavedScene"

namespace TRE
{
	class GameLoop
	{
	public:
		static GameLoop& Instance();

		void Init();
		void ClearReset();
		void Shutdown();

		bool IsGameRunning();
		bool IsResetted();		// Reset value will go back to false after one loop

		void ToggleRun(bool isRunning);
		void ResetScene();
		void ToggleRun(ToggleRunEvent& event);
		void Reset(ResetSceneEvent& event);

	private:
		// Game Loop
		bool m_GameRunning{ false };
		bool m_IsResetted{ false };

		entt::registry m_BackUp;

		// Delete possible copy ctor and assignment to ensure singleton
		GameLoop() {};
		GameLoop(GameLoop const&) = delete;
		void operator=(GameLoop const&) = delete;
		void* operator new(size_t) = delete;
	};
}