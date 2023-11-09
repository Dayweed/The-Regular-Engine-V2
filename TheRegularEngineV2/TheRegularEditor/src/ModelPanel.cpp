#include "ModelPanel.h"
#include "TREIncludes.h"
#include "EventSystem/EventHandler/EventHandler.h"

namespace TRE
{
	ModelPanel::ModelPanel(const std::shared_ptr<AssetSelector>& assetSelector)
	{
		m_AssetSelector = assetSelector;
		EventHandler::getEventHandlerInstance().subscribe(this, &ModelPanel::OnAssetSelectEvent);
	}

	void ModelPanel::Init()
	{

	}

	void ModelPanel::Update()
	{
		if (m_MeshSelected)
		{
			ImGui::Begin("Model Panel");

			const std::string& assetName = m_AssetSelector->GetSelectedAssetName();
			std::string toPrint;
			//Check if compiled before
			if (AssetManager::Instance().Compiled(m_ResourceHandle))
			{
				toPrint = "Model " + assetName;
				ImGui::Text(toPrint.c_str());

				//Show descriptor texture file
				ImGui::Text("Scale"); ImGui::SameLine();
				ImGui::DragFloat3("##Scale", &m_Scale.x);

				ImGui::Text("Rotation"); ImGui::SameLine();
				ImGui::DragFloat3("##Rotation", &m_Rotation.x);

				ImGui::Text("Translation"); ImGui::SameLine();
				ImGui::DragFloat3("##Translation", &m_Translation.x);

				ImGui::Text("Optimize"); ImGui::SameLine();
				if (ImGui::Checkbox("##OptimizeModel", &m_Optimize))
				{

				}

				if (ImGui::Button("Recompile Model"))
				{
					GeomDescriptorFile descriptorFile;
					descriptorFile.ReadDescriptorFile(m_DescriptorFilePath);
					descriptorFile.SetScale(m_Scale);
					descriptorFile.SetRotation(m_Rotation);
					descriptorFile.SetPosition(m_Translation);
					descriptorFile.SetOptimize(m_Optimize);

					descriptorFile.GenerateDescriptorFile();

					auto newMesh = AssetManager::Instance().CompileAndLoad<RenderObject>(assetName);

					for (auto& ent : ECSManager::Instance().GetEntities<MeshRenderer>())
					{
						if (ent->GetComponent<MeshRenderer>().m_RenderObject->GetHandle() == newMesh->GetHandle())
						{
							ECSSystemManager::Instance().GetSystem<MeshRendererSystem>()->SetMeshRenderer(ent, newMesh);
						}
					}
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
				}
			}
			ImGui::End();
		}
	}

	void ModelPanel::Shutdown()
	{

	}

	void ModelPanel::OnAssetSelectEvent(const AssetSelectorEvent& event)
	{
		m_MeshSelected = false;
		m_ResourceHandle = 0;
		m_DescriptorFilePath = "";
		if (event.m_AssetType == AssetSelectorEvent::AssetType::Model)
		{
			m_MeshSelected = true;

			//Open descriptor file if compiled before
			if (const auto resourceHandle = m_AssetSelector->GetSelectedAsset();
				resourceHandle && AssetManager::Instance().Compiled(resourceHandle))
			{
				m_ResourceHandle = resourceHandle;

				//Open descriptor file
				m_DescriptorFilePath = "../Assets/" + Resource::GetGUIDHex(resourceHandle) + ".geom.desc";
				std::ifstream file(m_DescriptorFilePath);
				if (file.is_open())
				{
					GeomDescriptorFile descriptor;
					descriptor.ReadDescriptorFile(m_DescriptorFilePath);

					m_Scale = descriptor.GetScale();
					m_Rotation = descriptor.GetRotation();
					m_Translation = descriptor.GetPosition();
					m_Optimize = descriptor.GetOptimize();
				}
				else
				{
					std::cout << "Could not open descriptor file " << m_DescriptorFilePath << std::endl;
				}

				file.close();
			}
		}

	}

}