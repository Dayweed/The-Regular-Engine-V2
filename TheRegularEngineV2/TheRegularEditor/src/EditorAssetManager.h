#pragma once
#include "pch.h"
#include "Resource/Resource.h"

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

		void Initialize();
		void Shutdown();

		//Use for drag and drop
		void AddAsset(const std::string& assetName, std::unique_ptr<Resource> asset);
		template<typename T>
		void CompileAndLoad(const std::string& assetName);
		void RemoveAsset(const std::string& assetName);
		bool Contains(const std::string& assetName);

		const ResourceHandle GetAsset(const std::string& assetName) const;
		const std::string GetName(const ResourceHandle resourceHandle) const;

		void PrintAllAssets() const;
	private:
		AssetManager() = default;
		AssetManager(const AssetManager&) = delete;
		AssetManager& operator=(const AssetManager&) = delete;
		void* operator new(size_t) = delete;
	private:
		std::unordered_map<std::string, ResourceHandle> m_AssetNameToHandle;
	};

	template<typename T>
	void AssetManager::CompileAndLoad(const std::string& assetName)
	{
		//Generate GUID
		//Create descriptorfile
		//Compile
		//Load

		const std::string hex = Resource::GetGUIDHex(Resource::GenerateGUID());
		ResourceType type = T::GetType();
		if (type == ResourceType::Mesh)
		{
			
		}
		else
		{

		}

	}
}