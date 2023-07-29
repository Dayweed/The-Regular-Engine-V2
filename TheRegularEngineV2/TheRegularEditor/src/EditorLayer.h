#pragma once
#include "TREIncludes.h"

namespace TRE
{
	class EditorLayer : public System
	{
		public:
			EditorLayer();
			~EditorLayer();

			void Update() override;
			void RenderImgui() override;
			void Shutdown() override;

		private:

	};
}