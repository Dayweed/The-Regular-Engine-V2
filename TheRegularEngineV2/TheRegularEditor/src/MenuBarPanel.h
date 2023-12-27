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
#include "EventSystem/Events/InputEvent.h"
#include "EventSystem/Events/EditorEvent.h"

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
			bool GetShowHierarchyPanel() const { return m_ShowHierarchyPanel; }
			bool GetShowInspectorPanel() const { return m_ShowInspectorPanel; }
			bool GetShowToolBarPanel() const { return m_ShowToolBarPanel; }
			bool GetShowScenePanel() const { return m_ShowScenePanel; }
			bool GetShowGamePanel() const { return m_ShowGamePanel; }
			bool GetShowConsolePanel() const { return m_ShowConsolePanel; }
			bool GetShowAssetPanel() const { return m_ShowAssetPanel; }
			bool GetShowCollisionMatrixPanel() const { return m_ShowCollisionMatrixPanel; }
			bool GetExitPrompt() const { return m_ExitPrompt; }

			void HandleShortcuts(TypingEvent& event);

			void Serialize(std::ofstream& file);
			void Deserialize(std::ifstream& file);

		private:
			//Bool to the status of every menu item
			bool m_ShowHierarchyPanel = true;
			bool m_ShowInspectorPanel = true;
			bool m_ShowToolBarPanel = true;
			bool m_ShowScenePanel = true;
			bool m_ShowGamePanel = true;
			bool m_ShowConsolePanel = true;
			bool m_ShowAssetPanel = false;
			bool m_ShowCollisionMatrixPanel = false;

			//bool for options
			bool m_Test = false;

			//bool for shortcuts
			bool m_ShortcutNewScene = false;
			bool m_ShortcutOpenScene = false;
			bool m_ShortcutSaveScene = false;

			//bool for exit prompt
			bool m_ExitPrompt = false;
			
			//Gizmo actions
			bool m_GridAndSnap = false;
			//Floats for position, rotation, and scale
			float m_PosIncrement = 1.0f;
			float m_RotIncrement = 1.0f;
			float m_ScaleIncrement = 1.0f;
			//bool for local and global
			bool m_LocalGizmo = false;

			//Editor camera
			float m_PanSpeed{ 1000.f };
			float m_ZoomSensitivity{ 100.f };
			float m_RotationSensitivity{ 4.f };

			//Bool for show all collider draws
			bool m_ShowAllColliders = false;

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