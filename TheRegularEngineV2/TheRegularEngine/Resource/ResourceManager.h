#pragma once
#include "Resource.h"
#include "Core/Asserts.h"

namespace TRE
{
	class ResourceManager
	{
	public:
		static ResourceManager& Instance()
		{
			static ResourceManager instance;
			return instance;
		}
	public:
		void AddResource(std::unique_ptr<Resource> asset);
		void RemoveResource(ResourceHandle Handle);
		void UnloadUnusedResources();

		ResourceType GetResourceType(ResourceHandle handle);

		template <typename T>
		std::shared_ptr<T> GetResource(ResourceHandle Handle);

		template <typename T>
		std::vector<std::shared_ptr<T>> GetResourcesOfType();

		template <typename N>
		void DestroyResourcesOfType(N type);

		void DestroyAllResources()
		{
			m_Resources.clear();
		}

		std::size_t GetAllResourceCount()
		{
			return m_Resources.size();
		}

		void SerializeAll()
		{
			UnloadUnusedResources();
			for (auto& asset : m_Resources)
			{
				if(asset.second->GetType() == ResourceType::Material)
					asset.second->Serialize();
			}
		}

	private:
		ResourceManager() {};
		~ResourceManager();
		ResourceManager(ResourceManager const&) = delete;
		void operator=(ResourceManager const&) = delete;
		void* operator new(size_t) = delete;
	private:
		std::unordered_map<ResourceHandle, std::shared_ptr<Resource>> m_Resources;
	};

	template <typename T>
	std::shared_ptr<T> ResourceManager::GetResource(ResourceHandle Handle)
	{
		if (m_Resources.find(Handle) == m_Resources.end())
		{
			return nullptr;
		}
		return std::dynamic_pointer_cast<T>(m_Resources[Handle]);
	}

	template <typename T>
	std::vector<std::shared_ptr<T>> ResourceManager::GetResourcesOfType()
	{
		ResourceType type = T::GetType();

		std::vector<std::shared_ptr<T>> assets;
		for (auto& asset : m_Resources)
		{
			if (asset.second->GetType() == type)
			{
				assets.push_back(std::dynamic_pointer_cast<T>(asset.second));
			}
		}
		return assets;
	}

	template <typename N>
	void ResourceManager::DestroyResourcesOfType(N type)
	{
		for (auto it = m_Resources.begin(); it != m_Resources.end();)
		{
			if (it->second->GetType() == type)
			{
				it = m_Resources.erase(it);
			}
			else
			{
				++it;
			}
		}
	}
}