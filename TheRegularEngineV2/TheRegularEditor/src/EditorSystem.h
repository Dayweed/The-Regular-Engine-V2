#pragma once
#include "TREIncludes.h"
#include "pch.h"
#include "PanelManager.h"
#include "SelectionManager.h"
#include "AssetSelector.h"

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

			std::shared_ptr<SelectionManager>& GetSelectionManager();

		private:
			std::unique_ptr<PanelManager> m_PanelManager;
			std::shared_ptr<SelectionManager> m_SelectionManager;
			std::shared_ptr<AssetSelector> m_AssetSelector;
	};
}