#pragma once
#include "Asset.h"

namespace TRE
{
	class AssetManager
	{
		public:
			AssetManager();
			~AssetManager();

		public:
			AssetType GetAssetType(AssetHandle Handle);
			std::shared_ptr<Asset> GetAsset(AssetHandle Handle);

		private:
			std::unordered_map<AssetHandle, std::shared_ptr<Asset>> m_Assets;
	};
}