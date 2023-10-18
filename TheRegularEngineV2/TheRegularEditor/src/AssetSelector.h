#pragma once
#include "pch.h"
#include "EditorAssetManager.h"

namespace TRE
{
	class AssetSelector
	{
	public:
		bool IsAssetSelected(const ResourceHandle& resourceHandle);
		bool IsAssetSelected(const std::string& assetName);
		const ResourceHandle& GetSelectedAsset();
		template <typename T>
		std::shared_ptr<T> GetSelectedAsset();
		void ClearSelectedAsset();
		void SelectEntity(const ResourceHandle& resourceHandle);
		void SelectEntity(const std::string& assetName);

	private:
		ResourceHandle m_SelectedAsset{ 0 };
	};

	template <typename T>
	std::shared_ptr<T> AssetSelector::GetSelectedAsset()
	{
		return AssetManager::Instance().GetAsset<T>(m_SelectedAsset);
	}
}