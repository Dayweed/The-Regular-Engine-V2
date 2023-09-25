#include "pch.h"
#include "ToolBarPanel.h"
#include "Imgui/imgui.h"
#include "Utilities.h"
#include "EditorAssetManager.h"

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
		const auto playGUID = EditorAssetManager::Instance().GetAsset("icon-play.png");
		const auto playHexGUID = Resource::GetGUIDHex(playGUID);
		std::unique_ptr<VulkanTexture> playButton = std::make_unique<VulkanTexture>("../Resources/" + playHexGUID + ".DDS");
		playButton->SetHandle(playGUID);
		EditorAssetManager::Instance().AddAsset("icon-play.png", std::move(playButton));
		m_PlayButtonTexture = ResourceManager::Instance().GetResource<VulkanTexture>(playGUID);
		m_PlayID = Util::GetTextureID(m_PlayButtonTexture->GetDescriptorImageInfo());

		const auto pauseGUID = EditorAssetManager::Instance().GetAsset("icon-pause.png");
		const auto pauseHexGUID = Resource::GetGUIDHex(pauseGUID);
		std::unique_ptr<VulkanTexture> pauseButton = std::make_unique<VulkanTexture>("../Resources/" + pauseHexGUID + ".DDS");
		pauseButton->SetHandle(pauseGUID);
		EditorAssetManager::Instance().AddAsset("icon-pause.png", std::move(pauseButton));
		m_PauseButtonTexture = ResourceManager::Instance().GetResource<VulkanTexture>(pauseGUID);
		m_PauseID = Util::GetTextureID(m_PauseButtonTexture->GetDescriptorImageInfo());

		const auto stopGUID = EditorAssetManager::Instance().GetAsset("icon-stop.png");
		const auto stopHexGUID = Resource::GetGUIDHex(stopGUID);
		std::unique_ptr<VulkanTexture> stopButton = std::make_unique<VulkanTexture>("../Resources/" + stopHexGUID + ".DDS");
		stopButton->SetHandle(stopGUID);
		EditorAssetManager::Instance().AddAsset("icon-stop.png", std::move(stopButton));
		m_StopButtonTexture = ResourceManager::Instance().GetResource<VulkanTexture>(stopGUID);
		m_StopID = Util::GetTextureID(m_StopButtonTexture->GetDescriptorImageInfo());
	}

	void ToolBarPanel::Update()
	{
		ImGui::Begin("Tool Bar");
		ImGui::SameLine(ImGui::GetContentRegionAvail().x/2 - 25);
		
		if (ImGui::ImageButton(m_PlayID, ImVec2(20, 20), ImVec2(0, 0), ImVec2(1, 1), 0))
		{
			TRE_CORE_INFO("Play Button Pressed");
		}

		ImGui::SameLine(ImGui::GetContentRegionAvail().x / 2);

		if (ImGui::ImageButton(m_PauseID, ImVec2(20, 20), ImVec2(0, 0), ImVec2(1, 1), 0))
		{
			TRE_CORE_INFO("Button Pressed");
		}

		ImGui::SameLine(ImGui::GetContentRegionAvail().x / 2 + 25);

		if (ImGui::ImageButton(m_StopID, ImVec2(20, 20), ImVec2(0, 0), ImVec2(1, 1), 0))
		{
			TRE_CORE_INFO("Work Button Pressed");
		}

		ImGui::End();
	}

	void ToolBarPanel::Shutdown()
	{
		m_PlayButtonTexture.reset();
		m_PauseButtonTexture.reset();
		m_StopButtonTexture.reset();
	}
}