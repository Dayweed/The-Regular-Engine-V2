#include "TexturePanel.h"
#include "TREIncludes.h"
#include "EventSystem/EventHandler/EventHandler.h"

namespace TRE
{
	TexturePanel::TexturePanel(const std::shared_ptr<AssetSelector>& assetSelector)
	{
		m_AssetSelector = assetSelector;
		EventHandler::getEventHandlerInstance().subscribe(this, &TexturePanel::OnAssetSelectEvent);
	}

	void TexturePanel::Init()
	{

	}

	void TexturePanel::Update()
	{
		if (m_TextureSelected)
		{
			ImGui::Begin("Texture Panel");

			const std::string& assetName = m_AssetSelector->GetSelectedAssetName();
			std::string toPrint;

			//Check if compiled before
			if (AssetManager::Instance().Compiled(m_ResourceHandle))
			{
				toPrint = "Texture " + assetName;
				ImGui::Text(toPrint.c_str());

				//Show descriptor texture file
				ImGui::Text("Compress"); ImGui::SameLine();
				if (ImGui::Checkbox("##CompressTexture", &m_Compress))
				{

				}

				ImGui::Text("Linear"); ImGui::SameLine();
				if (ImGui::Checkbox("##LinearTexture", &m_Linear))
				{

				}

				ImGui::Text("BCn"); ImGui::SameLine();
				static bool one, three, five;
				switch (m_BCn)
				{
				case 1:
					one = true;
					three = false;
					five = false;
					break;
				case 3:
					one = false;
					three = true;
					five = false;
					break;
				case 5:
					one = false;
					three = false;
					five = true;
					break;
				}
				if (ImGui::Checkbox("1", &one))
				{
					m_BCn = 1;
				}
				ImGui::SameLine();
				if (ImGui::Checkbox("3", &three))
				{
					m_BCn = 3;
				}
				ImGui::SameLine();
				if (ImGui::Checkbox("5", &five))
				{
					m_BCn = 5;
				}

				ImGui::Text("sRGB"); ImGui::SameLine();
				if (ImGui::Checkbox("##sRGBTexture", &m_sRGB))
				{

				}

				ImGui::Text("Transparent"); ImGui::SameLine();
				if (ImGui::Checkbox("##TransparentTexture", &m_Transparent))
				{
				}

				if (ImGui::Button("Recompile Texture"))
				{
					TextureDescriptorFile descriptorFile;
					descriptorFile.ReadDescriptorFile(m_DescriptorFilePath);
					descriptorFile.SetCompress(m_Compress);
					descriptorFile.SetLinear(m_Linear);
					descriptorFile.SetBCn(m_BCn);
					descriptorFile.SetTransparent(m_Transparent);
					descriptorFile.SetsRGB(m_sRGB);
					descriptorFile.SetTransparent(m_Transparent);

					descriptorFile.GenerateDescriptorFile();

					auto newTexture = AssetManager::Instance().CompileAndLoad<VulkanTexture>(assetName);

					//Find all materials or sprite renderers that use this texture and update them
					for (auto& material : AssetManager::Instance().GetAssetsOfType<Material>())
					{
						std::string assignedTextureName;
						if (material->ContainsTexture(newTexture->GetHandle(), assignedTextureName))
						{
							material->SetTexture(assignedTextureName, newTexture);
						}
					}
				}

				if (ImGui::Button("Delete Texture"))
				{
					AssetManager::Instance().RemoveAsset(assetName);
					m_TextureSelected = false;
				}
			}
			else
			{
				toPrint = "Texture " + assetName + " not compiled yet";
				ImGui::Text(toPrint.c_str());
				if (ImGui::Button("Compile Texture"))
				{
					auto texture = AssetManager::Instance().CompileAndLoad<VulkanTexture>(assetName);
					m_AssetSelector->UpdateSelectedAssetHandle(texture->GetHandle());
					UpdateTexturePanel();
				}
			}
			ImGui::End();
		}
	}

	void TexturePanel::Shutdown()
	{

	}

	void TexturePanel::OnAssetSelectEvent(const AssetSelectorEvent& event)
	{
		m_TextureSelected = false;
		m_ResourceHandle = 0;
		m_DescriptorFilePath = "";
		if (event.m_AssetType == AssetSelectorEvent::AssetType::Texture)
		{
			m_TextureSelected = true;

			UpdateTexturePanel();
		}
	}

	void TexturePanel::UpdateTexturePanel()
	{
		//Open descriptor file if compiled before
		if (const auto resourceHandle = m_AssetSelector->GetSelectedAsset();
			resourceHandle && AssetManager::Instance().Compiled(resourceHandle))
		{
			m_ResourceHandle = resourceHandle;

			//Open descriptor file
			m_DescriptorFilePath = "../Assets/" + Resource::GetGUIDHex(resourceHandle) + ".texture.desc";
			std::ifstream file(m_DescriptorFilePath);
			if (file.is_open())
			{
				TextureDescriptorFile descriptor;
				descriptor.ReadDescriptorFile(m_DescriptorFilePath);

				m_Compress = descriptor.GetCompress();
				m_Linear = descriptor.GetLinear();
				m_BCn = descriptor.GetBCn();
				m_sRGB = descriptor.GetsRGB();
				m_Transparent = descriptor.GetTransparent();
			}
			else
			{
				std::cout << "Could not open descriptor file " << m_DescriptorFilePath << std::endl;
			}

			file.close();
		}
	}
}