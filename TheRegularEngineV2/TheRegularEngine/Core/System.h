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

			virtual void Update() = 0;
			virtual void OnReset() = 0;
			virtual void OnDestroyGO() = 0;
			virtual void Shutdown() = 0;
	};
}