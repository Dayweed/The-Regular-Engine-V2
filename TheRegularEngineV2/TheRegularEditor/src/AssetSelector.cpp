#include "pch.h"
#include "AssetSelector.h"

namespace TRE
{
	bool AssetSelector::IsAssetSelected(const ResourceHandle& resourceHandle)
	{
		return m_SelectedAsset == resourceHandle;
	}

	bool AssetSelector::IsAssetSelected(const std::string& assetName)
	{
		const auto GUID = AssetManager::Instance().GetAssetHandle(assetName);
		return m_SelectedAsset == GUID;
	}
	
	const ResourceHandle& AssetSelector::GetSelectedAsset()
	{
		return m_SelectedAsset;
	}

	void AssetSelector::ClearSelectedAsset()
	{
		m_SelectedAsset = 0;
	}
	
	void AssetSelector::SelectEntity(const ResourceHandle& resourceHandle)
	{
		m_SelectedAsset = resourceHandle;
	}

	void AssetSelector::SelectEntity(const std::string& assetName)
	{
		m_SelectedAsset = AssetManager::Instance().GetAssetHandle(assetName);
	}
}