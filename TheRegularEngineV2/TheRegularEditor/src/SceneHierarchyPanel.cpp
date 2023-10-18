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

	}

	void SceneHierarchyPanel::Update()
	{
		ImGui::Begin("Hierarchy");

		if (ImGui::TreeNodeEx("Scene", ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_DefaultOpen))
		{
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

			//{
			//	//temp testing
			//	std::vector<TRE::Entity> toDisplay;

			//	for (size_t i{}; i < ECSManager::Instance().GetAllEntities().size(); ++i)
			//	{
			//		auto currentEntity = ECSManager::Instance().GetEntities<Properties>()[i];

			//		//get all parents only
			//		if (ECSSystemManager::Instance().GetSystem<ParentingSystem>()->GetParent(currentEntity) == nullptr)
			//		{
			//			toDisplay.push_back(currentEntity);
			//		}
			//	}

			//	for (size_t i{}; i < toDisplay.size(); ++i)
			//	{
			//		DisplayChildren(toDisplay[i]);
			//	}
			//}

			for (size_t i{}; i < ECSManager::Instance().GetAllEntities().size(); ++i)
			{
				ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow;
				auto currentEntity = ECSManager::Instance().GetEntities<Properties>()[i];
				const std::string& entityName = currentEntity->GetName();

				if (ECSSystemManager::Instance().GetSystem<ParentingSystem>()->GetParent(currentEntity) == nullptr)
				{
					DisplayChildren(currentEntity);

					//if (entityName != "cam")
					//{
					//	ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.f, 0.f, 0.f));
					//	if ((ImGui::Button("X") || ImGui::IsItemClicked()))
					//	{
					//		entityIDTodeleted = currentEntity->GetGUID();
					//	}
					//	ImGui::PopStyleColor(1);
					//	ImGui::SameLine();
					//}

					if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
					{
						ImGui::OpenPopup("Create_New_Entity");
						ImGui::Text("pressed with objects");
					}
				}
			}

			ImGui::TreePop();
		}

		if (entityIDTodeleted != "\0")
		{
			if (Entity ent = ECSManager::Instance().FindEntity(entityIDTodeleted))
			{
				//if (entityTobedeletedIsParent)
				//{
				//	std::cout << "is it here?\n";
					DeleteChildren(ent);
				//}

				//else
				//{
				//	ECSManager::Instance().DestroyEntity(ent);
				//}

				entityIDTodeleted = {};
				if (ent == m_SelectionManager->GetSelectedEntity())
				{
					m_SelectionManager->ClearSelectedEntity();
				}
			}
		}

		if (ImGui::BeginPopupContextWindow("Create_New_Entity"))
		{
			if (ImGui::Selectable("Create Entity"))
			{
				Entity GameObject = ECSManager::Instance().CreateEntity();
				GameObject->GetComponent<Properties>().m_Name = "GameObject (" + std::to_string(ECSManager::Instance().GetEntities<Properties>().size()) + ")";
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
		const std::string entityName = CurrentEntity->GetName();
		std::vector<TRE::Entity> childrenVector = ECSSystemManager::Instance().GetSystem<ParentingSystem>()->GetChildren(CurrentEntity);
		const int vectorSize = ECSSystemManager::Instance().GetSystem<ParentingSystem>()->GetChildren(CurrentEntity).size();

		//{
		//  //temp testing
		//	//there's children
		//	if (vectorSize)
		//	{
		//		ImGuiTreeNodeFlags flag = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanFullWidth;

		//		if (m_SelectionManager->GetSelectedEntity() == CurrentEntity)
		//		{
		//			flag |= ImGuiTreeNodeFlags_Selected;
		//		}

		//		if (ImGui::TreeNodeEx(entityName.c_str(), flag))
		//		{
		//			//drag and drop stuff

		//			if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
		//			{
		//				m_SelectionManager->SelectEntity(CurrentEntity);
		//			}

		//			for (size_t i{}; i < vectorSize; ++i)
		//			{
		//				TRE::Entity entityChild = ECSSystemManager::Instance().GetSystem<ParentingSystem>()->GetChildren(CurrentEntity)[i];
		//				DisplayChildren(entityChild);
		//			}

		//			ImGui::TreePop();
		//		}
		//	}

		//	else
		//	{
		//		ImGuiTreeNodeFlags flag = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_SpanFullWidth;

		//		if (m_SelectionManager->GetSelectedEntity() == CurrentEntity)
		//		{
		//			flag |= ImGuiTreeNodeFlags_Selected;
		//		}

		//		if (ImGui::TreeNodeEx(entityName.c_str(), flag))
		//		{
		//			//drag and drop stuff
		//			if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
		//			{
		//				m_SelectionManager->SelectEntity(CurrentEntity);
		//			}

		//			ImGui::TreePop();
		//		}
		//	}
		//}

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
						//std::cout << "parent: " << CurrentEntity->GetName() << "\n";
						//std::cout << "new child: " << payload_n->GetName() << std::endl << std::endl;
					}

					ImGui::EndDragDropTarget();
				}
				
				if (entityName != "cam")
				{
					ImGui::SameLine();
					ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.f, 0.f, 0.f));
					if ((ImGui::Button("X") || ImGui::IsItemClicked()))
					{
						entityIDTodeleted = CurrentEntity->GetGUID();
						entityTobedeletedIsParent = true;
					}
					ImGui::PopStyleColor(1);
				}

				if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
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
						//std::cout << "parent: " << CurrentEntity->GetName() << "\n";
						//std::cout << "new child: " << payload_n->GetName() << std::endl << std::endl;
					}

					ImGui::EndDragDropTarget();
				}
				
				if (entityName != "cam")
				{
					ImGui::SameLine();
					ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.f, 0.f, 0.f));
					if ((ImGui::Button("X") || ImGui::IsItemClicked()))
					{
						entityIDTodeleted = CurrentEntity->GetGUID();
						entityTobedeletedIsParent = false;
					}
					ImGui::PopStyleColor(1);
				}

				if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
				{
					m_SelectionManager->SelectEntity(CurrentEntity);
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
