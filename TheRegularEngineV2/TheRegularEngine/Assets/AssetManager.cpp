#include "pch.h"
#include "AssetManager.h"
#include "Asset.h"

namespace TRE
{
	AssetType AssetManager::GetAssetType(AssetHandle Handle)
	{
		return m_Assets[Handle]->GetAssetType();
	}

	std::shared_ptr<Asset> AssetManager::GetAsset(AssetHandle Handle)
	{
		return m_Assets[Handle];
	}

	AssetManager::AssetManager()
	{


	}

	AssetManager::~AssetManager()
	{

	}
}