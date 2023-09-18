#include "pch.h"
#include "AssetManager.h"
#include "Asset.h"

namespace TRE
{
	void AssetManager::AddAsset(std::unique_ptr<Asset> asset)
	{
		m_Assets[asset->GetHandle()] = std::move(asset);
	}

	void AssetManager::UnloadUnusedAssets()
	{
		for (auto it = m_Assets.begin(); it != m_Assets.end();)
		{
			if (it->second.use_count() == 1)
			{
				it = m_Assets.erase(it);
			}
			else
			{
				++it;
			}
		}
	}

	AssetType AssetManager::GetAssetType(AssetHandle Handle)
	{
		return m_Assets[Handle]->GetType();
	}

	AssetManager::~AssetManager()
	{

	}
}