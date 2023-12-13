#include "pch.h"
#include "EditorAssetManager.h"

namespace TRE
{
	void AssetManager::Poll()
	{
		// this function must run once at the very beginning to initialise variables
		static bool runOnce = false;

		// but perhaps, let's not poll every frame... 
		static std::time_t start_timer = std::time(nullptr);
		const long long result = std::time(nullptr) - start_timer;
		if (runOnce && result < 2) return;

		//Go through assets folder and link assetName to asset handle based on available descriptor files
		const std::filesystem::path assetsPath = "../Assets";
		const std::filesystem::path resourcePath = "../Resources";
		for (const auto& entry : std::filesystem::directory_iterator(assetsPath))
		{
			if (entry.path().extension() == ".desc")
			{
				std::ifstream file(entry.path());
				if (file.is_open())
				{
					std::string line;
					std::getline(file, line);
					if (line == "Asset File Path:")
					{
						std::getline(file, line);
						std::string assetName = line;
						assetName = assetName.substr(line.find_last_of('/') + 1);
						std::string assetHandle = entry.path().stem().string();
						assetHandle = assetHandle.substr(0, assetHandle.find_first_of('.'));
						std::string resourceCheck = resourcePath.string() + "/" + assetHandle;

						std::pair rscCheck = std::make_pair(Resource::GetGUIDFromHex(assetHandle), false);
						for (const auto& rscEntry : std::filesystem::directory_iterator(resourcePath))
						{
							// std::string rscHandle = rscEntry.path().stem().string();
							if (rscEntry.path().stem().string() == assetHandle)
							{
								rscCheck.second = true;
								break;
							}
						}
						m_AssetNameToHandle[assetName] = rscCheck;
					}
				}
				else
				{
					std::cout << "Could not open file: " << entry.path() << std::endl;
				}
				file.close();
			}
		}

		//RecompileAssetsOfType(ResourceType::Texture);

		runOnce = true;
		// reset timer
		std::time(&start_timer);
	}

	void AssetManager::Shutdown()
	{
		m_AssetNameToHandle.clear();
	}

	void AssetManager::AddAsset(const std::string& assetName, std::unique_ptr<Resource> asset)
	{
		m_AssetNameToHandle[assetName] = std::pair(asset->GetHandle(), true);
		ResourceManager::Instance().AddResource(std::move(asset));
	}

	void AssetManager::RemoveAsset(const std::string& assetName)
	{
		const ResourceHandle handle = m_AssetNameToHandle[assetName].first;
		ResourceType assetType = ResourceManager::Instance().GetResourceType(handle);
		std::filesystem::path assetsPath = "../Assets/";
		std::filesystem::path resourcePath = "../Resources/";
		const std::string hexHandle = Resource::GetGUIDHex(handle);
		//For physical files with intermediate, remove the intermediate file, descriptor file and resource file
		if (assetType == ResourceType::Mesh)
		{
			assetsPath += hexHandle + ".desc" + ".geom";
			std::filesystem::remove(assetsPath);
			assetsPath = "../Assets/";
			assetsPath += assetName + ".fbx";
			std::filesystem::remove(assetsPath);
			resourcePath += hexHandle + ".geom";
			std::filesystem::remove(resourcePath);
		}
		ResourceManager::Instance().RemoveResource(m_AssetNameToHandle[assetName].first);
	}

	void AssetManager::RenameAsset(const std::string& oldName, const std::string& newName)
	{
		//Regenerate new handle based on new name
		const ResourceType assetType = ResourceManager::Instance().GetResourceType(m_AssetNameToHandle[oldName].first);
		const ResourceHandle newHandle = Resource::GenerateGUID(newName);

		//Only type geom, texture and audio has physical asset file
		//Renaming of intermediate file
		if (assetType != ResourceType::Material)
		{
			//Rename asset file and update descriptor file
			std::filesystem::path assetsPath = "../Assets/";
			std::filesystem::path newAssetsPath = assetsPath;

			//Update asset name
			assetsPath += oldName;
			newAssetsPath += newName;
			if (std::filesystem::exists(assetsPath))
				std::filesystem::rename(assetsPath, newAssetsPath);
			else
				TRE_ERROR("AssetManager::RenameAsset: Asset with name {0} does not exist", oldName);
		}

		std::filesystem::path oldDescriptorPath = "../Assets/";
		//Update descriptor file
		oldDescriptorPath += Resource::GetGUIDHex(m_AssetNameToHandle[oldName].first);
		//Check for asset type
		if (assetType == ResourceType::Material)
			oldDescriptorPath += ".material";
		else if (assetType == ResourceType::Mesh)
			oldDescriptorPath += ".geom";
		else if (assetType == ResourceType::Texture)
			oldDescriptorPath += ".texture";
		else if (assetType == ResourceType::Shader)
			oldDescriptorPath += ".shader";
		else if(assetType == ResourceType::Audio)
			oldDescriptorPath += ".audio";
		else
			TRE_ERROR("AssetManager::RenameAsset: Asset with name {0} has invalid type", oldName);
		
		oldDescriptorPath += ".desc";

		//Rename old descriptor file
		if (std::filesystem::exists(oldDescriptorPath))
		{
			const std::string oldDescriptorPathString = oldDescriptorPath.string();
			std::string assetPath;
			std::string oldResourcePath;
			std::string resourcePath;
			const std::string hexHandle = Resource::GetGUIDHex(newHandle);
			std::string newDescriptorPath = oldDescriptorPathString.substr(0, oldDescriptorPathString.find_last_of('/') + 1);
			newDescriptorPath += hexHandle;
			if (assetType == ResourceType::Material)
			{
				newDescriptorPath += ".material";
				MaterialDescriptorFile materialDescriptor;
				materialDescriptor.ReadDescriptorFile(oldDescriptorPathString);
				assetPath = materialDescriptor.GetAssetPath();
				assetPath = assetPath.substr(0, assetPath.find_last_of('/') + 1);
				assetPath += newName;
				materialDescriptor.SetAssetPath(assetPath);
				oldResourcePath = materialDescriptor.GetResourcePath();
				resourcePath = oldResourcePath.substr(0, oldResourcePath.find_last_of('/') + 1);
				resourcePath += hexHandle;
				resourcePath += ".material";
				materialDescriptor.SetResourcePath(resourcePath);
				materialDescriptor.SetDescriptorPath(newDescriptorPath);
				materialDescriptor.Generate(newName);
			}

			//std::ifstream file(oldDescriptorPath);

			////Create new descriptor file temporarily for copying
			//std::filesystem::path newDescriptorPath = oldDescriptorPath;
			//newDescriptorPath += ".temp";

			////Write into this new temporary descriptor file
			//std::ofstream newFile(newDescriptorPath);
			//std::string line;
			//while (std::getline(file, line))
			//{
			//	if (line == "Asset File Path:")
			//	{
			//		newFile << line << std::endl;
			//		std::getline(file, line);
			//		line = line.substr(0, line.find_last_of('/') + 1);
			//		line += newName;
			//		newFile << line << std::endl;
			//	}
			//	else
			//	{
			//		newFile << line << std::endl;
			//	}
			//}
			//file.close();
			//newFile.close();

			//Delete old descriptor file and rename new one
			std::filesystem::remove(oldDescriptorPath);
			//std::filesystem::rename(newDescriptorPath, oldDescriptorPath);
		}
		else
			TRE_ERROR("AssetManager::RenameAsset: Asset descriptor with name {0} does not exist", oldName);

		m_AssetNameToHandle[newName] = { newHandle, m_AssetNameToHandle[oldName].second };
		//Update resource manager
		ResourceManager::Instance().RenameResource(m_AssetNameToHandle[oldName].first, newHandle);
		m_AssetNameToHandle.erase(oldName);
	}

	void AssetManager::RenameAsset(const ResourceHandle resourceHandle, const std::string& newName)
	{
		RenameAsset(GetName(resourceHandle), newName);
	}

	bool AssetManager::Contains(const std::string& assetName) const
	{
		return m_AssetNameToHandle.find(assetName) != m_AssetNameToHandle.end();
	}

	bool AssetManager::Contains(const ResourceHandle resourceHandle) const
	{
		for (const auto& x : m_AssetNameToHandle)
		{
			if (x.second.first == resourceHandle)
				return true;
		}
		return false;
	}

	bool AssetManager::Compiled(const std::string& assetName) const
	{
		if(Contains(assetName))
			return m_AssetNameToHandle.at(assetName).second;
		return false;
	}

	bool AssetManager::Compiled(const ResourceHandle resourceHandle) const
	{
		for (const auto& x : m_AssetNameToHandle)
		{
			if (x.second.first == resourceHandle)
				return x.second.second;
		}
		return false;
	}

	const ResourceHandle AssetManager::GetAssetHandle(const std::string& assetName) const
	{
		if(Contains(assetName))
			return m_AssetNameToHandle.at(assetName).first;
		
		return 0;
	}

	const std::string AssetManager::GetName(const ResourceHandle resourceHandle) const
	{
		std::string name;
		for (const auto& x : m_AssetNameToHandle)
		{
			if (x.second.first == resourceHandle)
				name = x.first;
		}
		return name;
	}

	const std::string AssetManager::GetName(const std::string& hexHandle) const
	{
		return GetName(Resource::GetGUIDFromHex(hexHandle));
	}

	void AssetManager::RecompileAssetsOfType(const ResourceType type)
	{
		for (auto& file : m_AssetNameToHandle)
		{
			//If compiled before, recompile again
			if (file.second.second)
			{
				if (type == ResourceType::Texture)
				{
					//This is wrong next time fix
					if (file.first.find(".png") != std::string::npos)
					{
						std::cout << file.first << " | " << Resource::GetGUIDHex(file.second.first) << std::endl;
						CompileAndLoad<VulkanTexture>(file.first);
					}
				}
			}
		}

		if (type == ResourceType::Mesh)
		{
			//RecompileMeshes();
		}
		else if (type == ResourceType::Texture)
		{
			//RecompileTextures();
			
		}
	}

	void AssetManager::PrintAllAssets() const
	{
		for (const auto x : m_AssetNameToHandle)
		{
			std::cout << x.first << "| " << x.second.first << (x.second.second ? "Compiled" : "Not Compiled") << "| " << Resource::GetGUIDHex(x.second.first) << std::endl;
		}
	}
}