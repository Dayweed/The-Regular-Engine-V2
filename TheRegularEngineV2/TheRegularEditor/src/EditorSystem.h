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
			void Shutdown() override;

		private:
			
	};
}