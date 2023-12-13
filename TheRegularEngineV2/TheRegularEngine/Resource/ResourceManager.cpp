#include "pch.h"
#include "ResourceManager.h"
#include "Resource.h"

namespace TRE
{
	void ResourceManager::AddResource(std::unique_ptr<Resource> asset)
	{
		m_Resources[asset->GetHandle()] = std::move(asset);
	}

	void ResourceManager::RemoveResource(ResourceHandle Handle)
	{
		m_Resources.erase(Handle);
	}

	void ResourceManager::UnloadUnusedResources()
	{
		for (auto it = m_Resources.begin(); it != m_Resources.end();)
		{
			if (it->second.use_count() == 0)
			{
				it = m_Resources.erase(it);
			}
			else
			{
				++it;
			}
		}
	}

	ResourceType ResourceManager::GetResourceType(ResourceHandle Handle)
	{
		return m_Resources[Handle]->GetType();
	}

	void ResourceManager::DestroyAllResources()
	{
		m_Resources.clear();
	}

	const std::size_t ResourceManager::GetAllResourceCount()
	{
		return m_Resources.size();
	}

	void ResourceManager::SerializeAll()
	{
		UnloadUnusedResources();
		for (auto& asset : m_Resources)
		{
			if (asset.second->GetType() == ResourceType::Material)
				asset.second->Serialize();
		}
	}

	const bool ResourceManager::IsResourceLoaded(ResourceHandle handle)
	{
		return m_Resources.find(handle) != m_Resources.end();
	}

	const bool ResourceManager::IsResourceLoaded(const std::string& hexHandle)
	{
		return IsResourceLoaded(Resource::GetGUIDFromHex(hexHandle));
	}

	void ResourceManager::RenameResource(ResourceHandle oldHandle, ResourceHandle newHandle)
	{
		m_Resources[newHandle] = m_Resources[oldHandle];
		m_Resources.erase(oldHandle);
		m_Resources[newHandle]->SetHandle(newHandle);
	}
}