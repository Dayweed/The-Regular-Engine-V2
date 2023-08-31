#pragma once
#include "TREIncludes.h"
#include "Panel.h"

namespace TRE
{
	class ContentBrowserPanel : public Panel
	{
		public:
			ContentBrowserPanel();
			~ContentBrowserPanel();
			void Init() override;
			void Update() override;
			void Shutdown() override;

		private:
	};
}