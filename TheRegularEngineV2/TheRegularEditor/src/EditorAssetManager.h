#pragma once
#include "Assets/Asset.h"
#include "pch.h"

namespace TRE
{
	class EditorAssetManager
	{
	public:
		static EditorAssetManager& Instance()
		{
			static EditorAssetManager instance;
			return instance;
		}

		void Initialize();
		void Shutdown();

		//Use for drag and drop
		void AddAsset(const std::string& assetName, std::unique_ptr<Asset> asset);
		bool Contains(const std::string& assetName);
	private:
		EditorAssetManager() = default;
		EditorAssetManager(const EditorAssetManager&) = delete;
		EditorAssetManager& operator=(const EditorAssetManager&) = delete;
		void* operator new(size_t) = delete;
	private:
		std::unordered_map<std::string, AssetHandle> m_AssetNameToHandle;
	};
}