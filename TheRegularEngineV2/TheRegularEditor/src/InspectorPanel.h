#pragma once
#include "TREIncludes.h"
#include "Panel.h"

namespace TRE
{
	class InspectorPanel : public Panel
	{
		public:
			InspectorPanel();
			~InspectorPanel();
			void Init() override;
			void Update() override;
			void Shutdown() override;

		private:
	};
}