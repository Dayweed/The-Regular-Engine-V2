#pragma once
namespace TRE
{
	class System
	{
		public:
			System() = default;
			virtual ~System() = default;

			virtual void RenderImgui() {};
			virtual void Update() = 0;
			virtual void OnDestroyGO() {};
			virtual void Shutdown() = 0;
	};
}