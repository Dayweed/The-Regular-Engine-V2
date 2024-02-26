#include "pch.h"
#include "MaterialPanel.h"
#include "EditorAssetManager.h"
#include "ContentBrowserPanel.h"
#include "EventSystem/EventHandler/EventHandler.h"

namespace TRE
{
	MaterialPanel::MaterialPanel(const std::shared_ptr<SelectionManager>& selectionManager, const std::shared_ptr<AssetSelector>& assetSelector)
	{
		m_SelectionManager = selectionManager;
		m_AssetSelector = assetSelector;
	}

	MaterialPanel::~MaterialPanel()
	{

	}

	void MaterialPanel::Init()
	{
		EventHandler::getEventHandlerInstance().subscribe(this, &MaterialPanel::OnKeyboardClick);
	}

	void MaterialPanel::Update()
	{
		//For selection of material from content browser
		if (const auto resourceHandle = m_AssetSelector->GetSelectedAsset(); resourceHandle)
		{
			if (std::shared_ptr<Material> material = ResourceManager::Instance().GetResource<Material>(resourceHandle); material)
			{
				ImGui::Begin("Material", nullptr, ImGuiWindowFlags_NoCollapse);

				InternalContent(material);
				//Theres a chance that this is not attached to any entity, so dont update the UBO

				ImGui::End();
			}
		}
		else if (auto& entity = m_SelectionManager->GetSelectedEntity(); entity)
		{
			if (entity->HasComponent<MeshRenderer>() == false)
				return;

			ImGui::Begin("Material", nullptr, ImGuiWindowFlags_NoCollapse);

			if (std::shared_ptr<Material> material = entity->GetComponent<MeshRenderer>().m_MaterialInstance; material)
			{
				InternalContent(material);
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
					assetName.erase(assetName.find(".png")); 	// This is to remove unneeded data at the end after ".fbx"
					assetName += ".png";

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
		ImGui::SameLine();
		if (ImGui::Button(("X##" + texture.first).c_str()))
		{
			texture.second = ResourceManager::Instance().GetResource<VulkanTexture>(VulkanTexture::GetDefaultTextureID());
		}
	}
	
	void MaterialPanel::Rename(std::shared_ptr<Material> material)
	{
		static char materialName[256];
		std::string tempName = AssetManager::Instance().GetName(material->GetHandle());
		tempName = tempName.substr(0, m_Name.find_last_of('.'));
		strcpy_s(materialName, tempName.c_str());
		
		ImGui::Text("Material Name");
		if (ImGui::InputText("##MaterialName", materialName, sizeof(materialName)))
		{
			//Rename asset
			if (m_EnterPressed)
			{
				m_Name = materialName;
				m_Name += ".material";
				AssetManager::Instance().RenameAsset(material->GetHandle(), m_Name);
				m_AssetSelector->SelectAsset(m_Name, AssetSelectorEvent::AssetType::Material);
			}
		}
		else
			m_EnterPressed = false;
	}

	void MaterialPanel::OnKeyboardClick(const InputEvent& event)
	{
		if (event._key == (int)KeyButton::Enter)
		{
			m_EnterPressed = true;
		}
	}

	void MaterialPanel::InternalContent(std::shared_ptr<Material> material)
	{
		Rename(material);

		for (auto& texture : material->GetTexturesRef())
		{
			DrawTexture(texture);
		}

		ImGui::Text("Color");
		auto& materialData = material->GetMaterialUBO();
		float data[4]{ materialData.m_Color.x, materialData.m_Color.y, materialData.m_Color.z, materialData.m_Color.w };
		ImGui::ColorEdit4(("##" + material->GetHandleHex()).c_str(), data);
		materialData.m_Color = { data[0], data[1], data[2], data[3] };

		if (ImGui::Button("Save"))
		{
			ResourceManager::Instance().SerializeResource<Material>(material->GetHandle());
		}

		if (ImGui::Button("Delete Material"))
		{
			AssetManager::Instance().RemoveAsset(m_Name);
		}

		if (material->IsValid() == false)
			material->Invalidate();
		material->SetMaterialUBO();
	}
}