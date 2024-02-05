#include "pch.h"
#include "SceneHierarchyPanel.h"
#include "Core/GameLoop.h"
#include "Imgui/imgui.h"
#include "Core/Parent.h"
#include "EditorSystem.h"

namespace TRE
{
	static bool hasInput{};

	SceneHierarchyPanel::SceneHierarchyPanel(const std::shared_ptr<SelectionManager>& Selection_Manager)
	{
		m_SelectionManager = Selection_Manager;
	}

	SceneHierarchyPanel::~SceneHierarchyPanel()
	{

	}

	void SceneHierarchyPanel::Init()
	{
		EventHandler::getEventHandlerInstance().subscribe(this, &SceneHierarchyPanel::HandleShortcuts);
	}

	void SceneHierarchyPanel::Update()
	{
		ImGui::Begin("Hierarchy");
		static char inputTextBuffer[128] = "";

		if (ImGui::BeginChild("Search Bar", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y * 0.029f), false, ImGuiWindowFlags_NoScrollbar))
		{
			ImGui::InputText("##input", inputTextBuffer, sizeof(inputTextBuffer));
		}
		ImGui::EndChild();

		if (ImGui::BeginChild("Show entities", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y)))
		{
			bool displayingPrefab{ GameLoop::Instance().GetDisplayingPrefab() };
			std::string SceneDisplay = displayingPrefab ? "Prefab" : SceneManager::Instance().GetCurrentSceneName();

			// Display button to return to scene
			if (displayingPrefab && ImGui::Button("Return to Scene", ImVec2(-FLT_MIN, 0.0f)))
			{
				m_SelectionManager->ClearSelectedEntity();
				ECSSystemManager::Instance().GetSystem<PrefabSystem>()->ReturnToScene();
			}


			if (inputTextBuffer[0] == '\0')
			{
				hasInput = false;
			}

			else
			{
				hasInput = true;
			}

			if (ImGui::TreeNodeEx(SceneDisplay.c_str(), ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_DefaultOpen))
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

				// Choose between getting all entities or just the prefab if it is displaying prefab
				std::vector<Entity> entities{ ECSManager::Instance().GetAllEntities(true) };
				if (GameLoop::Instance().GetDisplayingPrefab())
				{
					entities.clear();
					entities.emplace_back(ECSSystemManager::Instance().GetSystem<PrefabSystem>()->GetDisplayedPrefab());
				}

				if (ImGui::IsWindowHovered())
				{
					if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) || ImGui::IsMouseClicked(ImGuiMouseButton_Right))
						ImGui::SetWindowFocus();

					if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) || ImGui::IsMouseClicked(ImGuiMouseButton_Right))
						m_SelectionManager->ClearSelectedEntity();
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
									m_SelectionManager->SelectEntity(entityChild);
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
							}
							ImGui::EndPopup();
						}
					}
				}

				std::vector<TRE::Entity> filteredEntities{};
				//search bar
				for (size_t i{}; i < entities.size(); ++i)
				{
					std::string fileNameLower = entities[i]->GetName();

					std::transform(fileNameLower.begin(), fileNameLower.end(), fileNameLower.begin(), [](unsigned char c) {return std::tolower(c); });
					size_t found = fileNameLower.find(inputTextBuffer);

					if (found != std::string::npos)
					{
						//add parent
						AddParent(entities[i], filteredEntities);
						//filteredEntities.push_back(entities[i]);
					}
				}

				for (auto& currentEntity : filteredEntities)
				{
					if (ECSSystemManager::Instance().GetSystem<ParentingSystem>()->GetParent(currentEntity) == nullptr)
					{
						DisplayChildren(currentEntity);
					}
				}

				ImGui::TreePop();
			}

			Entity SelectedEntity{ EditorSystemManager::Instance().GetSystem<EditorSystem>()->GetSelectionManager()->GetSelectedEntity() };
			if (ImGui::IsWindowHovered() && m_ShortcutCopyEntity)
			{
				EntityCopier::Instance().CopyEntities(SelectedEntity);
			}
			if (ImGui::IsWindowHovered() && m_ShortcutPasteEntity)
			{
				Entity pastedEntity = EntityCopier::Instance().PasteEntities();
				m_SelectionManager->SelectEntity(pastedEntity);
			}
			if (ImGui::IsWindowHovered() && m_ShortcutDuplicateEntity && SelectedEntity)
			{
				EntityCopier::Instance().CopyEntities(SelectedEntity);
				Entity pastedEntity = EntityCopier::Instance().PasteEntities();
				m_SelectionManager->SelectEntity(pastedEntity);
			}
			if (ImGui::IsWindowHovered() && m_ShortcutDeleteEntity && SelectedEntity)
			{
				ECSManager::Instance().MarkForDeletion(SelectedEntity);
				EditorSystemManager::Instance().GetSystem<EditorSystem>()->GetSelectionManager()->ClearSelectedEntity();
			}

			m_ShortcutCopyEntity = false;
			m_ShortcutPasteEntity = false;
			m_ShortcutDuplicateEntity = false;
			m_ShortcutDeleteEntity = false;
		}
		ImGui::EndChild();

		ImGui::End();
	}

	void SceneHierarchyPanel::Shutdown()
	{

	}

	void SceneHierarchyPanel::DisplayChildren(TRE::Entity& CurrentEntity)
	{
		const std::string entityName = CurrentEntity->GetName() + "##" + CurrentEntity->GetGUID();
		std::vector<TRE::Entity> childrenVector = ECSSystemManager::Instance().GetSystem<ParentingSystem>()->GetChildren(CurrentEntity);
		ImGuiTreeNodeFlags Flags = ((m_SelectionManager->GetSelectedEntity() == CurrentEntity) ? ImGuiTreeNodeFlags_Selected : 0) 
									| (!childrenVector.empty() ? ImGuiTreeNodeFlags_OpenOnArrow : ImGuiTreeNodeFlags_Leaf)
									| (hasInput ? ImGuiTreeNodeFlags_DefaultOpen : 0);

		ImVec4 color = CurrentEntity->HasComponent<Prefabing>() ? ImVec4( 0, 1, 1, 1 ) : ImVec4(1, 1, 1, 1);
		ImGui::PushStyleColor(0, color);

		// Auto Open this node if it added a child in the previous frame
		if (temp_ParentOpen == entityName)
		{
			ImGui::TreeNodeSetOpen(ImGui::GetCurrentWindow()->GetID(temp_ParentOpen.c_str()), true);
			temp_ParentOpen = "";
		}

		if (ImGui::TreeNodeEx(entityName.c_str(), Flags))
		{
			if (ImGui::BeginDragDropSource())
			{
				ImGui::SetDragDropPayload("Entity", &CurrentEntity, sizeof(TRE::Entity));
				ImGui::Text("Dragging %s", entityName.c_str());
				ImGui::EndDragDropSource();
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
		else if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
		{
			m_SelectionManager->SelectEntity(CurrentEntity);
		}

		// Drag and Drop Parent
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Entity"))
			{
				TRE::Entity payload_n = *static_cast<const TRE::Entity*>(payload->Data); //this will be child of currententity

				// Set parent
				ECSSystemManager::Instance().GetSystem<ParentingSystem>()->SetParent(payload_n, CurrentEntity);
				temp_ParentOpen = entityName;
			}

			ImGui::EndDragDropTarget();
		}

		ImGui::PopStyleColor();
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

	void SceneHierarchyPanel::HandleShortcuts(TypingEvent& event)
	{
		const KeyButton key = static_cast<KeyButton>(event.m_Key);
		const KeyMods mods = static_cast<KeyMods>(event.m_Mod);

		if (mods == KeyMods::CONTROL || mods == KeyMods::NUMLOCK_CONTROL)
		{
			m_ShortcutCopyEntity = key == KeyButton::C;
			m_ShortcutPasteEntity = key == KeyButton::V;
			m_ShortcutDuplicateEntity = key == KeyButton::D;
		}
		m_ShortcutDeleteEntity = key == KeyButton::Delete;
	}

	void SceneHierarchyPanel::AddParent(TRE::Entity& CurrentEntity, std::vector<TRE::Entity>& vec)
	{
		TRE::Entity childrenVector = ECSSystemManager::Instance().GetSystem<ParentingSystem>()->GetParent(CurrentEntity);
		
		//if no child
		if (childrenVector == nullptr)
		{
			//remove duplicates
			auto it = std::find(vec.begin(), vec.end(), CurrentEntity);

			if (it == std::end(vec))
			{
				vec.push_back(CurrentEntity);
			}
		}

		else
		{
			AddParent(childrenVector, vec);
		}
	}
}
