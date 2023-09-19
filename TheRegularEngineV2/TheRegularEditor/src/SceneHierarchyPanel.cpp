#include "pch.h"
#include "SceneHierarchyPanel.h"
#include "Imgui/imgui.h"

namespace TRE
{
	SceneHierarchyPanel::SceneHierarchyPanel(const std::shared_ptr<SelectionManager>& Selection_Manager)
	{
		m_SelectionManager = Selection_Manager;
	}

	SceneHierarchyPanel::~SceneHierarchyPanel()
	{

	}

	void SceneHierarchyPanel::Init()
	{
		m_SelectionContext = nullptr;
	}

	void SceneHierarchyPanel::Update()
	{
		ImGui::Begin("Hierarchy");

		int deleteEntity = -1;
		//std::cout << "size of vector: " << ECSManager::Instance().GetEntities<Properties>().size() << "\n";
		//for (size_t i{}; i < ECSManager::Instance().GetEntities<Properties>().size(); ++i)
		//{
		//    std::cout << i << "\twhats the name: " << currentEntity->GetComponent<Properties>().m_Name << "\n";
		//    
		//    std::cout << "do i have children?\t" << currentEntity->GetChildren().size() << "\n";
		//}

		if (ImGui::TreeNodeEx("Scene", ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_DefaultOpen))
		{
			static int selection_mask = 0; //nothing is selected in the beginning
			int object_clicked = -1; //none of the objects are selected

			for (size_t i{}; i < ECSManager::Instance().GetEntities<Properties>().size(); ++i)
			{
				ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow;
				auto currentEntity = ECSManager::Instance().GetEntities<Properties>()[i];
				std::string entityName = currentEntity->GetName();

				if (ECSSystemManager::Instance().GetSystem<ParentingSystem>()->GetParent(currentEntity) == nullptr)
				{
					//entities without children and parent
					if (!ECSSystemManager::Instance().GetSystem<ParentingSystem>()->GetChildren(currentEntity).size())
					{
						ImGuiTreeNodeFlags node_flag = ((m_SelectionContext == currentEntity) ? ImGuiTreeNodeFlags_Selected : 0) | node_flags | ImGuiTreeNodeFlags_Leaf;

						if (entityName != "cam")
						{
							ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.f, 0.f, 0.f));
							if ((ImGui::Button("X") || ImGui::IsItemClicked()))
							{
								deleteEntity = i;
							}
							ImGui::PopStyleColor(1);
							ImGui::SameLine();
						}

						if (ImGui::TreeNodeEx(entityName.c_str(), node_flag))
						{
							if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
							{
								m_SelectionManager->SelectEntity(ECSManager::Instance().GetEntities<Properties>()[i]);
								m_SelectionContext = currentEntity;
							}

							if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
							{
								ImGui::OpenPopup("Create_New_Entity");
								ImGui::Text("pressed with objects");
							}

							ImGui::TreePop();
						}
					}
					
					else
					{
						//entities with children
						DisplayChildren(currentEntity);
					}
				}
			}

			ImGui::TreePop();
		}

		if (deleteEntity > -1)
		{
			Entity ent{ ECSManager::Instance().GetEntities<Properties>()[deleteEntity] };
			ECSManager::Instance().DestroyEntity(ent);
			deleteEntity = -1;
			if (ent == m_SelectionManager->GetSelectedEntity())
			{
				m_SelectionManager->ClearSelectedEntity();
			}
		}

		if (ImGui::BeginPopupContextWindow("Create_New_Entity"))
		{
			if (ImGui::Selectable("Create Entity"))
			{
				Entity GameObject = ECSManager::Instance().CreateEntity();
				GameObject->GetComponent<Properties>().m_Name = "GameObject";
			}
			ImGui::EndPopup();
		}

		ImGui::End();
	}

	void SceneHierarchyPanel::Shutdown()
	{

	}

	void SceneHierarchyPanel::DisplayChildren(TRE::Entity& CurrentEntity)
	{
		std::string entityName = CurrentEntity->GetName();
		
		//parent has children
		if (ECSSystemManager::Instance().GetSystem<ParentingSystem>()->GetChildren(CurrentEntity).size())
		{
			ImGuiTreeNodeFlags Flags = ((m_SelectionContext == CurrentEntity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;

			if (ImGui::TreeNodeEx(entityName.c_str(), Flags))
			{
				if (ImGui::IsItemClicked())
				{
					m_SelectionManager->SelectEntity(CurrentEntity);
					m_SelectionContext = CurrentEntity;
				}

				for (size_t i{}; i < ECSSystemManager::Instance().GetSystem<ParentingSystem>()->GetChildren(CurrentEntity).size(); ++i)
				{
					DisplayChildren(ECSSystemManager::Instance().GetSystem<ParentingSystem>()->GetChildren(CurrentEntity)[i]);
				}

				ImGui::TreePop();
			}
		}

		//its a leaf node
		else
		{
			ImGuiTreeNodeFlags Flags = ((m_SelectionContext == CurrentEntity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
			ImGui::TreeNodeEx(entityName.c_str(), Flags | ImGuiTreeNodeFlags_Leaf);
			if (ImGui::IsItemClicked())
			{
				m_SelectionManager->SelectEntity(CurrentEntity);
				m_SelectionContext = CurrentEntity;
			}

			ImGui::TreePop();
		}
	}

	Entity& SceneHierarchyPanel::GetSelectionContext()
	{
		return m_SelectionContext;
	}
}