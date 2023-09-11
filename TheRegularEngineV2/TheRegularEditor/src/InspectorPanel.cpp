#include "pch.h"
#include "InspectorPanel.h"
#include "Imgui/imgui.h"
#include "TREIncludes.h"

namespace TRE
{
	InspectorPanel::InspectorPanel(const std::shared_ptr<SelectionManager>& Selection_Manager)
	{
		m_SelectionManager = Selection_Manager;
	}

	InspectorPanel::~InspectorPanel()
	{

	}

	void InspectorPanel::Init()
	{

	}

	void InspectorPanel::Update()
	{
		ImGui::Begin("Inspector");

		//if (m_SelectionManager->GetSelectedEntity() != nullptr)
		//std::cout << "current object: " << m_SelectionManager->GetSelectedEntity()->GetName() << "\n";
		//std::cout << "size of vector: " << ECSManager::Instance().GetEntities<Properties>().size() << "\n";
		//for (size_t i{}; i < ECSManager::Instance().GetEntities<Properties>().size(); ++i)
		//{
		//	std::cout << "whats the name: " << ECSManager::Instance().GetEntities<Properties>()[i]->GetComponent<Properties>().m_Name << "\n";
		//	
		//}
		//create entity

		//object name
		if (m_SelectionManager->GetSelectedEntity() != nullptr)
		{
			if (m_SelectionManager->GetSelectedEntity()->HasComponent<Properties>())
			{
				bool check = m_SelectionManager->GetSelectedEntity()->GetComponent<Properties>().m_Active;
				ImGui::Checkbox("Active", &check);
				if (check != m_SelectionManager->GetSelectedEntity()->GetComponent<Properties>().m_Active) 
				{
					m_SelectionManager->GetSelectedEntity()->GetComponent<Properties>().m_Active = check;
				}

				ImGui::SameLine();
				char objectName[128] = "";
				strcpy(objectName, m_SelectionManager->GetSelectedEntity()->GetComponent<Properties>().m_Name.c_str());
				ImGui::InputTextWithHint("##ObjectName", "Object name", objectName, IM_ARRAYSIZE(objectName));
				if (0 != strcmp(objectName, m_SelectionManager->GetSelectedEntity()->GetComponent<Properties>().m_Name.c_str()))
				{
					m_SelectionManager->GetSelectedEntity()->GetComponent<Properties>().m_Name = objectName;
				}
			}
		
			if (m_SelectionManager->GetSelectedEntity()->HasComponent<Transform>())
			{
				if (ImGui::TreeNodeEx("Transform", ImGuiTreeNodeFlags_DefaultOpen))
				{
					float pos[3] = { m_SelectionManager->GetSelectedEntity()->GetComponent<Transform>().m_Position.x, m_SelectionManager->GetSelectedEntity()->GetComponent<Transform>().m_Position.y, m_SelectionManager->GetSelectedEntity()->GetComponent<Transform>().m_Position.z };
					ImGui::DragFloat3("Position", pos);
					//std::cout << "x before: " << m_SelectionManager->GetSelectedEntity()->GetComponent<Transform>().m_Position.x << "\n";
					if (pos[0] != m_SelectionManager->GetSelectedEntity()->GetComponent<Transform>().m_Position.x || pos[1] != m_SelectionManager->GetSelectedEntity()->GetComponent<Transform>().m_Position.y || pos[2] != m_SelectionManager->GetSelectedEntity()->GetComponent<Transform>().m_Position.z)
					{
						m_SelectionManager->GetSelectedEntity()->GetComponent<Transform>().m_Scale.x = pos[0];
						m_SelectionManager->GetSelectedEntity()->GetComponent<Transform>().m_Scale.y = pos[1];
						m_SelectionManager->GetSelectedEntity()->GetComponent<Transform>().m_Scale.z = pos[2];
						//std::cout << "x after: " << m_SelectionManager->GetSelectedEntity()->GetComponent<Transform>().m_Position.x << "\n";
					}
					ImGui::TreePop();
				}
			}

			//for (size_t i{}; i < ECSManager::Instance().GetEntities<Properties>().size(); ++i)
			//{
			//	std::cout << "name: " << ECSManager::Instance().GetEntities<Properties>()[i]->GetComponent<Properties>().m_Name << "\n";
			//	if (ECSManager::Instance().GetEntities<Properties>()[i]->GetComponent<Properties>().m_Active)
			//	{
			//		std::cout << "active\n";
			//	}
			//	
			//	else
			//	{
			//		std::cout << "not active\n";
			//	}
			//		
			//}
			//m_SelectionManager->GetSelectedEntity()->HasComponent<MeshRenderer>();
			//m_SelectionManager->GetSelectedEntity()->HasComponent<Camera>();
			//m_SelectionManager->GetSelectedEntity()->HasComponent<SphereCollider>();
			//m_SelectionManager->GetSelectedEntity()->HasComponent<BoxCollider>();
			//m_SelectionManager->GetSelectedEntity()->HasComponent<Audio>();

		}


		////tag
		//ImGui::AlignTextToFramePadding();
		//ImGui::Text("Tag");
		//ImGui::SameLine();
		//const char* Tags[] = { "Untagged" };
		//static int TagsIndex = 0; // If the selection isn't within 0..count, Combo won't display a preview
		//ImGui::Combo("##Tag", &TagsIndex, Tags, IM_ARRAYSIZE(Tags));

		////layer
		////ImGui::SameLine();
		//ImGui::Text("Layer");
		//ImGui::SameLine();
		//const char* Layers[] = { "Default" };
		//static int LayersIndex = 0; // If the selection isn't within 0..count, Combo won't display a preview
		////ImGui::Combo("##Layer", &LayersIndex, Layers, IM_ARRAYSIZE(Layers));
		//if(ImGui::BeginCombo("##Layer", Layers[LayersIndex]))
		//{
		//	for (int n = 0; n < IM_ARRAYSIZE(Layers); n++)
		//	{
		//		bool is_selected = (LayersIndex == n);
		//		if (ImGui::Selectable(Layers[n], is_selected))
		//			LayersIndex = n;
		//		if (is_selected)
		//			ImGui::SetItemDefaultFocus();
		//	}
		//	ImGui::EndCombo();
		//}

		ImGui::End();
	}
	
	void InspectorPanel::Shutdown()
	{

	}
}