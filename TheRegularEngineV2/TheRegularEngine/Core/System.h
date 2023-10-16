#pragma once

namespace TRE
{
	class System
	{
		public:
			System() = default;
			virtual ~System() = default;

			virtual void RenderImgui() = 0;
			virtual void Update() = 0;
			virtual void Shutdown() = 0;
	};

	class ECSSystem
	{
		public:
			ECSSystem() = default;
			virtual ~ECSSystem() = default;

			virtual void Init() {};				// This is only called once before running the Engine loop
			virtual void Update() {};			// This is ALWAYS called for each loop (Use this if the function always get called)
			virtual void GameUpdate() {};		// This is ONLY called when the game is running/scene is playing
			virtual void LateUpdate() {};		// This is called AFTER both GameUpdate() and LateUpdate()
			virtual void BeforeReset() {};		// This is called before the scene gets resetted in the previous loop
			virtual void OnReset() {};			// This is called after the scene gets resetted in the previous loop
			virtual void OnDestroyEntities() {};// This is ALWAYS called AFTER entities are deleted in the scene
			virtual void Shutdown() {};			// This is called when the engine shuts down properly
	};
}