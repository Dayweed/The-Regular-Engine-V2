/*!
	@file      SceneHierarchyPanel.h
	@author    Hu Jun Ning (Code Contribution 100%)
	@email     junning.hu@digipen.edu
	@coauthor  Co-Author Name (Code Contribution 100%)
	@email     CoAuthor.n@digipen.edu
	@date      02/09/2023
	@brief     Scene hierarchy panel

	Copyright (C) 2023 DigiPen Institute of Technology.
	Reproduction or disclosure of this file or its contents without the
	prior written consent of DigiPen Institute of Technology is prohibited.
************************************************************************/
#pragma once
#include "TREIncludes.h"
#include "Panel.h"
#include "SelectionManager.h"
#include "EventSystem/Events/InputEvent.h"

namespace TRE
{
	class SceneHierarchyPanel : public Panel
	{
		public:
			SceneHierarchyPanel(const std::shared_ptr<SelectionManager>& Selection_Manager);
			~SceneHierarchyPanel();
			void Init() override;
			void Update() override;
			void Shutdown() override;
			void DisplayChildren(TRE::Entity& CurrentEntity);
			void DeleteChildren(TRE::Entity& CurrentEntity);

			void HandleShortcuts(TypingEvent& event);

		private:
			std::shared_ptr<SelectionManager> m_SelectionManager;
			bool m_ShortcutCopyEntity = false;
			bool m_ShortcutPasteEntity = false;
	};
}