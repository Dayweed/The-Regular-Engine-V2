#pragma once
#include "pch.h"
#include "EditorAssetManager.h"
#include "EventSystem/Events/EditorEvent.h"

namespace TRE
{
	class AssetSelector
	{
	public:
		AssetSelectorEvent::AssetType FindAssetType(const std::string& typeName);

		bool IsAssetSelected(const ResourceHandle& resourceHandle);
		bool IsAssetSelected(const std::string& assetName);
		const ResourceHandle& GetSelectedAsset();
		const std::string& GetSelectedAssetName();
		const AssetSelectorEvent::AssetType GetSelectedAssetType();
		template <typename T>
		std::shared_ptr<T> GetSelectedAsset();
		void ClearSelectedAsset();
		void SelectAsset(const std::string& assetName, AssetSelectorEvent::AssetType assetType);
		void UpdateSelectedAssetHandle(const ResourceHandle handle);

	private:
		ResourceHandle m_SelectedAsset{ 0 };
		std::string m_SelectedAssetName;
		AssetSelectorEvent::AssetType m_SelectedAssetType{ AssetSelectorEvent::AssetType::Unknown };
	};

	template <typename T>
	std::shared_ptr<T> AssetSelector::GetSelectedAsset()
	{
		return AssetManager::Instance().GetAsset<T>(m_SelectedAsset);
	}
}