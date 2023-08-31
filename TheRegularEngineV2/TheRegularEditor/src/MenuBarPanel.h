#pragma once
#include "TREIncludes.h"
#include "Panel.h"

namespace TRE
{
	class MenuBarPanel : public Panel
	{
		public:
			MenuBarPanel();
			~MenuBarPanel();
			void Init() override;
			void Update() override;
			void Shutdown() override;

		private:
	};
}