#pragma once
#include "Asset.h"

namespace TRE
{
	class AssetManager
	{
		public:
			static AssetManager& Instance()
			{
				static AssetManager instance;
				return instance;
			}
		public:
			void AddAsset(std::unique_ptr<Asset> asset);

			AssetType GetAssetType(AssetHandle Handle);

			template <typename T>
			std::shared_ptr<T> GetAsset(AssetHandle Handle)
			{
				return std::dynamic_pointer_cast<T>(m_Assets[Handle]);
			}

			/*template <typename T, typename N>
			std::vector<std::shared_ptr<T>> GetAssetsOfType(N type)
			{
				std::vector<std::shared_ptr<T>> assets;
				for (auto& asset : m_Assets)
				{
					if (asset.second->GetType() == type)
					{
						assets.push_back(std::dynamic_pointer_cast<T>(asset.second));
					}
				}
				return assets;
			}*/

			template <typename N>
			void DestroyAssetsOfType(N type)
			{
				for (auto it = m_Assets.begin(); it != m_Assets.end();)
				{
					if (it->second->GetType() == type)
					{
						it = m_Assets.erase(it);
					}
					else
					{
						++it;
					}
				}
			}

			void DestroyAsset(AssetHandle Handle)
			{
				m_Assets[Handle].reset();
			}

			void DestroyAllAssets()
			{
				m_Assets.clear();
			}

			std::size_t GetAssetCount()
			{
				return m_Assets.size();
			}

		private:
			AssetManager() {};
			~AssetManager();
			AssetManager(AssetManager const&) = delete;
			void operator=(AssetManager const&) = delete;
			void* operator new(size_t) = delete;
		private:
			std::unordered_map<AssetHandle, std::shared_ptr<Asset>> m_Assets;
	};
}