#include "pch.h"
#include "AssetManager.h"
#include "Asset.h"

namespace TRE
{
	void AssetManager::AddAsset(std::unique_ptr<Asset> asset)
	{
		m_Assets[asset->GetHandle()] = std::move(asset);
	}

	AssetType AssetManager::GetAssetType(AssetHandle Handle)
	{
		return m_Assets[Handle]->GetType();
	}

	AssetManager::~AssetManager()
	{

	}
}