#include "TexturePanel.h"
#include "TREIncludes.h"

namespace TRE
{
	TexturePanel::TexturePanel(const std::shared_ptr<AssetSelector>& assetSelector)
	{
		m_AssetSelector = assetSelector;
	}

	void TexturePanel::Init()
	{

	}

	void TexturePanel::Update()
	{
		if (m_AssetSelector->GetSelectedAssetType() == AssetSelector::AssetType::Texture)
		{
			ImGui::Begin("Texture Panel");

			const std::string& assetName = m_AssetSelector->GetSelectedAssetName();
			std::string toPrint;


			//Check if compiled before
			if (const auto resourceHandle = m_AssetSelector->GetSelectedAsset(); resourceHandle && AssetManager::Instance().Compiled(resourceHandle))
			{
				toPrint = "Texture " + assetName;
				ImGui::Text(toPrint.c_str());
			}
			else
			{
				toPrint = "Texture " + assetName + " not compiled yet";
				ImGui::Text(toPrint.c_str());
				if (ImGui::Button("Compile"))
				{
					auto texture = AssetManager::Instance().CompileAndLoad<VulkanTexture>(assetName);
					m_AssetSelector->UpdateSelectedAssetHandle(texture->GetHandle());
				}
			}
			ImGui::End();
		}
	}

	void TexturePanel::Shutdown()
	{

	}
}