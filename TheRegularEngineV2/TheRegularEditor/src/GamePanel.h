#pragma once
#include "Panel.h"

namespace TRE
{
	class GamePanel : public Panel
	{
		public:
			GamePanel();
			~GamePanel();

			void Init() override;
			void Update() override;
			void Shutdown() override;

			void UpdateViewportSize();

		private:
			ImVec2 m_ViewportSize;
			ImVec2 m_ImageSize;
			ImVec2 m_ImageOffset;
			ImVec2 m_WindowPos;
			bool m_IsViewportHovered = false;
	};
}