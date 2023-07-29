#pragma once

namespace TRE
{
	class System
	{
		public:
			System() = default;
			virtual ~System() = default;

			virtual void Update() = 0;
			virtual void RenderImgui() = 0;
			virtual void Shutdown() = 0;
	};

}