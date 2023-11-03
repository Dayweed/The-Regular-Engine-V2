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

		// This checks if the game had played and not stopped (true)
		bool GetGameSimulating();

		bool GetSceneReset();
		void SetSceneReset(bool reset);

		bool GetDisplayingPrefab();
		void SetDisplayingPrefab(bool isDisplaying);

		entt::registry& GetBackUpRegistry();

		void ToggleRun(bool isRunning);
		void ResetScene();
		void ToggleRun(ToggleRunEvent& event);
		void Reset(ResetSceneEvent& event);

	private:
		// Game Loop
		bool m_GameRunning{ false };
		bool m_GameSimulating{ false };	// This checks if the game had played and not stopped (true)

		// Game Got Reseted this scene
		bool m_SceneReset{ false };

		// Prefab display in the scene
		// (This will auto be set back to false if m_GameRunning == true or the user press play)
		// Entities are stored and cleared, if m_DisplayingPrefab == false, similar when game is running
		bool m_DisplayingPrefab{ false };

		entt::registry m_BackUp;

		// Delete possible copy ctor and assignment to ensure singleton
		GameLoop() {};
		GameLoop(GameLoop const&) = delete;
		void operator=(GameLoop const&) = delete;
		void* operator new(size_t) = delete;
	};
}