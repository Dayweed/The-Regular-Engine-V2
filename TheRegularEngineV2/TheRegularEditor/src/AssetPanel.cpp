#include "AssetPanel.h"
#include "EditorAssetManager.h"
#include "EventSystem/EventHandler/EventHandler.h"
#include "EventSystem/Events/EditorEvent.h"

namespace TRE
{
	AssetPanel::AssetPanel(const std::shared_ptr<AssetSelector>& assetSelector)
		: m_AssetSelector(assetSelector)
	{
		EventHandler::getEventHandlerInstance().subscribe(this, &AssetPanel::OnAssetSelectEvent);
		EventHandler::getEventHandlerInstance().subscribe(this, &AssetPanel::OnAssetPanelEvent);
	}

	void AssetPanel::Init()
	{
	}

	void AssetPanel::Update()
	{
		if (m_ShowAssetPanel)
		{
			ImGui::Begin("Asset Panel", &m_ShowAssetPanel);
			for (const auto& asset : AssetManager::Instance().GetAssets())
			{
				std::string print = asset.first + " | " + Resource::GetGUIDHex(asset.second.first) + " | " + (asset.second.second ? "Compiled" : "Not Compiled");
				ImGui::Text(print.c_str());
			}
			ImGui::End();
		}
	}

	void AssetPanel::Shutdown()
	{

	}

	void AssetPanel::OnAssetSelectEvent(const AssetSelectorEvent& event)
	{

	}

	void AssetPanel::OnAssetPanelEvent(const AssetPanelEvent& event)
	{
		m_ShowAssetPanel = event.m_ShowAssetPanel;
	}
}