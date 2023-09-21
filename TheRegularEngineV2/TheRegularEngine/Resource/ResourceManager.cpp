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
			if (it->second.use_count() == 1)
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

	ResourceManager::~ResourceManager()
	{

	}
}