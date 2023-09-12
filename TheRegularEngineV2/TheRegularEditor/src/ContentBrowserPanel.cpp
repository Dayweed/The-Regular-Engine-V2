#include "pch.h"
#include "ContentBrowserPanel.h"
#include "Imgui/imgui.h"

namespace TRE
{

	//Root Folders
	static std::filesystem::path assetPath = "../Assets/";
	ContentBrowserPanel::ContentBrowserPanel()
		: m_CurrentDirectory(assetPath), m_AssetDirectory(assetPath)
	{
	}

	ContentBrowserPanel::~ContentBrowserPanel()
	{

	}

	void ContentBrowserPanel::PollItems()
	{
		m_Assets.clear();
		for (auto& p : std::filesystem::directory_iterator(m_AssetDirectory))
		{
			const std::filesystem::path path = p.path();
			const std::filesystem::path relativePath =	std::filesystem::relative(path, assetPath);
			const std::string filenameString = relativePath.filename().string();
			if (p.is_directory())
			{
				m_Assets.emplace_back(Asset{ true, "m_Invalid", filenameString, path });
			}
			else
			{
				Asset newAsset{};

				//Determine the type of resource to drag and drop
				const bool isImage = filenameString.ends_with(".png");
				const bool isAudio = filenameString.ends_with(".wav");
				const bool isShader = filenameString.ends_with(".vert") || filenameString.ends_with(".frag");
				const bool isScene = filenameString.ends_with(".scene");
				const bool isPrefab = filenameString.ends_with(".prefab");
				const bool isMeta = filenameString.ends_with(".meta");
				const bool isFont = filenameString.ends_with(".ttf");

				//Determine the icon type TBD

				if (isImage || isAudio || isShader || isScene | isPrefab || isFont)
				{
					//Allow Dragging of these file types
					newAsset.m_ResourceType = isImage ? "m_TextureResource" : newAsset.m_ResourceType;
					newAsset.m_ResourceType = isAudio ? "m_AudioResource" : newAsset.m_ResourceType;
					newAsset.m_ResourceType = isShader ? "m_ShaderResource" : newAsset.m_ResourceType;
					newAsset.m_ResourceType = isFont ? "m_FontResource" : newAsset.m_ResourceType;
					newAsset.m_ResourceType = isScene ? "m_Scene" : newAsset.m_ResourceType;
					newAsset.m_ResourceType = isPrefab ? "m_Prefab" : newAsset.m_ResourceType;
				}

				newAsset.m_Path = path;
				newAsset.m_FileName = filenameString;

				m_Assets.emplace_back(newAsset);
			}
		}
	}

	void ContentBrowserPanel::BrowseProjectFiles()
	{
		//Folder List Display
		if(ImGui::BeginChild("Folder List", ImVec2(ImGui::GetContentRegionAvail().x * 0.25f, ImGui::GetContentRegionAvail().y), true))
		{
			for (auto& p : std::filesystem::directory_iterator(m_AssetDirectory))
			{
				const std::filesystem::path path = p.path();
				const std::filesystem::path relativePath = std::filesystem::relative(path, m_AssetDirectory);
				const std::string filenameString = relativePath.filename().string();
				if (p.is_directory())
				{
					if (ImGui::Button(filenameString.c_str()))
					{
						m_CurrentDirectory = m_AssetDirectory;
						m_AssetDirectory /= path.filename();
						PollItems();
					}
				}
			}
		}
		ImGui::EndChild();
		ImGui::SameLine();

		//Item List Display

	}

	void ContentBrowserPanel::Init()
	{
		
	}
	
	void ContentBrowserPanel::Update()
	{

		//To get dt Engine::GetInstance().GetWindow()->GetDeltaTime();
		ImGui::Begin("Content Browser Panel");

		//if(m_CurrentDirectory != std::filesystem::path(assetPath))
		//{
		//	if(ImGui::Button("Back"))
		//	{
		//		m_CurrentDirectory = m_CurrentDirectory.parent_path();
		//	}
		//}

		//for(auto& directoryEntry: std::filesystem::directory_iterator(m_CurrentDirectory))	
		//{
		//	const auto& path = directoryEntry.path();
		//	auto relativePath = std::filesystem::relative(path, assetPath);
		//	std::string fileNameString = relativePath.filename().string();

		//	if(directoryEntry.is_directory())
		//	{
		//		if (ImGui::Button(fileNameString.c_str()))
		//		{
		//			m_CurrentDirectory /= path.filename();
		//		}

		//	}
		//	else
		//	{
		//		if (ImGui::Button(fileNameString.c_str()))
		//		{

		//		}
		//	}
		//}


		//Testing the Browser stuff

		ImGui::End();
	}

	void ContentBrowserPanel::Shutdown()
	{

	}
}