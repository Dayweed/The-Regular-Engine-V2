#pragma once
#include "pch.h"
#include "Resource/ResourceManager.h"
#include "Geom.h"
#include "GeomDescriptorFile.h"
#include "Graphics/RenderObject.h"
#include "Graphics/VulkanTexture.h"
#include "TextureDescriptorFile.h"

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
		template <typename T>
		void AddAsset(const std::string& assetName);
		template<typename T>
		std::shared_ptr<T> CompileAndLoad(const std::string& assetName);
		void RemoveAsset(const std::string& assetName);

		//Like dangerous
		void RenameAsset(const std::string& oldName, const std::string& newName);
		void RenameAsset(const ResourceHandle resourceHandle, const std::string& newName);
		
		bool Contains(const std::string& assetName) const;
		bool Contains(const ResourceHandle resourceHandle) const;
		bool Compiled(const std::string& assetName) const;
		bool Compiled(const ResourceHandle resourceHandle) const;
		const ResourceHandle GetAssetHandle(const std::string& assetName) const;
		template<typename T>
		std::shared_ptr<T> GetAsset(const std::string& assetName);
		template<typename T>
		std::vector<std::shared_ptr<T>> GetAssetsOfType();
		const std::string GetName(const ResourceHandle resourceHandle) const;
		const std::string GetName(const std::string& hexHandle) const;

		void PrintAllAssets() const;
	private:
		AssetManager() = default;
		AssetManager(const AssetManager&) = delete;
		AssetManager& operator=(const AssetManager&) = delete;
		void* operator new(size_t) = delete;
	private:
		std::unordered_map<std::string, std::pair<ResourceHandle, bool>> m_AssetNameToHandle;
	};

	template <typename T>
	void AssetManager::AddAsset(const std::string& assetName)
	{
		const auto GUID = AssetManager::Instance().GetAssetHandle(assetName);
		const auto hexGUID = Resource::GetGUIDHex(GUID);

		const std::string assetFolderPath = "../Assets/";
		const std::string resourceFolderPath = "../Resources/";
		ResourceType type = T::GetType();
		if (type == ResourceType::Mesh)
		{
			std::unique_ptr<T> geom = std::make_unique<T>(resourceFolderPath + hexGUID + ".geom");
			geom->SetHandle(GUID);
			AssetManager::Instance().AddAsset(assetName, std::move(geom));

		}
		else if (type == ResourceType::Texture)
		{
			std::unique_ptr<T> texture = std::make_unique<T>(resourceFolderPath + hexGUID + ".DDS");
			texture->SetHandle(GUID);
			AssetManager::Instance().AddAsset(assetName, std::move(texture));
		}
		else
		{
			TRE_CORE_ERROR("AssetManager::AddAsset: Unsupported type");
		}
	}

	template<typename T>
	std::shared_ptr<T> AssetManager::CompileAndLoad(const std::string& assetName)
	{
		//Generate GUID
		//Create descriptorfile
		//Compile
		//Load

		ResourceHandle handle;
		if(m_AssetNameToHandle.contains(assetName))
			handle = m_AssetNameToHandle.at(assetName).first;
		else
			handle = Resource::GenerateGUID();
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
		else if (type == ResourceType::Texture)
		{
			TextureDescriptorFile descriptorFile;
			const std::string assetPath = assetFolderPath + assetName;
			const std::string descPath = assetFolderPath + hex + ".texture" + ".desc";
			const std::string resourcePath = resourceFolderPath + hex + ".DDS";
			descriptorFile.SetAssetPath(assetPath);
			descriptorFile.SetResourcePath(resourcePath);
			descriptorFile.SetDescriptorPath(descPath);
			descriptorFile.GenerateDescriptorFile();
			//Compile
			Texture::RunCompiler(descPath);
			//Load
			std::unique_ptr<VulkanTexture> texture = std::make_unique<VulkanTexture>(resourcePath);
			texture->SetHandle(handle);
			AddAsset(assetName, std::move(texture));

			return ResourceManager::Instance().GetResource<T>(handle);
		}

		return nullptr;
	}

	template<typename T>
	std::shared_ptr<T> AssetManager::GetAsset(const std::string& assetName)
	{
		//if compiled before return
		if(Compiled(assetName))
			return ResourceManager::Instance().GetResource<T>(m_AssetNameToHandle.at(assetName).first);

		//If not try compiling
		return CompileAndLoad<T>(assetName);
	}

	template<typename T>
	std::vector<std::shared_ptr<T>> AssetManager::GetAssetsOfType()
	{
		std::vector<std::shared_ptr<T>> assets;
		for (auto& resource : ResourceManager::Instance().GetResourcesOfType<T>())
		{
			if(Contains(resource->GetHandle()))
				assets.push_back(resource);
		}
		return assets;
	}
}