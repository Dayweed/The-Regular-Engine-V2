#include "pch.h"
#include "SceneHierarchyPanel.h"
#include "Core/GameLoop.h"
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
			if (ImGui::Button("Create Entity"))
			{
				Entity GameObject = ECSManager::Instance().CreateEntity();
				GameObject->GetComponent<Properties>().m_Name = "GameObject (" + std::to_string(ECSManager::Instance().GetEntities<Properties>().size()) + ")";
			}

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
			std::vector<Entity> entities{ ECSManager::Instance().GetAllEntities() };
			if (GameLoop::Instance().GetDisplayingPrefab())
			{
				entities.clear();
				entities.emplace_back(ECSSystemManager::Instance().GetSystem<PrefabSystem>()->GetDisplayedPrefab());
			}

			for (auto& currentEntity : entities)
			{
				ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow;
				const std::string& entityName = currentEntity->GetName();

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
		const std::string entityName = CurrentEntity->GetName();
		std::vector<TRE::Entity> childrenVector = ECSSystemManager::Instance().GetSystem<ParentingSystem>()->GetChildren(CurrentEntity);
		const int vectorSize = ECSSystemManager::Instance().GetSystem<ParentingSystem>()->GetChildren(CurrentEntity).size();

		//parent has children
		if (vectorSize)
		{
			ImGuiTreeNodeFlags Flags = ((m_SelectionManager->GetSelectedEntity() == CurrentEntity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;

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

				if (m_SelectionManager->GetSelectedEntity())
				{
					if (!m_SelectionManager->GetSelectedEntity()->HasComponent<Camera>())
					{
						//ImGui::OpenPopup("Popup_without_cam");
						//create and delete entity
						if (ImGui::BeginPopupContextWindow(/*"Popup_without_cam"*/))
						{
							if (ImGui::Selectable("Create Entity"))
							{
								Entity GameObject = ECSManager::Instance().CreateEntity();
								GameObject->GetComponent<Properties>().m_Name = "GameObject (" + std::to_string(ECSManager::Instance().GetEntities<Properties>().size()) + ")";
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

					else
					{
						//ImGui::OpenPopup("Popup");

						if (ImGui::BeginPopupContextWindow(/*"Popup"*/))
						{
							if (ImGui::Selectable("Create Entity"))
							{
								Entity GameObject = ECSManager::Instance().CreateEntity();
								GameObject->GetComponent<Properties>().m_Name = "GameObject (" + std::to_string(ECSManager::Instance().GetEntities<Properties>().size()) + ")";
							}
							ImGui::EndPopup();
						}
					}
				}

				for (auto& entityChild : childrenVector)
				{
					DisplayChildren(entityChild);
				}

				ImGui::TreePop();
			}
		}

		//its a leaf node
		else
		{
			ImGuiTreeNodeFlags Flags = ImGuiTreeNodeFlags_Leaf | ((m_SelectionManager->GetSelectedEntity() == CurrentEntity) ? ImGuiTreeNodeFlags_Selected : 0);
			
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

				if (m_SelectionManager->GetSelectedEntity())
				{
					if (!m_SelectionManager->GetSelectedEntity()->HasComponent<Camera>())
					{
						//ImGui::OpenPopup("Popup_without_cam");
						//create and delete entity
						if (ImGui::BeginPopupContextWindow(/*"Popup_without_cam"*/))
						{
							if (ImGui::Selectable("Create Entity"))
							{
								Entity GameObject = ECSManager::Instance().CreateEntity();
								GameObject->GetComponent<Properties>().m_Name = "GameObject (" + std::to_string(ECSManager::Instance().GetEntities<Properties>().size()) + ")";
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

					else
					{
						//ImGui::OpenPopup("Popup");

						if (ImGui::BeginPopupContextWindow(/*"Popup"*/))
						{
							if (ImGui::Selectable("Create Entity"))
							{
								Entity GameObject = ECSManager::Instance().CreateEntity();
								GameObject->GetComponent<Properties>().m_Name = "GameObject (" + std::to_string(ECSManager::Instance().GetEntities<Properties>().size()) + ")";
							}
							ImGui::EndPopup();
						}
					}
				}

				ImGui::TreePop();
			}
		}
	}

	void SceneHierarchyPanel::DeleteChildren(TRE::Entity& CurrentEntity)
	{
		const std::string entityName = CurrentEntity->GetName();
		std::vector<TRE::Entity> childrenVector = ECSSystemManager::Instance().GetSystem<ParentingSystem>()->GetChildren(CurrentEntity);
		const int vectorSize = ECSSystemManager::Instance().GetSystem<ParentingSystem>()->GetChildren(CurrentEntity).size();


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
