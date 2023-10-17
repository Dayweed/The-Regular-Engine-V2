#include "pch.h"
#include "EditorAssetManager.h"

namespace TRE
{
	void AssetManager::Initialize()
	{
		//Go throught assets folder and link assetname to asset handle based on available descriptor files
		std::filesystem::path assetsPath = "../Assets";
		std::filesystem::path resourcePath = "../Resources";
		for (const auto& entry : std::filesystem::directory_iterator(assetsPath))
		{
			std::string extension = entry.path().extension().string();
			if (entry.path().extension() == ".desc")
			{
				std::ifstream file(entry.path());
				std::string assetHandle = entry.path().stem().string();
				assetHandle = assetHandle.substr(0, assetHandle.find_first_of('.'));
				if (file.is_open())
				{
					std::string line;
					std::getline(file, line);
					if (line == "Asset File Path:")
					{
						std::getline(file, line);
						std::string assetName = line;
						assetName = assetName.substr(line.find_last_of('/') + 1);
						std::string resourceCheck = resourcePath.string() + "/" + assetHandle;
						
						for (const auto& rscEntry : std::filesystem::directory_iterator(resourcePath))
						{
							std::string rscHandle = rscEntry.path().stem().string();
							if (rscHandle == assetHandle)
							{
								m_AssetNameToHandle[assetName] = Resource::GetGUIDFromHex(assetHandle);
								break;
							}
						}
					}
				}
				else
				{
					std::cout << "Could not open file: " << entry.path() << std::endl;
				}
				file.close();
			}
		}
	}

	void AssetManager::Shutdown()
	{
		m_AssetNameToHandle.clear();
	}

	void AssetManager::AddAsset(const std::string& assetName, std::unique_ptr<Resource> asset)
	{
		m_AssetNameToHandle[assetName] = asset->GetHandle();
		ResourceManager::Instance().AddResource(std::move(asset));
	}

	void AssetManager::RemoveAsset(const std::string& assetName)
	{
		ResourceManager::Instance().RemoveResource(m_AssetNameToHandle[assetName]);
	}

	bool AssetManager::Contains(const std::string& assetName) const
	{
		return m_AssetNameToHandle.find(assetName) != m_AssetNameToHandle.end();
	}

	bool AssetManager::Contains(const ResourceHandle resourceHandle) const
	{
		for (auto x : m_AssetNameToHandle)
		{
			if (x.second == resourceHandle)
				return true;
		}
		return false;
	}

	const ResourceHandle AssetManager::GetAssetHandle(const std::string& assetName) const
	{
		if(Contains(assetName))
			return m_AssetNameToHandle.at(assetName);
		
		TRE_ERROR("AssetManager::GetAssetHandle: Asset with name {0} does not exist", assetName);
		return 0;
	}

	const std::string AssetManager::GetName(const ResourceHandle resourceHandle) const
	{
		std::string name;
		for (auto x : m_AssetNameToHandle)
		{
			if (x.second == resourceHandle)
				name = x.first;
		}
		return name;
	}

	void AssetManager::PrintAllAssets() const
	{
		for (const auto x : m_AssetNameToHandle)
		{
			std::cout << x.first << "| " << x.second <<"| " << Resource::GetGUIDHex(x.second) << std::endl;
		}
	}
}