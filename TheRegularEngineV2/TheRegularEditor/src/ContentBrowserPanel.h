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
			void PollItems();
			void BrowseProjectFiles();
			void Init() override;
			void Update() override;
			void Shutdown() override;

		private:
			//Current File Path
			std::filesystem::path m_CurrentDirectory;
			//Asset Folder Path
			std::filesystem::path m_AssetDirectory;

			//Texture for icons waiting on zr to put into my descriptor set

			////Variables for formatting
			//const float m_Padding = 20.f;
			//const float m_ImgSize = 64.f;
			//const float m_CellSize = m_ImgSize + m_Padding;

			////Variable for limiting poll rate
			//const float m_RefreshRate = 1.f;
			//float m_CurrentTimer = 0.0f;

			//struct Asset
			//{
			//	bool m_Folder = false;
			//	std::string m_ResourceType{"m_Invalid"};
			//	std::string m_FileName{};
			//	std::filesystem::path m_Path{};
			//};
			//std::vector<Asset> m_Assets;

			////Store some ImGui Assets
			//ImGuiConfigFlags m_PopUps;

			////PopUp for invalid resources
			//bool m_InvalidResourcePopUp = false;

	};
}