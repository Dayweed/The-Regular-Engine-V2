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
#include "SelectionManager.h"
#include "Editor/ImGuizmo.h"

namespace TRE
{
	class ViewportPanel : public Panel
	{
		public:
			ViewportPanel(const std::shared_ptr<SelectionManager>& selection_Manager);
			~ViewportPanel();
			void Init() override;
			void Update() override;
			void Shutdown() override;

			void OnMouseMove(const MouseMoveEvent& event);
			void OnMouseClick(const InputEvent& event);
			void OnMouseScroll(const MouseScrollEvent& event);
			void OnKeyboardClick(const InputEvent& event);
		private:
			//Gonna rewrite in editor camera next time
			glm::vec2 PanSensitivity(const float viewportWidth, const float viewportHeight)
			{
				float x = std::min(viewportWidth / 1000.f, 2.4f); //Max is 2.4f
				float xFactor = 0.0366f * (x * x) - 0.1778f * x + 0.3021f;

				float y = std::min(viewportHeight / 1000.f, 2.4f); //Max is 2.4f
				float yFactor = 0.0366f * (y * y) - 0.1778f * y + 0.3021f;

				return { xFactor, yFactor };
			}

			void UpdateViewportSize();
		private:
			std::shared_ptr<SelectionManager> m_SelectionManager;
			int m_GizmoOperation = -1; // -1 means no operation

			ImVec2 m_ViewportSize;
			ImVec2 m_ImageSize;
			ImVec2 m_ImageOffset;
			ImVec2 m_WindowPos;
			ImVec2 m_WorldMousePos;
			bool m_IsViewportHovered = false;
			bool m_IsViewportFocused = false;
			glm::vec2 m_MousePos{};
			float m_ZoomSensitivity = 50.f;
			float m_PanSpeed = 300.f;
			float m_RotationSensitivity = 3.f;
	};
}