#include "pch.h"
#include "SceneHierarchyPanel.h"
#include "Core/GameLoop.h"
#include "Imgui/imgui.h"
#include "Core/Parent.h"

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

	}

	void SceneHierarchyPanel::Update()
	{
		ImGui::Begin("Hierarchy");

		bool displayingPrefab{ GameLoop::Instance().GetDisplayingPrefab() };
		std::string SceneDisplay = displayingPrefab ? "Prefab" : SceneManager::Instance().GetCurrentSceneName();

		// Display button to return to scene
		if (displayingPrefab && ImGui::Button("Return to Scene", ImVec2(-FLT_MIN, 0.0f)))
		{
			ECSSystemManager::Instance().GetSystem<PrefabSystem>()->ReturnToScene();
		}

		if (ImGui::TreeNodeEx(SceneDisplay.c_str(), ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_DefaultOpen))
		{
			//create new entities individually
			//if (ImGui::Button("Create Entity"))
			//{
			//	Entity GameObject = ECSManager::Instance().CreateEntity();
			//	GameObject->GetComponent<Properties>().m_Name = "GameObject (" + std::to_string(ECSManager::Instance().GetEntities<Properties>().size()) + ")";
			//}

			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Entity"))
				{
					TRE::Entity payload_n = *(const TRE::Entity*)payload->Data; //this will be child of currententity

					//if the child entity has parent
					if (ECSSystemManager::Instance().GetSystem<ParentingSystem>()->GetParent(payload_n))
					{
						ECSSystemManager::Instance().GetSystem<ParentingSystem>()->RemoveParent(payload_n);
					}
				}

				ImGui::EndDragDropTarget();
			}

			//theres no more drag and drop receive from content browser?
			//if (ImGui::BeginDragDropTarget())
			//{
			//	if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Content Browser item"))
			//	{
			//		Entity GameObject = ECSManager::Instance().CreateEntity();
			//		GameObject->GetComponent<Properties>().m_Name = (const char*)payload->Data;
			//	}

			//	ImGui::EndDragDropTarget();
			//}

			// Choose between getting all entities or just the prefab if it is displaying prefab
			std::vector<Entity> entities{ ECSManager::Instance().GetAllEntities(true) };
			if (GameLoop::Instance().GetDisplayingPrefab())
			{
				entities.clear();
				entities.emplace_back(ECSSystemManager::Instance().GetSystem<PrefabSystem>()->GetDisplayedPrefab());
			}

			if (ImGui::IsWindowHovered())
			{
				if (ImGui::IsMouseClicked(0) || ImGui::IsMouseClicked(1))
					ImGui::SetWindowFocus();
			}

			if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows))
			{
				if (m_SelectionManager->GetSelectedEntity())
				{
					if (!m_SelectionManager->GetSelectedEntity()->HasComponent<Camera>())
					{
						//create and delete entity
						if (ImGui::BeginPopupContextWindow())
						{
							//create new entities as entity's children
							if (ImGui::Selectable("Create Entity"))
							{
								Entity entityChild = ECSManager::Instance().CreateEntity();
								entityChild->GetComponent<Properties>().m_Name = "GameObject (" + std::to_string(entities.size()) + ")";
								ECSSystemManager::Instance().GetSystem<ParentingSystem>()->SetParent(entityChild, m_SelectionManager->GetSelectedEntity());
								m_SelectionManager->ClearSelectedEntity();
							}

							if (ImGui::Selectable("Delete Entity"))
							{
								TRE::Entity entityToDelete = m_SelectionManager->GetSelectedEntity();
								DeleteChildren(entityToDelete);
								m_SelectionManager->ClearSelectedEntity();
							}
							ImGui::EndPopup();
						}
					}
				}
				else
				{
					if (ImGui::BeginPopupContextWindow())
					{
						//create new entities as entity's children
						if (ImGui::Selectable("Create Entity"))
						{
							Entity entityChild = ECSManager::Instance().CreateEntity();
							entityChild->GetComponent<Properties>().m_Name = "GameObject (" + std::to_string(entities.size()) + ")";
							m_SelectionManager->SelectEntity(entityChild);
							//ECSSystemManager::Instance().GetSystem<ParentingSystem>()->SetParent(entityChild, m_SelectionManager->GetSelectedEntity());
							m_SelectionManager->ClearSelectedEntity();
						}
						ImGui::EndPopup();
					}
				}
			}

			for (auto& currentEntity : entities)
			{
				if (ECSSystemManager::Instance().GetSystem<ParentingSystem>()->GetParent(currentEntity) == nullptr)
				{
					DisplayChildren(currentEntity);
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
		const std::string entityName = CurrentEntity->GetName() + "##" + CurrentEntity->GetGUID();
		std::vector<TRE::Entity> childrenVector = ECSSystemManager::Instance().GetSystem<ParentingSystem>()->GetChildren(CurrentEntity);
		size_t vectorSize = ECSSystemManager::Instance().GetSystem<ParentingSystem>()->GetChildren(CurrentEntity).size();
		ImGuiTreeNodeFlags Flags = ((m_SelectionManager->GetSelectedEntity() == CurrentEntity) ? ImGuiTreeNodeFlags_Selected : 0) | (vectorSize ? ImGuiTreeNodeFlags_OpenOnArrow : ImGuiTreeNodeFlags_Leaf);
		 
		if (ImGui::TreeNodeEx(entityName.c_str(), Flags))
		{
			if (ImGui::BeginDragDropSource())
			{
				ImGui::SetDragDropPayload("Entity", &CurrentEntity, sizeof(TRE::Entity));
				ImGui::Text("Dragging %s", entityName.c_str());
				ImGui::EndDragDropSource();
			}

			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Entity"))
				{
					TRE::Entity payload_n = *(const TRE::Entity*)payload->Data; //this will be child of currententity
						
					ECSSystemManager::Instance().GetSystem<ParentingSystem>()->SetParent(payload_n, CurrentEntity);
				}

				ImGui::EndDragDropTarget();
			}

			if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
			{
				m_SelectionManager->SelectEntity(CurrentEntity);
			}

			if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
			{
				m_SelectionManager->SelectEntity(CurrentEntity);
			}

			for (auto& entityChild : childrenVector)
			{
				DisplayChildren(entityChild);
			}

			ImGui::TreePop();
		}
	}

	void SceneHierarchyPanel::DeleteChildren(TRE::Entity& CurrentEntity)
	{
		const std::string entityName = CurrentEntity->GetName();
		std::vector<TRE::Entity> childrenVector = ECSSystemManager::Instance().GetSystem<ParentingSystem>()->GetChildren(CurrentEntity);
		const size_t vectorSize = ECSSystemManager::Instance().GetSystem<ParentingSystem>()->GetChildren(CurrentEntity).size();


		if (vectorSize)
		{
			for (auto& entityChild : childrenVector)
			{
				DeleteChildren(entityChild);
			}

			ECSManager::Instance().MarkForDeletion(CurrentEntity);
		}

		else
		{
			ECSManager::Instance().MarkForDeletion(CurrentEntity);
		}
	}
}
