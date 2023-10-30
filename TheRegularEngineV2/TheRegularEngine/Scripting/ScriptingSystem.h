#pragma once

#include "Core/System.h"
#include "Core/ECS.h"

namespace TRE
{
	class ScriptingSystem : public ECSSystem
	{
		void Init() override;
		void Update() override;
		void GameUpdate() override;		// This is ONLY called when the game is running/scene is playing
		void LateUpdate() override;		// This is called AFTER both GameUpdate() and LateUpdate()
		void BeforeReset() override;		// This is called before the scene gets resetted in the previous loop
		void AfterReset() override;		// This is called after the scene gets resetted in the previous loop
		void OnDestroyEntities() override;// This is ALWAYS called AFTER entities are deleted in the scene
		void Shutdown() override;

		void AddScriptableObject(Entity entity);
		void RemoveScriptableObject(Entity entity);
		void InitializeScriptableObjects();

		void UpdateScriptableObjects();
		void CheckForNewScriptableObjects();


		bool DemoInit = true;

		void SetScriptable(bool b){m_ScriptableUpdate = b;}

	private:
		std::vector<Entity> m_ScriptEntities;
		bool m_ScriptableUpdate;
		bool m_IsRunning;
	};
}