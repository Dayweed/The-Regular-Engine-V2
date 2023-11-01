#include "pch.h"
#include "ToolBarPanel.h"
#include "Imgui/imgui.h"
#include "Utilities.h"
#include "EditorAssetManager.h"
#include "EventSystem/EventHandler/EventHandler.h"
#include "EditorSystem.h"
#include "Scripting/ScriptEngine.h"

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
		const auto playGUID = AssetManager::Instance().GetAssetHandle("icon-play.png");
		const auto playHexGUID = Resource::GetGUIDHex(playGUID);
		std::unique_ptr<VulkanTexture> playButton = std::make_unique<VulkanTexture>("../Resources/" + playHexGUID + ".DDS");
		playButton->SetHandle(playGUID);
		AssetManager::Instance().AddAsset("icon-play.png", std::move(playButton));
		m_PlayButtonTexture = ResourceManager::Instance().GetResource<VulkanTexture>(playGUID);
		m_PlayID = Util::GetTextureID(m_PlayButtonTexture->GetDescriptorImageInfo());

		const auto pauseGUID = AssetManager::Instance().GetAssetHandle("icon-pause.png");
		const auto pauseHexGUID = Resource::GetGUIDHex(pauseGUID);
		std::unique_ptr<VulkanTexture> pauseButton = std::make_unique<VulkanTexture>("../Resources/" + pauseHexGUID + ".DDS");
		pauseButton->SetHandle(pauseGUID);
		AssetManager::Instance().AddAsset("icon-pause.png", std::move(pauseButton));
		m_PauseButtonTexture = ResourceManager::Instance().GetResource<VulkanTexture>(pauseGUID);
		m_PauseID = Util::GetTextureID(m_PauseButtonTexture->GetDescriptorImageInfo());

		const auto stopGUID = AssetManager::Instance().GetAssetHandle("icon-stop.png");
		const auto stopHexGUID = Resource::GetGUIDHex(stopGUID);
		std::unique_ptr<VulkanTexture> stopButton = std::make_unique<VulkanTexture>("../Resources/" + stopHexGUID + ".DDS");
		stopButton->SetHandle(stopGUID);
		AssetManager::Instance().AddAsset("icon-stop.png", std::move(stopButton));
		m_StopButtonTexture = ResourceManager::Instance().GetResource<VulkanTexture>(stopGUID);
		m_StopID = Util::GetTextureID(m_StopButtonTexture->GetDescriptorImageInfo());
	}

	void ToolBarPanel::Update()
	{
		ImGui::Begin("Tool Bar", 0, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
		ImGui::SameLine(ImGui::GetContentRegionAvail().x/2 - 25);
		
		if (ImGui::ImageButton(m_PlayID, ImVec2(20, 20), ImVec2(0, 0), ImVec2(1, 1), 0) || ImGui::IsKeyPressed(ImGuiKey_F5))
		{
			EventHandler::getEventHandlerInstance().Publish(ToggleRunEvent{ true });
		}

		ImGui::SameLine(ImGui::GetContentRegionAvail().x / 2);

		if (ImGui::ImageButton(m_PauseID, ImVec2(20, 20), ImVec2(0, 0), ImVec2(1, 1), 0) || ImGui::IsKeyPressed(ImGuiKey_F6))
		{
			EventHandler::getEventHandlerInstance().Publish(ToggleRunEvent{ false });
		}

		ImGui::SameLine(ImGui::GetContentRegionAvail().x / 2 + 25);

		if (ImGui::ImageButton(m_StopID, ImVec2(20, 20), ImVec2(0, 0), ImVec2(1, 1), 0) || ImGui::IsKeyPressed(ImGuiKey_F7))
		{
			EventHandler::getEventHandlerInstance().Publish(ResetSceneEvent{false});
			EditorSystemManager::Instance().GetSystem<EditorSystem>()->GetSelectionManager()->ClearSelectedEntity();
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