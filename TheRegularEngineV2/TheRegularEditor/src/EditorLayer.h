#pragma once
#include "TREIncludes.h"

namespace TRE
{
	class EditorLayer : public Layer
	{
		public:
			EditorLayer();
			~EditorLayer();

			void Update() override;
			void Shutdown() override;

		private:

	};
}