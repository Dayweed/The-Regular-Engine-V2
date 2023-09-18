#include "pch.h"
#include "EditorAssetManager.h"
#include "Assets/AssetManager.h"

namespace TRE
{
	void EditorAssetManager::Initialize()
	{
		//Go throught assets folder and link assetname to asset handle based on available descriptor files
		std::filesystem::path assetsPath = "../Assets";
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
						m_AssetNameToHandle[assetName] = Asset::GetGUIDFromHex(assetHandle);
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

	void EditorAssetManager::AddAsset(const std::string& assetName, std::unique_ptr<Asset> asset)
	{
		m_AssetNameToHandle[assetName] = asset->GetHandle();
		AssetManager::Instance().AddAsset(std::move(asset));
	}

	bool EditorAssetManager::Contains(const std::string& assetName)
	{
		return m_AssetNameToHandle.find(assetName) != m_AssetNameToHandle.end();
	}
}