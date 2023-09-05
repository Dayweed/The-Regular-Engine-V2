/*!
	@file      ViewportPanel.h
	@author    Hu Jun Ning (Code Contribution 100%)
	@email     junning.hu@digipen.edu
	@coauthor  Co-Author Name (Code Contribution 100%)
	@email     CoAuthor.n@digipen.edu
	@date      02/09/2023
	@brief     Viewport panel

	Copyright (C) 2023 DigiPen Institute of Technology.
	Reproduction or disclosure of this file or its contents without the
	prior written consent of DigiPen Institute of Technology is prohibited.
************************************************************************/
#pragma once
#include "TREIncludes.h"
#include "Panel.h"
#include "EventSystem/Events/InputEvent.h"

namespace TRE
{
	class ViewportPanel : public Panel
	{
		public:
			ViewportPanel();
			~ViewportPanel();
			void Init() override;
			void Update() override;
			void Shutdown() override;

			void OnMouseMove(const MouseMoveEvent& event);
			void OnMouseClick(const InputEvent& event);

		private:
			bool m_IsViewportHovered = false;
			glm::vec2 m_MousePos{};
			glm::vec2 m_MouseStartPos{};
			glm::vec2 m_MouseEndPos{};
	};
}