#pragma once
#include "TREIncludes.h"
#include "Panel.h"

namespace TRE
{
	class ConsolePanel : public Panel
	{
		public:
			ConsolePanel();
			~ConsolePanel();
			void Init() override;
			void Update() override;
			void Shutdown() override;

		private:

	};
}