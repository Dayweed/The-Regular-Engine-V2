#pragma once
#include "TREIncludes.h"
#include "Panel.h"

namespace TRE
{
	class SceneHierarchyPanel : public Panel
	{
		public:
			SceneHierarchyPanel();
			~SceneHierarchyPanel();
			void Init() override;
			void Update() override;
			void Shutdown() override;

		private:
	};
}