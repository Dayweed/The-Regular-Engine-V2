#pragma once
#include "pch.h"
#include "Resource/ResourceManager.h"
#include "Geom.h"
#include "GeomDescriptorFile.h"
#include "Graphics/RenderObject.h"

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
		std::shared_ptr<T> CompileAndLoad(const std::string& assetName);
		void RemoveAsset(const std::string& assetName);
		
		bool Contains(const std::string& assetName) const;
		const ResourceHandle GetAssetHandle(const std::string& assetName) const;
		template<typename T>
		std::shared_ptr<T> GetAsset(const std::string& assetName);
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
	std::shared_ptr<T> AssetManager::CompileAndLoad(const std::string& assetName)
	{
		//Generate GUID
		//Create descriptorfile
		//Compile
		//Load

		const ResourceHandle handle = Resource::GenerateGUID();
		const std::string hex = Resource::GetGUIDHex(handle);
		const std::string assetFolderPath = "../Assets/";
		const std::string resourceFolderPath = "../Resources/";
		ResourceType type = T::GetType();
		if (type == ResourceType::Mesh)
		{
			//Create DescriptorFile
			GeomDescriptorFile descriptorFile;
			const std::string assetPath = assetFolderPath + assetName;
			const std::string descPath = assetFolderPath + hex + ".geom" + ".desc";
			const std::string resourcePath = resourceFolderPath + hex + ".geom";
			descriptorFile.SetAssetPath(assetPath);
			descriptorFile.SetResourcePath(resourcePath);
			descriptorFile.SetDescriptorPath(descPath);
			descriptorFile.GenerateDescriptorFile();
			//Compile
			Geom::RunCompiler(descPath);
			//Load
			std::unique_ptr<RenderObject> ro = std::make_unique<RenderObject>(resourcePath);
			ro->SetHandle(handle);
			AddAsset(assetName, std::move(ro));

			return ResourceManager::Instance().GetResource<T>(handle);
		}
		else
		{

		}

		return nullptr;
	}

	template<typename T>
	std::shared_ptr<T> AssetManager::GetAsset(const std::string& assetName)
	{
		if(Contains(assetName))
			return ResourceManager::Instance().GetResource<T>(m_AssetNameToHandle.at(assetName));
		
		return nullptr;
	}
}