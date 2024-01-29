#include "pch.h"
#include "InspectorPanel.h"
#include "Imgui/imgui.h"
#include "cpp/imgui_stdlib.h"
#include "cpp/imgui_stdlib.cpp"
#include "TREIncludes.h"
#include "Scripting/ScriptEngine.h"
#include "EditorAssetManager.h"
#include "Graphics/FontRenderer.h"

namespace TRE
{
#pragma region HasIsDirty
	// this magic is brought to you by https://stackoverflow.com/a/16000226
	// should this struct be somewhere else? hmm......

	// class/struct does NOT have member named "m_IsDirty"
	template <typename T, typename = int>
	struct HasIsDirty : std::false_type { };

	// class/struct has member named "m_IsDirty"
	template <typename T>
	struct HasIsDirty <T, decltype((void)T::m_IsDirty, 0)> : std::true_type { };
#pragma endregion

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

		//AssetManager::Instance().PrintAllAssets();

		//create entity
		auto& entity = m_SelectionManager->GetSelectedEntity();

		//object name
		if (entity != nullptr)
		{
			// This updates the tables
			m_SelectionManager->UpdateSelectedEntity();

			if (m_SelectionManager->GetSelectedEntity() == nullptr)
			{
				ImGui::End();
				return;
			}

			auto& properties = m_SelectionManager->GetSelectedEntityProperty();

			bool isPrefabInstance = false;

			// Show option of prefabing possibility if have prefab
			if (entity->HasComponent<Prefabing>())
			{
				if (ImGui::Button("Overwrite Prefab"))
				{
					ECSSystemManager::Instance().GetSystem<PrefabSystem>()->SavePrefabEntity(entity, false);
				}
				ImGui::SameLine();
			}
			if (ImGui::Button("Save As New Prefab"))
			{
				ECSSystemManager::Instance().GetSystem<PrefabSystem>()->SavePrefabEntity(entity);
			}
			if (entity->HasComponent<Prefabing>())
			{
				// Is Prefab Instance
				isPrefabInstance = true;
				ImGui::SameLine();
				Prefabing& pref{ entity->GetComponent<Prefabing>() };
				ImGui::SameLine();
				if (ImGui::Button("Clone Prefab"))
				{
					Entity instance{ ECSSystemManager::Instance().GetSystem<PrefabSystem>()->CreatePrefabEntityInstance(pref.m_PrefabGUID) };
					m_SelectionManager->UpdateSelectedEntity();
				}
				ImGui::SameLine();
				if (ImGui::Button("UnPrefab"))
				{
					ECSSystemManager::Instance().GetSystem<PrefabSystem>()->UnPrefabInstance(entity);
					isPrefabInstance = false;
				}
				bool haveEdits{ !pref.m_Overrides.empty() || !pref.m_AddeddComps.empty() || !pref.m_RemovedComps.empty() };
				ImGui::SameLine();
				if (haveEdits && ImGui::Button("Revert"))
				{
					ECSSystemManager::Instance().GetSystem<PrefabSystem>()->RevertInstance(entity, pref.m_PrefabGUID);
					m_SelectionManager->UpdateSelectedEntity();
				}
				else if (!haveEdits)
				{
					ImGui::NewLine();
				}
			}

			// Right Click to Add Component
			bool openAddComp = false;
			if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows))
			{
				//Remove Component
				if (ImGui::BeginPopupContextWindow())
				{
					if (ImGui::Selectable("AddComponent"))
					{
						openAddComp = true;
					}
					ImGui::EndPopup();
				}
			}

			// View all inspectable components
			for (auto& List : properties)
			{
				//std::cout << "properties: " << List.first.c_str() << "\n";
				for (size_t c{}; c < List.first.size(); ++c)
				{
					std::string charac{ List.first[c] };
					float diff{ static_cast<float>(c) / static_cast<float>(List.first.size()) };
					ImGui::TextColored({ 1, diff, 0, 1 }, charac.c_str());
					ImGui::SameLine(0, 0);
					// ^ leaving the spacing as the default value of -1 does weird stuff
					// but 0 ensures there are no gaps! :D
				}

				if (ECSManager::Instance().IsRemovableComponent(List.first))
				{
					if (ImGui::Button("Remove Component", ImVec2(-FLT_MIN, 0.0f)) && ImGui::IsMouseClicked(ImGuiMouseButton_Left) || ImGui::IsItemClicked())
					{
						// Update Prefabing if have
						if (isPrefabInstance)
						{
							entity->GetComponent<Prefabing>().m_RemovedComps.emplace(List.first);
						}

						ECSManager::Instance().RemCompFromName(entity, List.first);
						m_SelectionManager->UpdateSelectedEntity();
						break;
					}

					// Right Click to Remove Component
					if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows))
					{
						//Remove Component
						if (ImGui::BeginPopupContextWindow())
						{
							bool tobreak = false;
							std::string buttonString{ "Remove " + List.first };
							if (ImGui::Selectable(buttonString.c_str()))
							{
								// Update Prefabing if have
								if (isPrefabInstance)
								{
									entity->GetComponent<Prefabing>().m_RemovedComps.emplace(List.first);
								}

								ECSManager::Instance().RemCompFromName(entity, List.first);
								m_SelectionManager->UpdateSelectedEntity();
								tobreak = true;
							}
							ImGui::EndPopup();
							if (tobreak) break;
						}
					}
				}

				ImGui::NewLine();
				for (auto& [Name, Data] : List.second)
				{
#pragma region Normal Components
					bool UpdatedData = false;

					std::string NameField = "##" + entity->GetGUID() + "/" + Name;
					std::string NameStr = Name.substr(Name.find_last_of('/') + 1);

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
						ImGui::TextColored({ 0, 1, 1, 1 }, NameStr.c_str());
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
								UpdatedData = UpdatedData ? true : ImGui::DragInt(NameField.c_str(), &Value);
							}
							else if constexpr (std::is_same_v<T, float>)
							{
								UpdatedData = UpdatedData ? true : ImGui::DragFloat(NameField.c_str(), &Value);
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
							else if constexpr (std::is_same_v<T, glm::vec2>)
							{
								float pos[2]{ Value.x, Value.y };
								UpdatedData = UpdatedData ? true : ImGui::DragFloat2(NameField.c_str(), pos);
								Value = { pos[0], pos[1] };
							}
							else if constexpr (std::is_same_v<T, glm::vec3> || std::is_same_v<T, FMOD_VECTOR>)
							{
								float pos[3]{ Value.x, Value.y, Value.z };
								UpdatedData = UpdatedData ? true : ImGui::DragFloat3(NameField.c_str(), pos);
								Value = { pos[0], pos[1], pos[2] };
							}
							else if constexpr (std::is_same_v<T, glm::vec4>)
							{
								float data[4]{ Value.x, Value.y, Value.z, Value.w };
								UpdatedData = UpdatedData ? true : ImGui::DragFloat4(NameField.c_str(), data);
								Value = { data[0], data[1], data[2], data[3] };
							}
							else if constexpr (std::is_same_v<T, Color>)
							{
								float color[4]{ Value.m_Value.r, Value.m_Value.g, Value.m_Value.b, Value.m_Value.a };
								UpdatedData = UpdatedData ? true : ImGui::ColorEdit4(NameField.c_str(), color);
								Value.m_Value = { color[0], color[1], color[2], color[3] };
							}
							else if constexpr (std::is_same_v<T, Color3>)
							{
								float color[3]{ Value.m_Value.r, Value.m_Value.g, Value.m_Value.b };
								UpdatedData = UpdatedData ? true : ImGui::ColorEdit3(NameField.c_str(), color);
								Value.m_Value = { color[0], color[1], color[2] };
							}
							else if constexpr (std::is_same_v<T, resource_ref>)
							{
								static char resourceName[200];
								strcpy(resourceName, AssetManager::Instance().GetName(Value.m_Value).c_str());
								std::string handle = "##" + std::to_string(Value.m_Value);
								if (ImGui::InputText(handle.c_str(), resourceName, sizeof(resourceName), ImGuiInputTextFlags_ReadOnly) || ImGui::IsItemHovered())
								{
									if (ImGui::BeginDragDropTarget())
									{
										if (const ImGuiPayload* payload = ImGui::GetDragDropPayload(); payload != nullptr)
										{
											if (payload->IsDataType("m_TextureResource"))
											{
												std::string assetName = (const char*)payload->Data;
												assetName = assetName.substr(assetName.find_last_of('\\') + 1);
												assetName.erase(assetName.find(".png")); 	// This is to remove unneeded data at the end after ".fbx"
												assetName += ".png";

												std::shared_ptr<VulkanTexture> droppedTexture;

												//Check if asset is already compiled
												//Compiled before
												if (AssetManager::Instance().Contains(assetName))
												{
													//Load into memory
													if (droppedTexture = AssetManager::Instance().GetAsset<VulkanTexture>(assetName); droppedTexture == nullptr)
													{
														AssetManager::Instance().AddAsset<VulkanTexture>(assetName);
														droppedTexture = AssetManager::Instance().GetAsset<VulkanTexture>(assetName);
													}
												}
												else
												{
													//Compile and load asset
													droppedTexture = AssetManager::Instance().CompileAndLoad<VulkanTexture>(assetName);
												}
												Value.m_Value = droppedTexture->GetHandle();
											}
										}
										else
										{
											TRE_CORE_ERROR("Failed to get drag drop texture");
										}
										ImGui::EndDragDropTarget();
									}
								}
							}
							else if constexpr (std::is_same_v<T, resource_list>)
							{
								std::string imguiHandle = "##" + std::to_string(Value.m_Value);
								std::string selected = AssetManager::Instance().GetName(Value.m_Value);

								if (ImGui::BeginCombo(imguiHandle.c_str(), selected.c_str()))
								{
									if (ImGui::Selectable("None", false))
									{
										Value.m_Value = 0;
									}

									if (Value.m_Type == "MATERIAL")
									{
										auto vec = AssetManager::Instance().GetAssetsOfType<Material>();
										std::ranges::sort(vec, [](const auto& mat1, const auto& mat2)
											{
												std::string mat1Name{ AssetManager::Instance().GetName(mat1->GetHandle()) };
												for (char& ch : mat1Name)
													ch = static_cast<char>(tolower(ch));

												std::string mat2Name{ AssetManager::Instance().GetName(mat2->GetHandle()) };
												for (char& ch : mat2Name)
													ch = static_cast<char>(tolower(ch));

												return mat1Name < mat2Name;
											});

										for (const auto& material : vec)
										{
											ResourceHandle handle = material->GetHandle();
											std::string name = AssetManager::Instance().GetName(handle);
											bool isSelected = (selected == name);
											if (ImGui::Selectable(name.c_str(), isSelected))
											{
												selected = name;
												Value.m_Value = handle;
												break;
											}
											if (isSelected)
												ImGui::SetItemDefaultFocus();
										}
									}
									else if (Value.m_Type == "MESH")
									{
										auto vec = AssetManager::Instance().GetAssetsOfType<RenderObject>();
										std::ranges::sort(vec, [](const auto& mesh1, const auto& mesh2)
											{
												std::string mesh1Name{ AssetManager::Instance().GetName(mesh1->GetHandle()) };
												for (char& ch : mesh1Name)
													ch = static_cast<char>(tolower(ch));

												std::string mesh2Name{ AssetManager::Instance().GetName(mesh2->GetHandle()) };
												for (char& ch : mesh2Name)
													ch = static_cast<char>(tolower(ch));

												return mesh1Name < mesh2Name;
											});

										for (const auto& mesh : vec)
										{
											ResourceHandle handle = mesh->GetHandle();
											std::string name = AssetManager::Instance().GetName(handle);
											bool isSelected = (selected == name);
											if (ImGui::Selectable(name.c_str(), isSelected))
											{
												selected = name;
												Value.m_Value = handle;
												break;
											}
											if (isSelected)
												ImGui::SetItemDefaultFocus();
										}
									}
									else if (Value.m_Type == "TEXTURE")
									{
										auto vec = AssetManager::Instance().GetAssetsOfType<VulkanTexture>();
										std::ranges::sort(vec, [](const auto& texture1, const auto& texture2)
											{
												std::string texture1Name{ AssetManager::Instance().GetName(texture1->GetHandle()) };
												for (char& ch : texture1Name)
													ch = static_cast<char>(tolower(ch));

												std::string texture2Name{ AssetManager::Instance().GetName(texture2->GetHandle()) };
												for (char& ch : texture2Name)
													ch = static_cast<char>(tolower(ch));

												return texture1Name < texture2Name;
											});

										for (const auto& texture : vec)
										{
											ResourceHandle handle = texture->GetHandle();
											std::string name = AssetManager::Instance().GetName(handle);
											bool isSelected = (selected == name);
											if (ImGui::Selectable(name.c_str(), isSelected))
											{
												selected = name;
												Value.m_Value = handle;
												break;
											}
											if (isSelected)
												ImGui::SetItemDefaultFocus();
										}
									}
									else
									{

									}

									ImGui::EndCombo();
								}
							}
							else if constexpr (std::is_same_v<T, audio_file_dropdown>)
							{
								; // UpdatedData and stuff for audio_file_dropdown here
							}
							else if constexpr (std::is_same_v<T, waypoint>)
							{
								float pos[3]{ Value.m_Value.x, Value.m_Value.y, Value.m_Value.z };
								UpdatedData = UpdatedData ? true : ImGui::DragFloat3(NameField.c_str(), pos);
								Value.m_Value = { pos[0], pos[1], pos[2] };
							}
							else if constexpr (std::is_same_v<T, std::vector<waypoint>>)
							{
								ImGui::SameLine();
								std::string label{ "Add New Waypoint##" + entity->GetGUID() };
								if (ImGui::Button(label.c_str()))
								{
									waypoint wp{};
									wp.m_Value = entity->GetComponent<Transform>().m_Position;
									Value.push_back(wp);
									UpdatedData = true;
								}

								std::vector<int> deleteInd{};
								for (size_t i{}; i < Value.size(); ++i)
								{
									float pos[3]{ Value[i].m_Value.x, Value[i].m_Value.y, Value[i].m_Value.z };
									UpdatedData = UpdatedData ? true : ImGui::DragFloat3(NameField.c_str(), pos);
									Value[i].m_Value = { pos[0], pos[1], pos[2] };
									ImGui::SameLine();
									//// Teleport to that position (Removed since it does not work since the position of Transform will get overwritten)
									//if (ImGui::Button("O"))
									//{
									//	entity->GetComponent<Transform>().m_Position = Value[i].m_Value;
									//	entity->GetComponent<Transform>().m_IsDirty = true;
									//	UpdatedData = true;
									//}
									//ImGui::SameLine();
									// Move waypoint up
									std::string uplabel{ "^##" + entity->GetGUID() + std::to_string(i) };
									if (ImGui::Button(uplabel.c_str()))
									{
										if (i > 0)
										{
											waypoint wp{ Value[i - 1] };
											Value[i - 1] = Value[i];
											Value[i] = wp;
											UpdatedData = true;
										}
									}
									ImGui::SameLine();
									// Move waypoint down
									std::string downlabel{ "v##" + entity->GetGUID() + std::to_string(i) };
									if (ImGui::Button(downlabel.c_str()))
									{
										if (i < Value.size() - 1)
										{
											waypoint wp{ Value[i] };
											Value[i] = Value[i + 1];
											Value[i + 1] = wp;
											UpdatedData = true;
										}
									}
									ImGui::SameLine();
									// Delete waypoint
									std::string xlabel{ "x##" + entity->GetGUID() + std::to_string(i) };
									if (ImGui::Button(xlabel.c_str()))
									{
										deleteInd.emplace_back(static_cast<int>(i));
										UpdatedData = true;
									}
								}

								// Delete each index
								for (int i{ static_cast<int>(deleteInd.size() - 1) }; i >= 0; --i)
								{
									Value.erase(Value.begin() + deleteInd[i]);
								}
							}
							else if constexpr (std::is_same_v<T, CollisionLayer>)
							{
								// display the name of the layer currently being used by the entity in the dropdown
								if (ImGui::BeginCombo("##CollisionLayerDropdown", CollisionLayer::m_LayerNameList[static_cast<int>(Value.m_LayerID)].first.c_str()))
								{
									for (auto& element : CollisionLayer::m_LayerNameList)
									{
										if (ImGui::Selectable(element.first.c_str()))
										{
											UpdatedData = true;
											Value.m_LayerID = element.second;
										}
									}
									ImGui::EndCombo();
								}
							}
							else if constexpr (std::is_same_v<T, PhysicsMaterial>)
							{
								// display the name of the layer currently being used by the entity in the dropdown
								if (ImGui::BeginCombo("##PhysicsMaterialDropdown", PhysicsMaterial::m_LayerNameList[static_cast<int>(Value.m_MaterialID)].first.c_str()))
								{
									for (auto& element : PhysicsMaterial::m_LayerNameList)
									{
										if (ImGui::Selectable(element.first.c_str()))
										{
											UpdatedData = true;
											Value.m_MaterialID = element.second;
										}
									}
									ImGui::EndCombo();
								}
							}
							else if constexpr (std::is_same_v<T, FontType>)
							{
								std::string selected = Value.m_Value;

								if (ImGui::BeginCombo("Font Type", selected.c_str()))
								{
									if (ImGui::Selectable("None", false))
									{
										Value.m_Value = "";
									}

									if (Value.m_Type == "FontType")
									{
										std::vector<std::string> LoadFontTypes = FontRenderer::GetLoadedFonts();
										std::cout << "Loaded Font Count: " << LoadFontTypes.size() << std::endl;
										std::ranges::sort(LoadFontTypes, [](const auto& type1, const auto& type2)
											{
												for (char ch : type1)
													ch = static_cast<char>(tolower(ch));

												for (char ch : type2)
													ch = static_cast<char>(tolower(ch));

												return type1 < type2;
											});

										for (const auto& material : LoadFontTypes)
										{
											bool isSelected = (selected == material);
											if (ImGui::Selectable(material.c_str(), isSelected))
											{
												selected = material;
												Value.m_Value = material;
												break;
											}
											if (isSelected)
												ImGui::SetItemDefaultFocus();
										}
									}

									ImGui::EndCombo();
								}

								if (ImGui::BeginDragDropTarget())
								{
									if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("m_Font"))
									{
										std::string assetName = (const char*)payload->Data;
										assetName = assetName.substr(assetName.find_last_of('\\') + 1);
										assetName.erase(assetName.find(".ttf")); 	// This is to remove unneeded data at the end after ".fbx"
										assetName += ".ttf";
										
										std::string FilePath = std::filesystem::current_path().parent_path().string() + "\\"  + "Assets/Font/" + assetName;
										
										//Load Font here
										FontRenderer::LoadFont(FilePath);

										Value.m_Value = FilePath;
									}
									else
									{
										TRE_CORE_ERROR("Failed to get drag drop font");
									}
									ImGui::EndDragDropTarget();
								}
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
#pragma endregion

#if true
#pragma region Script Component
					if (List.first == ComponentManager::Instance().GetComponentName<ScriptComponent>())
					{
						// here we will have a portion of the

						if(ImGui::Button("Add Script"))
						{
							ImGui::OpenPopup("AddScript");
						}

						if(ImGui::Button("Remove Script"))
						{
							ImGui::OpenPopup("RemoveScript");
						}

						if(ImGui::BeginPopup("AddScript"))
						{

							if(ImGui::BeginCombo("##Scripts", "Scripts"))
							{
								std::vector<std::string> scripts = ScriptEngine::s_ScriptEngineData->RegisteredScriptClasses;
								for (auto& script : scripts)
								{
									if (ImGui::Selectable(script.c_str()))
									{
										entity->GetComponent<ScriptComponent>().AddScriptToComponent(script);
									}
								}
								ImGui::EndCombo();
							}
							ImGui::EndPopup();
						}

						if(ImGui::BeginPopup("RemoveScript"))
						{
							if(ImGui::BeginCombo("##ActiveScripts", "Active Scripts"))
							{
								std::vector<std::string> scripts = entity->GetComponent<ScriptComponent>().m_RegisteredScripts;
								for (auto& script : scripts)
								{
									if (ImGui::Selectable(script.c_str()))
									{
										entity->GetComponent<ScriptComponent>().RemoveScriptFromComponent(script);
									}
								}
								ImGui::EndCombo();
							}
							ImGui::EndPopup();
						}
						
						if (ScriptEngine::s_ScriptEngineData->EntityFieldMap.find(entity->GetGUID()) != ScriptEngine::s_ScriptEngineData->EntityFieldMap.end())
						{
							// Displaying all the script data in the entity
							std::vector<std::shared_ptr<ScriptInstance>> instances = ScriptEngine::GetAllEntityScripts(entity->GetGUID());

							//check if the vector is empty
							if(instances.empty())
{
								ImGui::Text("No Scripts");
							}

							else
							{
								//display all the scripts
								for (auto& instance : instances)
								{
									
									if(ImGui::CollapsingHeader(instance->GetScriptClass()->GetScriptClassName().c_str(),ImGuiTreeNodeFlags_DefaultOpen))
									{
										const auto& fields = instance->GetScriptClass()->GetFields();
										for (const auto& [name, inst] : fields)
										{
											ImGui::Text(name.c_str());
											ImGui::SameLine();

											if (inst.m_Type == ScriptFieldTypes::None)
											{
												ImGui::TextColored({ 1, 0, 0, 1 }, "[Data Type] ScriptFieldTypes::None");
											}
											else if (inst.m_Type == ScriptFieldTypes::Float)
											{
												float Value = instance->GetFieldValue<float>(name);
												UpdatedData = UpdatedData ? true : ImGui::DragFloat(NameField.c_str(), &Value);
												instance->SetFieldValue<float>(name, Value);
											}
											else if (inst.m_Type == ScriptFieldTypes::Double)
											{
												double Value = instance->GetFieldValue<double>(name);
												UpdatedData = UpdatedData ? true : ImGui::InputDouble(NameField.c_str(), &Value);
												instance->SetFieldValue<double>(name, Value);
											}
											else if (inst.m_Type == ScriptFieldTypes::Boolean)
											{
												bool Value = instance->GetFieldValue<bool>(name);
												UpdatedData = UpdatedData ? true : ImGui::Checkbox(NameField.c_str(), &Value);
												instance->SetFieldValue<bool>(name, Value);
											}
											else if (inst.m_Type == ScriptFieldTypes::Char)
											{
												ImGui::TextColored({ 1, 0, 0, 1 }, "[Data Type] ScriptFieldTypes::Char");
											}
											else if (inst.m_Type == ScriptFieldTypes::Byte)
											{
												ImGui::TextColored({ 1, 0, 0, 1 }, "[Data Type] ScriptFieldTypes::Byte");
											}
											else if (inst.m_Type == ScriptFieldTypes::Short)
											{
												short data = instance->GetFieldValue<short>(name);
												int Value = static_cast<int>(data);
												UpdatedData = UpdatedData ? true : ImGui::InputInt(NameField.c_str(), &Value);
												instance->SetFieldValue<short>(name, static_cast<short>(Value));
											}
											else if (inst.m_Type == ScriptFieldTypes::Int)
											{
												int Value = instance->GetFieldValue<int>(name);
												UpdatedData = UpdatedData ? true : ImGui::InputInt(NameField.c_str(), &Value);
												instance->SetFieldValue<int>(name, Value);
											}
											else if (inst.m_Type == ScriptFieldTypes::Long)
											{
												long data = instance->GetFieldValue<long>(name);
												int Value = static_cast<int>(data);
												UpdatedData = UpdatedData ? true : ImGui::InputInt(NameField.c_str(), &Value);
												instance->SetFieldValue<long>(name, static_cast<long>(Value));
											}
											else if (inst.m_Type == ScriptFieldTypes::UnsignedChar)
											{
												ImGui::TextColored({ 1, 0, 0, 1 }, "[Data Type] ScriptFieldTypes::None");
											}
											else if (inst.m_Type == ScriptFieldTypes::UnsignedInt)
											{
												unsigned int data = instance->GetFieldValue<unsigned int>(name);
												int Value = static_cast<int>(data);
												UpdatedData = UpdatedData ? true : ImGui::InputInt(NameField.c_str(), &Value);
												instance->SetFieldValue<unsigned int>(name, static_cast<unsigned int>(Value));
											}
											else if (inst.m_Type == ScriptFieldTypes::UnsignedLong)
											{
												unsigned long data = instance->GetFieldValue<unsigned long>(name);
												int Value = static_cast<int>(data);
												UpdatedData = UpdatedData ? true : ImGui::InputInt(NameField.c_str(), &Value);
												instance->SetFieldValue<unsigned long>(name, static_cast<unsigned long>(Value));
											}
											else if (inst.m_Type == ScriptFieldTypes::Vector2)
											{
												ImGui::TextColored({ 1, 0, 0, 1 }, "[Data Type] ScriptFieldTypes::Vector2");
											}
											else if (inst.m_Type == ScriptFieldTypes::Vector3)
											{
												ImGui::TextColored({ 1, 0, 0, 1 }, "[Data Type] ScriptFieldTypes::Vector3");
											}
											else if (inst.m_Type == ScriptFieldTypes::Vector4)
											{
												ImGui::TextColored({ 1, 0, 0, 1 }, "[Data Type] ScriptFieldTypes::Vector4");
											}
											else if (inst.m_Type == ScriptFieldTypes::Entity)
											{
												ImGui::TextColored({ 1, 0, 0, 1 }, "[Data Type] ScriptFieldTypes::Entity");
											}
											else if (inst.m_Type == ScriptFieldTypes::String)
											{
												ImGui::TextColored({ 1, 0, 0, 1 }, "[Data Type] ScriptFieldTypes::String TOO LAGGY TO LET IT RUN");
												/*MonoString* data = instance->GetFieldValue<MonoString*>(name);
												std::string Value = mono_string_to_utf8(data);
												UpdatedData = UpdatedData ? true : ImGui::InputText(NameField.c_str(), &Value);
												instance->SetFieldValue<MonoString*>(name, mono_string_new(mono_domain_get(), Value.c_str()));*/
											}
											else
											{
												std::string function{ __FUNCTION__ };
												TRE_ERROR("[" + function + "] Not all ScriptFieldTypes is accounted!");
												assert(false && "Refer to Error above");
											}
										
										}
									}

								}
							}
						}

						//the green part is to make the button bigger
						if (ImGui::Button("Reload"/*, ImVec2(-FLT_MIN, 0.0f)) && ImGui::IsMouseClicked(ImGuiMouseButton_Left) || ImGui::IsItemClicked(*/))
						{

						}
					}
#pragma endregion
#endif

					// Do additional stuff if values are change for certain components
					if (UpdatedData)
					{
						// if only there was some way to get types from strings... :(
						std::string compName{ List.first };

						// capturing compName by reference
						auto SetIsDirty = [&compName]<typename Comp>(const Entity & e)
						{
							if (compName == ComponentManager::Instance().GetComponentName<Comp>())
								if (HasIsDirty<Comp>::value)
									e->GetComponent<Comp>().m_IsDirty = true;
						};

						// because I can't do SetIsDirty<Component>(entity) :(
						SetIsDirty.operator() < Transform > (entity);
						SetIsDirty.operator() < Rigidbody > (entity);
						SetIsDirty.operator() < SphereCollider > (entity);
						SetIsDirty.operator() < BoxCollider > (entity);
						SetIsDirty.operator() < CapsuleCollider > (entity);
						SetIsDirty.operator() < CylinderCollider > (entity);
						SetIsDirty.operator() < ScriptComponent > (entity);
						SetIsDirty.operator() < Camera > (entity);
						// add more of your components here! :)
						// my HasIsDirty<> will even check for the dirty bit on your behalf!:D
					}
				}
				ImGui::Separator();
			}

			// Update values into the entity itself
			auto& components = m_SelectionManager->GetSelectedEntityComponents();

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
			if (ImGui::Button("Add Component", ImVec2(-FLT_MIN, 0.0f)) || openAddComp)
			{
				ImGui::OpenPopup("AddComponent");
			}

			if (ImGui::BeginPopup("AddComponent"))
			{
				std::vector<std::string> nonAddedComponents{ ECSManager::Instance().GetAllNonAddedComponents(entity) };
				std::ranges::sort(nonAddedComponents); // std::sort(nonAddedComponents.begin(), nonAddedComponents.end());
				for (std::string& compName : nonAddedComponents)
				{
					if (ImGui::Selectable(compName.c_str()))
					{
						ECSManager::Instance().AddCompFromName(entity, compName);

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

						// Force add additional components for specific components
						if (compName == ComponentManager::Instance().GetComponentName<Camera>())
						{
							// Force Add AudioListener
							ECSManager::Instance().AddCompFromName(entity, ComponentManager::Instance().GetComponentName<AudioListener>());
						}
						else if (compName == ComponentManager::Instance().GetComponentName<ScriptComponent>())
						{
							entity->GetComponent<ScriptComponent>().m_GUID = entity->GetGUID();;
						}

						m_SelectionManager->UpdateSelectedEntity();
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