#pragma once
#include "ECS/System.h"

namespace TRE
{
	class AnimationSystem : public ECSSystem
	{
		public:
			virtual void Init() override;              // This is only called once before running the Engine loop
			virtual void Update() override;            // This is ALWAYS called for each loop (Use this if the function always get called)
			virtual void GameUpdate() override;        // This is ONLY called when the game is running/scene is playing
			virtual void LateUpdate() override;        // This is called AFTER both Update() and GameUpdate()
			virtual void BeforeReset() override;       // This is called before the scene gets reset in the previous loop
			virtual void AfterReset() override;        // This is called after the scene gets reset in the previous loop

		private:

	};
}
