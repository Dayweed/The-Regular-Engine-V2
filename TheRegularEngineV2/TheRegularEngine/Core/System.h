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

			virtual void Init() {};
			virtual void BeforeReset() {};
			virtual void OnReset() {};
			virtual void Update() {};
			virtual void OnDestroyGO() {};
			virtual void Shutdown() {};
	};
}