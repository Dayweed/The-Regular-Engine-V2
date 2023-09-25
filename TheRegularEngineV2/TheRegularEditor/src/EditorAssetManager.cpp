#include "pch.h"
#include "EditorAssetManager.h"
#include "Resource/ResourceManager.h"

namespace TRE
{
	void EditorAssetManager::Initialize()
	{
		//Go throught assets folder and link assetname to asset handle based on available descriptor files
		std::filesystem::path assetsPath = "../Assets";
		std::filesystem::path resourcePath = "../Resources";
		for (const auto& entry : std::filesystem::directory_iterator(assetsPath))
		{
			if (entry.path().extension() == ".desc")
			{
				std::ifstream file(entry.path());
				std::string assetHandle = entry.path().stem().string();
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
				file.close();
			}
		}
	}
	void EditorAssetManager::Shutdown()
	{
		m_AssetNameToHandle.clear();
	}

	void EditorAssetManager::AddAsset(const std::string& assetName, std::unique_ptr<Resource> asset)
	{
		m_AssetNameToHandle[assetName] = asset->GetHandle();
		ResourceManager::Instance().AddResource(std::move(asset));
	}

	void EditorAssetManager::RemoveAsset(const std::string& assetName)
	{
		ResourceManager::Instance().RemoveResource(m_AssetNameToHandle[assetName]);
	}

	bool EditorAssetManager::Contains(const std::string& assetName)
	{
		return m_AssetNameToHandle.find(assetName) != m_AssetNameToHandle.end();
	}

	ResourceHandle EditorAssetManager::GetAsset(const std::string& assetName)
	{
		return m_AssetNameToHandle[assetName];
	}

	std::string EditorAssetManager::GetName(const ResourceHandle resourceHandle)
	{
		std::string name;
		for (auto x : m_AssetNameToHandle)
		{
			if (x.second == resourceHandle)
				name = x.second;
		}
		return name;
	}
}