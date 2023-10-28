#pragma once
#include "pch.h"
#include "EditorAssetManager.h"

namespace TRE
{
	class AssetSelector
	{
	public:
		enum class AssetType
		{
			Unknown,
			Material,
			Texture,
			Model,
			Scene,
			Audio,
			Font,
			Script,
			Animation,
			Particle,
			Prefab,
			Count
		};
	public:
		AssetType FindAssetType(const std::string& typeName);

		bool IsAssetSelected(const ResourceHandle& resourceHandle);
		bool IsAssetSelected(const std::string& assetName);
		const ResourceHandle& GetSelectedAsset();
		const std::string& GetSelectedAssetName();
		const AssetType GetSelectedAssetType();
		template <typename T>
		std::shared_ptr<T> GetSelectedAsset();
		void ClearSelectedAsset();
		//void SelectEntity(const ResourceHandle& resourceHandle, AssetType assetType);
		void SelectAsset(const std::string& assetName, AssetType assetType);

	private:
		ResourceHandle m_SelectedAsset{ 0 };
		std::string m_SelectedAssetName;
		AssetType m_SelectedAssetType{ AssetType::Unknown };
	};

	template <typename T>
	std::shared_ptr<T> AssetSelector::GetSelectedAsset()
	{
		return AssetManager::Instance().GetAsset<T>(m_SelectedAsset);
	}
}