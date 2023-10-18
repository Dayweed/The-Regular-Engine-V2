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
#include "SelectionManager.h"
#include "AssetSelector.h"

namespace TRE
{
	class ContentBrowserPanel : public Panel
	{
		public:
			ContentBrowserPanel(const std::shared_ptr<SelectionManager>& Selection_Manager, const std::shared_ptr<AssetSelector>& assetSelector);
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
			//Scenes Folder Path
			std::filesystem::path m_SceneDirectory;
			//Scripts Folder Path
			std::filesystem::path m_ScriptDirectory;

			std::shared_ptr<SelectionManager> m_SelectionManager;
			std::shared_ptr<AssetSelector> m_AssetSelector;

			//Texture for icons
			ImTextureID m_TmpTexturesID{};

			//Shared pointer for icons
			std::shared_ptr<VulkanTexture> m_TmpTextures;

			//Variables for formatting
			const float m_Padding = 20.f;
			const float m_ImgSize = 64.f;
			const float m_CellSize = m_ImgSize + m_Padding;

			//Variable for limiting poll rate
			const float m_RefreshRate = 1.f;
			float m_CurrentTimer = 0.0f;

			struct Asset
			{
				bool m_Folder = false;
				ImTextureID m_TextureID{};
				std::string m_ResourceType{"m_Invalid"};
				std::string m_FileName{};
				std::filesystem::path m_Path{};
			};
			std::vector<Asset> m_Assets;

			//Store some ImGui Assets
			ImGuiConfigFlags m_PopUps;

			//PopUp for invalid resources
			bool m_InvalidResourcePopUp = false;
			bool m_AssetClicked = false;
	};
}