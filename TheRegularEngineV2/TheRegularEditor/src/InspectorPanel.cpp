#include "pch.h"
#include "InspectorPanel.h"
#include "Imgui/imgui.h"
#include "cpp/imgui_stdlib.h"
#include "cpp/imgui_stdlib.cpp"
#include "TREIncludes.h"
#include "EditorAssetManager.h"

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

		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Content Browser item"))
			{
				std::string assetName = (const char*)payload->Data;
				std::cout << "drag and dropped " << assetName << " from Content Browser Panel\n";
			}

			ImGui::EndDragDropTarget();
		}

		//create entity
		auto entity = m_SelectionManager->GetSelectedEntity();

		//object name
		if (entity != nullptr)
		{
			/*if (entity->HasComponent<Properties>())
			{
				bool check = entity->GetComponent<Properties>().m_Active;
				ImGui::Checkbox("Active", &check);
				if (check != entity->GetComponent<Properties>().m_Active) 
				{
					entity->GetComponent<Properties>().m_Active = check;
				}

				ImGui::SameLine();
				char objectName[128] = "";
				strcpy(objectName, entity->GetComponent<Properties>().m_Name.c_str());
				ImGui::InputTextWithHint("##ObjectName", "Object name", objectName, IM_ARRAYSIZE(objectName));
				if (0 != strcmp(objectName, entity->GetComponent<Properties>().m_Name.c_str()))
				{
					m_SelectionManager->GetSelectedEntity()->GetComponent<Properties>().m_Name = objectName;
				}
			}*/
		
			/*if (entity->HasComponent<Transform>())
			{
				if (ImGui::TreeNodeEx("Transform", ImGuiTreeNodeFlags_DefaultOpen))
				{
					auto transformSystem = ECSSystemManager::Instance().GetSystem<TransformSystem>();
					const glm::vec3& tempPos = transformSystem->GetPosition(entity);
					float pos[3] = { tempPos.x, tempPos.y, tempPos.z };
					
					ImGui::DragFloat3("Position", pos);
					if (pos[0] != tempPos.x || pos[1] != tempPos.y || pos[2] != tempPos.z)
					{
						transformSystem->SetPosition(entity, { pos[0], pos[1], pos[2]});
					}
					ImGui::TreePop();
				}
			}*/

			// This updates the tables
			m_SelectionManager->SelectEntity(entity);

			auto& properties = m_SelectionManager->GetSelectedEntityProperty();

			bool isPrefabInstance = false;

			// Show option of prefabing possibility if have prefab
			if (ImGui::Button("Save As Prefab"))
			{
				ECSSystemManager::Instance().GetSystem<PrefabSystem>()->SavePrefabEntity(entity);
			}
			if (entity->HasComponent<Prefabing>())
			{
				// Is Prefab Instance
				isPrefabInstance = true;
				ImGui::SameLine();
				if (ImGui::Button("Overwrite Prefab"))
				{
					ECSSystemManager::Instance().GetSystem<PrefabSystem>()->SavePrefabEntity(entity, false);
				}
				Prefabing& pref{ entity->GetComponent<Prefabing>() };
				ImGui::SameLine();
				if (ImGui::Button("Clone Prefab"))
				{
					ECSSystemManager::Instance().GetSystem<PrefabSystem>()->CreatePrefabEntityInstance(pref.m_PrefabGUID);
				}
				ImGui::SameLine();
				if (ImGui::Button("UnPrefab"))
				{
					entity->RemoveComponent<Prefabing>();
					isPrefabInstance = false;
				}
				bool haveEdits{ !pref.m_Overrides.empty() || !pref.m_AddeddComps.empty() || !pref.m_RemovedComps.empty() };
				ImGui::SameLine();
				if (haveEdits && ImGui::Button("Revert"))
				{
					ECSSystemManager::Instance().GetSystem<PrefabSystem>()->RevertInstance(entity, pref.m_PrefabGUID);
					m_SelectionManager->SelectEntity(entity);
				}
				else if (!haveEdits)
				{
					ImGui::NewLine();
				}
			}

			// View all inspectable components
			for (auto& List : properties)
			{
				//std::cout << "properties: " << List.first.c_str() << "\n";
				for (size_t c{}; c < List.first.size(); ++c)
				{
					std::string charac { List.first[c]  };
					float diff{ static_cast<float>(c) / static_cast<float>(List.first.size()) };
					ImGui::TextColored({ 1, diff, 0, 1 }, charac.c_str());
					ImGui::SameLine();
				}

				if (ECSManager::Instance().IsRemovableComponent(List.first))
				{
					if (ImGui::Button("Remove Component", ImVec2(-FLT_MIN, 0.0f)) || ImGui::IsItemClicked())
					{
						// Update Prefabing if have
						if (isPrefabInstance)
						{
							entity->GetComponent<Prefabing>().m_RemovedComps.emplace(List.first);
						}

						ECSManager::Instance().RemCompFromName(entity, List.first);
						m_SelectionManager->SelectEntity(entity);
						break;
					}
				}

				ImGui::NewLine();
				for (auto& [Name, Data] : List.second)
				{
					bool UpdatedData = false;

					std::string NameField = "##" + entity->GetGUID() + "/" + Name;
					std::string NameStr = Name.substr(Name.find_last_of("/") + 1);

					bool isEdited = false;
					if (isPrefabInstance)
					{
						Prefabing& prefab{ entity->GetComponent<Prefabing>() };
						// Is override
						if (prefab.m_Overrides.find(List.first) != prefab.m_Overrides.end())
						{
							std::unordered_set<std::string> vecStr{ prefab.m_Overrides.find(List.first)->second };
							if (std::find(vecStr.begin(), vecStr.end(), Name) != vecStr.end())
							{
								isEdited = true;
							}
						}
						// Is an Added Component
						else if (std::find(prefab.m_AddeddComps.begin(), prefab.m_AddeddComps.end(), List.first) != prefab.m_AddeddComps.end())
						{
							isEdited = true;
						}
					}
					if (isEdited)
					{
						ImGui::TextColored({ 0.5, 0.5, 1, 1 }, NameStr.c_str());
					}
					else
					{
						ImGui::Text(NameStr.c_str());
					}
					ImGui::SameLine();

					std::visit([&](auto&& Value)
						{
							using T = std::decay_t<decltype(Value)>;

							if constexpr (std::is_same_v<T, int>)
							{
								UpdatedData = UpdatedData ? true : ImGui::InputInt(NameField.c_str(), &Value);
							}
							else if constexpr (std::is_same_v<T, float>)
							{
								ImGui::InputFloat(NameStr.c_str(), &Value);
								//std::cout << "name: " << NameStr << " value: " << Value << "\n";
							}
							else if constexpr (std::is_same_v<T, bool>)
							{
								UpdatedData = UpdatedData ? true : ImGui::Checkbox(NameField.c_str(), &Value);
							}
							else if constexpr (std::is_same_v<T, string_t>)
							{
								UpdatedData = UpdatedData ? true : ImGui::InputText(NameField.c_str(), &Value);
							}
							else if constexpr (std::is_same_v<T, oobb>)
							{
								// Fake example of using structs (Should remove b4 m2!)...
								//printf("\t oobb   (%f, %f)", Value.m_Min, Value.m_Max);
							}
							else if constexpr (std::is_same_v<T, glm::vec3>)
							{
								float pos[3]{ Value.x, Value.y, Value.z };
								UpdatedData = UpdatedData ? true : ImGui::DragFloat3(NameField.c_str(), pos);
								Value = { pos[0], pos[1], pos[2] };
							}
							else if constexpr (std::is_same_v<T, respurce_ref>)
							{
								static char renderObject[200];
								strcpy(renderObject, EditorAssetManager::Instance().GetName(Value.m_Vale).c_str());
								ImGui::InputText("##", renderObject, sizeof(renderObject));
			
							}							
							else static_assert(always_false<T>::value, "We are not covering all the cases!");
						}
					, Data);


					// Update Prefabing Instance data if have
					if (UpdatedData && isPrefabInstance)
					{
						Prefabing& prefab{ entity->GetComponent<Prefabing>() };
						// Ignore overriding if it is a newly added component
						if (prefab.m_AddeddComps.find(List.first) == prefab.m_AddeddComps.end())
						{
							// See if can emplace back
							auto it{ prefab.m_Overrides.find(List.first) };
							if (it == prefab.m_Overrides.end())
							{
								prefab.m_Overrides.emplace(std::piecewise_construct, std::forward_as_tuple(List.first), std::forward_as_tuple());
							}
							prefab.m_Overrides[List.first].emplace(Name);
						}
					}
				}
				ImGui::Separator();
			}

			// Update values into the entity itself
			auto components = m_SelectionManager->GetSelectedEntityComponents();

			for (size_t i{}; i < properties.size(); ++i)
			{
				property::base& compProp { *components[i].second };
				std::vector<property::entry> List{ properties[i].second };
				for (const auto& [Name, Data] : List)
				{
					// Copy to compProp
					property::set(compProp, Name.c_str(), Data);
				}
			}

			// Add additional components
			if (ImGui::Button("Add Component", ImVec2(-FLT_MIN, 0.0f)))
			{
				ImGui::OpenPopup("AddComponent");
			}

			if (ImGui::BeginPopup("AddComponent"))
			{
				for (std::string& compName : ECSManager::Instance().GetAllNonAddedComponents(entity))
				{
					if (ImGui::Selectable(compName.c_str()))
					{
						// Update Prefabing if have
						if (isPrefabInstance)
						{
							entity->GetComponent<Prefabing>().m_AddeddComps.emplace(compName);

							// Remove from m_RemovedComps to let it stay
							if (entity->GetComponent<Prefabing>().m_RemovedComps.find(compName) != entity->GetComponent<Prefabing>().m_RemovedComps.end())
							{
								entity->GetComponent<Prefabing>().m_RemovedComps.erase(compName);
							}
						}

						ECSManager::Instance().AddCompFromName(entity, compName);
						m_SelectionManager->SelectEntity(entity);
					}
				}

				ImGui::EndPopup();
			}
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