/*!
	@file      ToolBarPanel.h
	@author    - (Code Contribution 100%)
	@email     -
	@coauthor  Co-Author Name (Code Contribution 100%)
	@email     CoAuthor.n@digipen.edu
	@date      12/09/2023
	@brief     Tool bar panel

	Copyright (C) 2023 DigiPen Institute of Technology.
	Reproduction or disclosure of this file or its contents without the
	prior written consent of DigiPen Institute of Technology is prohibited.
************************************************************************/
#pragma once
#include "TREIncludes.h"
#include "Panel.h"

namespace TRE
{
	class ToolBarPanel : public Panel
	{
		public:
			ToolBarPanel();
			~ToolBarPanel();
			void Init() override;
			void Update() override;
			void Shutdown() override;

		private:
			std::shared_ptr<VulkanTexture> m_PlayButtonTexture;
			std::shared_ptr<VulkanTexture> m_PauseButtonTexture;
			std::shared_ptr<VulkanTexture> m_StopButtonTexture;

			ImTextureID m_PlayID;
			ImTextureID m_PauseID;
			ImTextureID m_StopID;
	};
}