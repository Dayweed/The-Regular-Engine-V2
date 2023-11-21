#pragma once
#include "Panel.h"
#include "AssetSelector.h"

namespace TRE
{
	class AssetPanel : public Panel
	{
	public:
		AssetPanel(const std::shared_ptr<AssetSelector>& assetSelector);
		void Init() override;
		void Update() override;
		void Shutdown() override;

		void OnAssetSelectEvent(const AssetSelectorEvent& event);
		void OnAssetPanelEvent(const AssetPanelEvent& event);
	private:
		std::shared_ptr<AssetSelector> m_AssetSelector;
		bool m_ShowAssetPanel{ false };
	};
}