#pragma once
namespace TRE
{
	class System
	{
		public:
			System() = default;
			virtual ~System() = default;

			virtual void Update() {};
			virtual void RenderImgui() {};
			virtual void Shutdown() {};
	};
}