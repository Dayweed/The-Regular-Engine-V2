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
				constexpr unsigned maxAssetNameSize = 35;
				std::string assetName(asset.first);

				if (assetName.size() < maxAssetNameSize)
					assetName.append(std::string(maxAssetNameSize - assetName.size(), ' '));

				// should the GUID be padded somehow to make sure it's 16 characters long for viewing consistency?
				// some GUIDs aren't 16 chars when displayed...
				std::string print = assetName + " | " + Resource::GetGUIDHex(asset.second.first) + " | " + (asset.second.second ? "Compiled" : "Not Compiled");

				// make the displayed text highlightable/selectable
				// from https://github.com/ocornut/imgui/issues/950#issuecomment-840541529
				ImGui::PushID(print.c_str());
				ImGui::PushItemWidth(ImGui::GetWindowSize().x);
				ImGui::InputText("", (char*)print.c_str(), print.size(), ImGuiInputTextFlags_ReadOnly);
				ImGui::PopItemWidth();
				ImGui::PopID();
				ImGui::Spacing();
			}
			ImGui::End();
		}
	}

	void AssetPanel::Shutdown()
	{

	}

	void AssetPanel::OnAssetSelectEvent(const AssetSelectorEvent& /*event*/)
	{

	}

	void AssetPanel::OnAssetPanelEvent(const AssetPanelEvent& event)
	{
		m_ShowAssetPanel = event.m_ShowAssetPanel;
	}
}