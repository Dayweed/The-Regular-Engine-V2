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
				const auto& textures = material->GetTextures();
				ImGui::Text("Diffuse");
				static char diffuseTexture[256];
				strcpy_s(diffuseTexture, AssetManager::Instance().GetName(textures[0]->GetHandle()).c_str());
				if (ImGui::InputText("##Diffuse", diffuseTexture, sizeof(diffuseTexture), ImGuiInputTextFlags_ReadOnly) || ImGui::IsItemHovered())
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
							material->SetTexture(0, droppedTexture);
						}

						ImGui::EndDragDropTarget();
					}
				}

				//ImGui::ColorEdit4("albedo", glm::value_ptr(meshRenderer.m_MaterialInstance->m_Albedo));
				ImGui::Text("metallic");
				//ImGui::SliderFloat("metallic", &meshRenderer.m_MaterialInstance->m_Metallic, 0.0f, 1.0f);
				ImGui::Text("roughness");
				//ImGui::SliderFloat("roughness", &meshRenderer.m_MaterialInstance->m_Roughness, 0.0f, 1.0f);
				ImGui::Text("ao");
				//ImGui::SliderFloat("ao", &meshRenderer.m_MaterialInstance->m_AO, 0.0f, 1.0f);
				ImGui::Text("Normal");

				
			}

			ImGui::End();
		}		
	}

	void MaterialPanel::Shutdown()
	{

	}
}