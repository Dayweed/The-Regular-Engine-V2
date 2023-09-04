#pragma once
#include "TREIncludes.h"
#include "pch.h"
#include "PanelManager.h"

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
			std::unique_ptr<PanelManager> m_PanelManager;
	};
}