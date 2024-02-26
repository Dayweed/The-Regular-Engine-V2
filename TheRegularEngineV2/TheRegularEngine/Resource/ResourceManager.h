#pragma once
#include "Resource.h"
#include "Core/Asserts.h"
#include "Graphics/RenderObject.h"
#include "Graphics/VulkanTexture.h"
#include "Graphics/Material.h"
#include "Graphics/Shader.h"

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
		template<typename T>
		void LoadResource(const std::string& hexHandle);
		void AddResource(std::unique_ptr<Resource> asset);
		void RemoveResource(ResourceHandle Handle);
		void UnloadUnusedResources();

		ResourceType GetResourceType(ResourceHandle handle);
		template <typename T>
		std::shared_ptr<T> GetResource(ResourceHandle Handle);
		template <typename T>
		std::shared_ptr<T> GetResource(const std::string& assetName);
		template <typename T>
		std::vector<std::shared_ptr<T>> GetResourcesOfType();

		template <typename N>
		void DestroyResourcesOfType(N type);
		void DestroyAllResources();

		const std::size_t GetAllResourceCount();
		const bool IsResourceLoaded(ResourceHandle handle);
		const bool IsResourceLoaded(const std::string& hexHandle);

		void RenameResource(ResourceHandle oldHandle, ResourceHandle newHandle);

		template<typename T>
		void SerializeResource(ResourceHandle handle);
		void SerializeAll();

	private:
		ResourceManager() = default;
		ResourceManager(ResourceManager const&) = delete;
		void operator=(ResourceManager const&) = delete;
		void* operator new(size_t) = delete;
	private:
		std::unordered_map<ResourceHandle, std::shared_ptr<Resource>> m_Resources;
	};

	template<typename T>
	void ResourceManager::LoadResource(const std::string& hexHandle)
	{
		const auto type = T::GetType();
		if (type == ResourceType::Texture)
		{
			std::unique_ptr<VulkanTexture> texture = std::make_unique<VulkanTexture>("../Resources/" + hexHandle + ".DDS");
			texture->SetHandle(Resource::GetGUIDFromHex(hexHandle));
			m_Resources[texture->GetHandle()] = std::move(texture);
		}
		else if (type == ResourceType::Mesh)
		{
			std::unique_ptr<RenderObject> ro = std::make_unique<RenderObject>("../Resources/" + hexHandle + ".geom");
			ro->SetHandle(Resource::GetGUIDFromHex(hexHandle));
			m_Resources[ro->GetHandle()] = std::move(ro);

		}
		else if (type == ResourceType::Material)
		{
			std::shared_ptr<Material> material = Material::Deserialize(hexHandle);
			(void)material;
		}
		else
		{
			TRE_CORE_INFO("Loading resource of type not supported");
		}
	}

	template <typename T>
	std::shared_ptr<T> ResourceManager::GetResource(ResourceHandle Handle)
	{
		if (m_Resources.find(Handle) == m_Resources.end())
		{
			//Try loading if not found
			LoadResource<T>(Resource::GetGUIDHex(Handle));

			if (m_Resources.find(Handle) == m_Resources.end())
			{
				return nullptr;
			}
		}
		return std::dynamic_pointer_cast<T>(m_Resources[Handle]);
	}

	template <typename T>
	std::shared_ptr<T> ResourceManager::GetResource(const std::string& assetName)
	{
		const auto handle = Resource::GenerateGUID(assetName);

		return GetResource<T>(handle);
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

	template<typename T>
	void ResourceManager::SerializeResource(ResourceHandle handle)
	{
		auto resource = GetResource<T>(handle);
		resource->Serialize();
	}
}