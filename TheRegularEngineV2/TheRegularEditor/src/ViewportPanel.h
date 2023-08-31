#pragma once
#include "TREIncludes.h"
#include "Panel.h"

namespace TRE
{
	class ViewportPanel : public Panel
	{
		public:
			ViewportPanel();
			~ViewportPanel();
			void Init() override;
			void Update() override;
			void Shutdown() override;

		private:
			
	};
}