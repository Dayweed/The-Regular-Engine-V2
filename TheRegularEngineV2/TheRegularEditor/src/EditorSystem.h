#pragma once
#include "TREIncludes.h"

namespace TRE
{
	class EditorSystem : public System
	{
		public:
			EditorSystem();
			~EditorSystem();

			void Update() override;
			void RenderImgui() override;
			void Shutdown() override;

		private:

	};
}