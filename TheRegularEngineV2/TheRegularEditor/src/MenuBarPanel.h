/*!
	@file      MenuBarPanel.h
	@author    Hu Jun Ning (Code Contribution 100%)
	@email     junning.hu@digipen.edu
	@coauthor  Co-Author Name (Code Contribution 100%)
	@email     CoAuthor.n@digipen.edu
	@date      02/09/2023
	@brief     Menu bar panel

	Copyright (C) 2023 DigiPen Institute of Technology.
	Reproduction or disclosure of this file or its contents without the
	prior written consent of DigiPen Institute of Technology is prohibited.
************************************************************************/
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

			/*!*****************************************************************************
			*\brief 	Getter functions
			*
			*******************************************************************************/
			void GetShowHierarchyPanel(bool& showHierarchyPanel) { showHierarchyPanel = m_ShowHierarchyPanel; }
			void GetShowInspectorPanel(bool& showInspectorPanel) { showInspectorPanel = m_ShowInspectorPanel; }
			void GetShowToolBarPanel(bool& showToolBarPanel) { showToolBarPanel = m_ShowToolBarPanel; }
			void GetShowScenePanel(bool& showScenePanel) { showScenePanel = m_ShowScenePanel; }
			void GetShowGamePanel(bool& showGamePanel) { showGamePanel = m_ShowGamePanel; }
			void GetShowConsolePanel(bool& showConsolePanel) { showConsolePanel = m_ShowConsolePanel; }
			void GetShowAssetPanel(bool& showAssetPanel) { showAssetPanel = m_ShowAssetPanel; }

		private:
			//Bool to the status of every menu item
			bool m_ShowHierarchyPanel = true;
			bool m_ShowInspectorPanel = true;
			bool m_ShowToolBarPanel = true;
			bool m_ShowScenePanel = true;
			bool m_ShowGamePanel = true;
			bool m_ShowConsolePanel = true;
			bool m_ShowAssetPanel = true;

			//bool for options
			bool m_Test = false;

			//bool for exit prompt
			bool m_ExitPrompt = false;

			//Store some custom flags
			ImGuiConfigFlags m_PopUps{};
			
			/*!*****************************************************************************
			*\brief 	Creates a new, empty scene.
			*
			*******************************************************************************/
			void NewScene();
			/*!*****************************************************************************
			*\brief 	Opens the file explorer and allows the user to select a scene to load.
			*
			*******************************************************************************/
			void OpenScene();
			/*!*****************************************************************************
			*\brief 	Opens the file explorer and allows the user to save over a scene.
			*
			*******************************************************************************/
			void SaveScene();
	};
}