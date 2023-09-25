#pragma once
#include "Resource/Resource.h"
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
		void AddAsset(const std::string& assetName, std::unique_ptr<Resource> asset);
		void RemoveAsset(const std::string& assetName);
		bool Contains(const std::string& assetName);

		ResourceHandle GetAsset(const std::string& assetName);
		std::string GetName(const ResourceHandle resourceHandle);
	private:
		EditorAssetManager() = default;
		EditorAssetManager(const EditorAssetManager&) = delete;
		EditorAssetManager& operator=(const EditorAssetManager&) = delete;
		void* operator new(size_t) = delete;
	private:
		std::unordered_map<std::string, ResourceHandle> m_AssetNameToHandle;
	};
}