#include "pch.h"
#include "AssetManager.h"
#include "Asset.h"

#include "Core/Asserts.h"
#include "Graphics/RenderObject.h"
#include "Graphics/VulkanTexture.h"
#include "Graphics/Material.h"
#include "Graphics/Shader.h"

namespace TRE
{
	template <typename T>
	void AssetManager::LoadAsset(AssetHandle handle)
	{
		std::unique_ptr<T> asset = std::make_unique<T>();
		if (T == RenderObject || T == VulkanTexture)
			asset = std::move(std::make_unique<T>(handle));
		else if (T == Shader)
		{

		}
		else if (T == Material)
		{

		}
		else
		{
			TRE_CORE_CRITICAL("Asset type not supported");
		}
		asset->SetHandle(handle);
		AddAsset(std::move(asset));
	}

	void AssetManager::AddAsset(std::unique_ptr<Asset> asset)
	{
		m_Assets[asset->GetHandle()] = std::move(asset);
	}

	AssetType AssetManager::GetAssetType(AssetHandle Handle)
	{
		return m_Assets[Handle]->GetType();
	}

	AssetManager::~AssetManager()
	{

	}
}