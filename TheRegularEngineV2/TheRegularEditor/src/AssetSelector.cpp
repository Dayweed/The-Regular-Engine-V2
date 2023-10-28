#include "pch.h"
#include "AssetSelector.h"

namespace TRE
{
	AssetSelector::AssetType AssetSelector::FindAssetType(const std::string& typeName)
	{
		if (typeName == "m_Material")
			return AssetType::Material;
		else if (typeName == "m_TextureResource")
			return AssetType::Texture;
		else if (typeName == "m_3DObject")
			return AssetType::Model;
		else if (typeName == "m_Scene")
			return AssetType::Scene;
		else if (typeName == "m_AudioResource")
			return AssetType::Audio;
		else if (typeName == "m_FontResource")
			return AssetType::Font;
		else if (typeName == "Script")
			return AssetType::Script;
		//else if (typeName == "Animation")
		//	return AssetType::Animation;
		else if (typeName == "Particle")
			return AssetType::Particle;
		else if (typeName == "m_Prefab")
			return AssetType::Prefab;
		else
			TRE_INFO("Unknown asset type: {0}", typeName);
			return AssetType::Unknown;
	}

	bool AssetSelector::IsAssetSelected(const ResourceHandle& resourceHandle)
	{
		return m_SelectedAsset == resourceHandle;
	}

	bool AssetSelector::IsAssetSelected(const std::string& assetName)
	{
		const auto GUID = AssetManager::Instance().GetAssetHandle(assetName);
		return m_SelectedAsset == GUID;
	}
	
	const ResourceHandle& AssetSelector::GetSelectedAsset()
	{
		return m_SelectedAsset;
	}

	const std::string& AssetSelector::GetSelectedAssetName()
	{
		return m_SelectedAssetName;
	}

	const AssetSelector::AssetType AssetSelector::GetSelectedAssetType()
	{
		return m_SelectedAssetType;
	}

	void AssetSelector::ClearSelectedAsset()
	{
		m_SelectedAsset = 0;
		m_SelectedAssetName = "";
		m_SelectedAssetType = AssetType::Unknown;
	}
	
	/*void AssetSelector::SelectEntity(const ResourceHandle& resourceHandle, AssetType assetType)
	{
		m_SelectedAsset = resourceHandle;
		m_SelectedAssetName = AssetManager::Instance().GetName(resourceHandle);
	}*/

	void AssetSelector::SelectAsset(const std::string& assetName, AssetType assetType)
	{
		m_SelectedAssetName = assetName;
		m_SelectedAssetType = assetType;
		m_SelectedAsset = AssetManager::Instance().GetAssetHandle(assetName);

		std::cout << "Selected asset: " << assetName << std::endl;
		std::cout << "Selected asset GUID: " << Resource::GetGUIDHex(m_SelectedAsset) << std::endl;

		//Check if material has been loaded for material panel to see
		if (assetType == AssetType::Material && m_SelectedAsset)
		{
			const auto rscHandle = AssetManager::Instance().GetAssetHandle(assetName);
			if (ResourceManager::Instance().IsResourceLoaded(rscHandle) == false)
			{
				auto material = Material::Deserialize(Resource::GetGUIDHex(rscHandle));
				(void)material;
			}
			std::cout <<"Material loaded" << std::endl;
		}
	}
}