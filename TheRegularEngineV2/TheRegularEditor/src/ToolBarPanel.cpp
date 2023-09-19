#include "pch.h"
#include "ToolBarPanel.h"
#include "Imgui/imgui.h"
#include "Utilities.h"

namespace TRE
{
	ToolBarPanel::ToolBarPanel()
	{
	}

	ToolBarPanel::~ToolBarPanel()
	{

	}

	void ToolBarPanel::Init()
	{

	}

	void ToolBarPanel::Update()
	{
		ImGui::Begin("Tool Bar");
		//ImGui::Text("Testing where the tool bar is");
		//How to pad in imgui
		ImGui::SameLine(ImGui::GetContentRegionAvail().x/2 - 25);

		auto textureHandle = Asset::GetGUIDFromHex("500a79fe5030f521");
		std::shared_ptr<VulkanTexture> m_PlayButtonTexture = AssetManager::Instance().GetAsset<VulkanTexture>(textureHandle);

		if (ImGui::ImageButton(Util::GetTextureID(m_PlayButtonTexture->GetDescriptorImageInfo()), ImVec2(20, 20), ImVec2(0, 0), ImVec2(1, 1), 0))
		{
			TRE_CORE_INFO("Play Button Pressed");
		}

		ImGui::SameLine(ImGui::GetContentRegionAvail().x / 2);

		if (ImGui::ImageButton(Util::GetTextureID(m_PlayButtonTexture->GetDescriptorImageInfo()), ImVec2(20, 20), ImVec2(0, 0), ImVec2(1, 1), 0))
		{
			TRE_CORE_INFO("Button Pressed");
		}

		ImGui::SameLine(ImGui::GetContentRegionAvail().x / 2 + 25);

		if (ImGui::ImageButton(Util::GetTextureID(m_PlayButtonTexture->GetDescriptorImageInfo()), ImVec2(20, 20), ImVec2(0, 0), ImVec2(1, 1), 0))
		{
			TRE_CORE_INFO("Work Button Pressed");
		}

		ImGui::End();
	}

	void ToolBarPanel::Shutdown()
	{

	}
}