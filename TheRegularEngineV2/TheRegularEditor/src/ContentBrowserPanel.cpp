#include "ContentBrowserPanel.h"
#include "Imgui/imgui.h"

namespace TRE
{

	//Root Folders
	static std::filesystem::path assetPath = "../Assets/";
	ContentBrowserPanel::ContentBrowserPanel()
		: m_CurrentDirectory(assetPath)
	{
	}

	ContentBrowserPanel::~ContentBrowserPanel()
	{

	}

	void ContentBrowserPanel::Init()
	{
		
	}
	
	void ContentBrowserPanel::Update()
	{
		ImGui::Begin("Content Browser Panel");

		if(m_CurrentDirectory != std::filesystem::path(assetPath))
		{
			if(ImGui::Button("Back"))
			{
				m_CurrentDirectory = m_CurrentDirectory.parent_path();
			}
		}

		for(auto& directoryEntry: std::filesystem::directory_iterator(m_CurrentDirectory))	
		{
			const auto& path = directoryEntry.path();
			auto relativePath = std::filesystem::relative(path, assetPath);
			std::string fileNameString = relativePath.filename().string();

			if(directoryEntry.is_directory())
			{
				if (ImGui::Button(fileNameString.c_str()))
				{
					m_CurrentDirectory /= path.filename();
				}

			}
			else
			{
				if (ImGui::Button(fileNameString.c_str()))
				{

				}
			}
		}
		ImGui::End();
	}

	void ContentBrowserPanel::Shutdown()
	{

	}
}