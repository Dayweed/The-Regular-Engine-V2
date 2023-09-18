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
			void AddAsset(std::unique_ptr<Asset> asset);
			void UnloadUnusedAssets();

			AssetType GetAssetType(AssetHandle handle);

			template <typename T>
			std::shared_ptr<T> GetAsset(AssetHandle Handle);

			template <typename T>
			std::vector<std::shared_ptr<T>> GetAssetsOfType();

			void DestroyAsset(AssetHandle Handle)
			{
				m_Assets[Handle].reset();
			}

			template <typename N>
			void DestroyAssetsOfType(N type);

			void DestroyAllAssets()
			{
				m_Assets.clear();
			}

			std::size_t GetAllAssetCount()
			{
				return m_Assets.size();
			}

			void Serialize()
			{
				UnloadUnusedAssets();
				for (auto& asset : m_Assets)
				{
					if(asset.second->GetType() == AssetType::Material)
						asset.second->Serialize();
				}
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

	template <typename T>
	std::shared_ptr<T> AssetManager::GetAsset(AssetHandle Handle)
	{
		if (m_Assets.find(Handle) == m_Assets.end())
		{
			/*std::stringstream ss;
			ss << std::hex << Handle;
			TRE_CORE_ERROR("Couldnt find asset of handle: " + ss.str());*/
			return nullptr;
		}
		return std::dynamic_pointer_cast<T>(m_Assets[Handle]);
	}

	template <typename T>
	std::vector<std::shared_ptr<T>> AssetManager::GetAssetsOfType()
	{
		AssetType type = T::GetType();

		std::vector<std::shared_ptr<T>> assets;
		for (auto& asset : m_Assets)
		{
			if (asset.second->GetType() == type)
			{
				assets.push_back(std::dynamic_pointer_cast<T>(asset.second));
			}
		}
		return assets;
	}

	template <typename N>
	void AssetManager::DestroyAssetsOfType(N type)
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
}