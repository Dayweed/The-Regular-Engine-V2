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

		//std::cout << "size of vector: " << ECSManager::Instance().GetEntities<Properties>().size() << "\n";
		//for (size_t i{}; i < ECSManager::Instance().GetEntities<Properties>().size(); ++i)
		//{
		//    std::cout << i << "\twhats the name: " << ECSManager::Instance().GetEntities<Properties>()[i]->GetComponent<Properties>().m_Name << "\n";
		//    
		//    std::cout << "do i have children?\t" << ECSManager::Instance().GetEntities<Properties>()[i]->GetChildren().size() << "\n";
		//}

		if (ImGui::TreeNodeEx("Scene", ImGuiTreeNodeFlags_OpenOnArrow))
		{
			static int selection_mask = 0; //nothing is selected in the beginning
			int object_clicked = -1; //none of the objects are selected

			for (size_t i{}; i < ECSManager::Instance().GetEntities<Properties>().size(); ++i)
			{
				ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow;

				if (ECSManager::Instance().GetEntities<Properties>()[i]->GetParent() == nullptr)
				{
					//entities without children and parent
					if (!ECSManager::Instance().GetEntities<Properties>()[i]->GetChildren().size())
					{
						ImGuiTreeNodeFlags node_flag = ((m_SelectionContext == ECSManager::Instance().GetEntities<Properties>()[i]) ? ImGuiTreeNodeFlags_Selected : 0) | node_flags | ImGuiTreeNodeFlags_Leaf;
					
						if (ImGui::TreeNodeEx(ECSManager::Instance().GetEntities<Properties>()[i]->GetName().c_str(), node_flag))
						{
							if (ImGui::IsItemClicked())
							{
								m_SelectionManager->SelectEntity(ECSManager::Instance().GetEntities<Properties>()[i]);
								m_SelectionContext = ECSManager::Instance().GetEntities<Properties>()[i];
							}

							ImGui::TreePop();
						}
					}
					
					else
					{
						//entities with children
						DisplayChildren(ECSManager::Instance().GetEntities<Properties>()[i]);
					}
				}
			}

			ImGui::TreePop();
		}

		ImGui::End();
	}

	void SceneHierarchyPanel::Shutdown()
	{

	}

	void SceneHierarchyPanel::DisplayChildren(TRE::Entity& CurrentEntity)
	{
		//parent has children
		if (CurrentEntity->GetChildren().size())
		{
			ImGuiTreeNodeFlags Flags = ((m_SelectionContext == CurrentEntity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
			if (ImGui::TreeNodeEx(CurrentEntity->GetName().c_str(), Flags))
			{
				if (ImGui::IsItemClicked())
				{
					m_SelectionManager->SelectEntity(CurrentEntity);
					m_SelectionContext = CurrentEntity;
				}

				for (size_t i{}; i < CurrentEntity->GetChildren().size(); ++i)
				{
					DisplayChildren(CurrentEntity->GetChildren()[i]);
				}

				ImGui::TreePop();
			}
		}

		//its a leaf node
		else
		{
			ImGuiTreeNodeFlags Flags = ((m_SelectionContext == CurrentEntity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
			ImGui::TreeNodeEx(CurrentEntity->GetName().c_str(), Flags | ImGuiTreeNodeFlags_Leaf);
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