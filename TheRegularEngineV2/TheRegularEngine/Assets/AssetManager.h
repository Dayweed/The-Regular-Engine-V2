#pragma once
#include "Asset.h"
#include "Core/Asserts.h"

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
			template <typename T>
			void LoadAsset(AssetHandle handle)
			{
				std::unique_ptr<T> asset = std::make_unique<T>(handle);
				asset->SetHandle(handle);
				AddAsset(std::move(asset));
			}

			void AddAsset(std::unique_ptr<Asset> asset);

			AssetType GetAssetType(AssetHandle Handle);

			template <typename T>
			std::shared_ptr<T> GetAsset(AssetHandle Handle)
			{
				if (m_Assets.find(Handle) == m_Assets.end())
				{
					std::stringstream ss;
					ss << std::hex << Handle;
					TRE_CORE_ERROR("Couldnt find asset of handle: " + ss.str());
					return nullptr;
				}
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

			std::size_t GetAllAssetCount()
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