#include "pch.h"
#include "MaterialPanel.h"
#include "EditorAssetManager.h"

namespace TRE
{
	MaterialPanel::MaterialPanel(const std::shared_ptr<SelectionManager>& Selection_Manager)
	{
		m_SelectionManager = Selection_Manager;
	}

	MaterialPanel::~MaterialPanel()
	{

	}

	void MaterialPanel::Init()
	{

	}

	void MaterialPanel::Update()
	{
		if (auto entity = m_SelectionManager->GetSelectedEntity())
		{
			if (entity->HasComponent<MeshRenderer>() == false)
				return;

			ImGui::Begin("Material", nullptr, ImGuiWindowFlags_NoCollapse);

			MeshRenderer& meshRenderer = entity->GetComponent<MeshRenderer>();
			std::shared_ptr<Material> material = meshRenderer.m_MaterialInstance;
			if (material)
			{
				auto& textures = material->GetTexturesRef();
				for (auto& texture : textures)
				{
					DrawTexture(texture);
				}

				/*for (std::pair<const std::string, std::shared_ptr<VulkanTexture>>& textures : meshRenderer.m_MaterialInstance->GetTexturesRef())
				{
					DrawTexture(textures);
				}*/
			}

			ImGui::End();
		}		
	}

	void MaterialPanel::Shutdown()
	{

	}

	void MaterialPanel::DrawTexture(std::pair<const std::string, std::shared_ptr<VulkanTexture>>& texture)
	{
		ImGui::Text(texture.first.c_str());
		static char textureContent[256];
		strcpy_s(textureContent, AssetManager::Instance().GetName(texture.second->GetHandle()).c_str());
		if (ImGui::InputText(("##" + texture.first).c_str(), textureContent, sizeof(textureContent), ImGuiInputTextFlags_ReadOnly) || ImGui::IsItemHovered())
		{
			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("m_TextureResource"))
				{
					std::string assetName = (const char*)payload->Data;
					assetName = assetName.substr(assetName.find_last_of('\\') + 1);
					assetName = assetName.substr(0, assetName.find_last_of(".png") + 1);

					std::shared_ptr<VulkanTexture> droppedTexture;

					//Check if asset is already compiled
					//Compiled before
					if (AssetManager::Instance().Contains(assetName))
					{
						//Load into memory
						if (droppedTexture = AssetManager::Instance().GetAsset<VulkanTexture>(assetName); droppedTexture == nullptr)
						{
							AssetManager::Instance().AddAsset<VulkanTexture>(assetName);
							droppedTexture = AssetManager::Instance().GetAsset<VulkanTexture>(assetName);
						}
					}
					else
					{
						//Compile and load asset
						droppedTexture = AssetManager::Instance().CompileAndLoad<VulkanTexture>(assetName);
					}
					texture.second = droppedTexture;
				}

				ImGui::EndDragDropTarget();
			}
		}
	}
}