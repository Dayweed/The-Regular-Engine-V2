#pragma once
#include "TREIncludes.h"

namespace TRE
{
	class EditorSystem : public System
	{
		public:
			EditorSystem();
			~EditorSystem();

			void RenderImgui() override;
			void Update() override;
			void Shutdown() override;

		private:
			
	};

	class Panel
	{
		public:
			Panel();
			~Panel();

			void CreatePanel(std::string Panel_Name);

		private:
			std::string PanelName;
	};
}