/*!
	@file      ContentBrowserPanel.h
	@author    Hu Jun Ning (Code Contribution 100%)
	@email     junning.hu@digipen.edu
	@coauthor  Co-Author Name (Code Contribution 100%)
	@email     CoAuthor.n@digipen.edu
	@date      02/09/2023
	@brief     Content browser panel

	Copyright (C) 2023 DigiPen Institute of Technology.
	Reproduction or disclosure of this file or its contents without the
	prior written consent of DigiPen Institute of Technology is prohibited.
************************************************************************/
#pragma once
#include "TREIncludes.h"
#include "Panel.h"

namespace TRE
{
	class ContentBrowserPanel : public Panel
	{
		public:
			ContentBrowserPanel();
			~ContentBrowserPanel();
			void Init() override;
			void Update() override;
			void Shutdown() override;

		private:
			std::filesystem::path m_CurrentDirectory;
	};
}