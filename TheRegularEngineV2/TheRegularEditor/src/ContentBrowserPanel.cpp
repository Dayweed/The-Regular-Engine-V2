#include "pch.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "Imgui/imgui_internal.h"
#include "Imgui/imgui.h"
#include "Utilities.h"
#include "EditorAssetManager.h"
#include "ContentBrowserPanel.h"
#include "Graphics/Material.h"
#include "EditorSystem.h"
#include "Core/GameLoop.h"
#include "ShaderTypes/PBRShader.h"
#include "Graphics/EditorCamera.h"

namespace TRE
{
	static std::string lastSceneClicked;
	ContentBrowserPanel::ContentBrowserPanel(const std::shared_ptr<SelectionManager>& Selection_Manager, const std::shared_ptr<AssetSelector>& assetSelector)
	{
		m_SelectionManager = Selection_Manager;
		m_AssetSelector = assetSelector;
		
		m_AssetDirectory = std::filesystem::current_path().parent_path();
		m_AssetDirectory += "\\Assets";
		m_SceneDirectory = std::filesystem::current_path().parent_path();
		m_SceneDirectory += "\\Scenes";
		m_ScriptDirectory = std::filesystem::current_path().parent_path();
		m_ScriptDirectory += "\\Scripts";
		m_CurrentDirectory = m_AssetDirectory;

		//Custom Flag Combinations
		m_PopUps |= ImGuiWindowFlags_NoResize;
		m_PopUps |= ImGuiWindowFlags_NoCollapse;
		m_PopUps |= ImGuiWindowFlags_NoMove;
		m_PopUps |= ImGuiWindowFlags_NoSavedSettings;
		m_PopUps |= ImGuiWindowFlags_AlwaysAutoResize;
	}

	ContentBrowserPanel::~ContentBrowserPanel()
	{

	}

	void ContentBrowserPanel::PollItems()
	{
		m_Assets.clear();
		for (auto& p : std::filesystem::directory_iterator(m_CurrentDirectory))
		{
			const std::filesystem::path path = p.path();
			const std::filesystem::path relativePath =	std::filesystem::relative(path, m_AssetDirectory);
			const std::string filenameString = relativePath.filename().string();
			if (p.is_directory())
			{
				//if the asset is a file
				m_Assets.emplace_back(Asset{ true, m_FolderIconID ,"m_Invalid", filenameString, p.path()});
			}
			else
			{
				Asset newAsset{};

				//Determine the type of resource to drag and drop
				const bool isImage = filenameString.ends_with(".png");
				const bool isAudio = filenameString.ends_with(".wav");
				const bool isShader = filenameString.ends_with(".glsl");
				const bool isScene = filenameString.ends_with(".json");
				const bool isPrefab = filenameString.ends_with(".prefab");
				const bool isMeta = filenameString.ends_with(".meta");
				const bool isFont = filenameString.ends_with(".ttf");
				const bool is3DObj = filenameString.ends_with(".fbx");
				const bool isDesc = filenameString.ends_with(".desc");
				const bool isMaterial = filenameString.ends_with(".material.desc");
				const bool isScript = filenameString.ends_with(".cs");

				//Determine the icon type
				newAsset.m_TextureID = isImage							? m_ImageIconID : newAsset.m_TextureID;
				newAsset.m_TextureID = isAudio							? m_AudioIconID : newAsset.m_TextureID;
				newAsset.m_TextureID = isScene							? m_SceneIconID : newAsset.m_TextureID;
				newAsset.m_TextureID = isShader							? m_CubeIconID : newAsset.m_TextureID;
				newAsset.m_TextureID = isPrefab 						? m_PrefabIconID : newAsset.m_TextureID;
				newAsset.m_TextureID = isMeta							? m_MetaIconID : newAsset.m_TextureID;
				newAsset.m_TextureID = isFont							? m_FontIconID : newAsset.m_TextureID;
				newAsset.m_TextureID = is3DObj							? m_3DObjIconID : newAsset.m_TextureID;
				newAsset.m_TextureID = isMaterial						? m_MaterialIconID : newAsset.m_TextureID;
				newAsset.m_TextureID = isScript							? m_CSScriptIconID : newAsset.m_TextureID;

				if (isImage || isAudio || isShader || isScene || isPrefab || isFont || is3DObj || isMaterial || isScript)
				{
					//Allow Dragging of these file types
					newAsset.m_ResourceType = isImage		? "m_TextureResource" : newAsset.m_ResourceType;
					newAsset.m_ResourceType = isAudio		? "m_AudioResource" : newAsset.m_ResourceType;
					newAsset.m_ResourceType = isShader		? "m_ShaderResource" : newAsset.m_ResourceType;
					newAsset.m_ResourceType = isFont		? "m_FontResource" : newAsset.m_ResourceType;
					newAsset.m_ResourceType = isScene		? "m_Scene" : newAsset.m_ResourceType;
					newAsset.m_ResourceType = isPrefab		? "m_Prefab" : newAsset.m_ResourceType;
					newAsset.m_ResourceType = is3DObj		? "m_3DObject" : newAsset.m_ResourceType;
					newAsset.m_ResourceType = isMaterial	? "m_Material" : newAsset.m_ResourceType;
					newAsset.m_ResourceType = isScript		? "m_Script" : newAsset.m_ResourceType;
				}

				if (isMaterial)
				{
					std::string hexHandle = filenameString.substr(0, filenameString.find_first_of('.'));
					newAsset.m_FileName = AssetManager::Instance().GetName(hexHandle);
					newAsset.m_Path = "../Resources/" + hexHandle + ".material";

					//Because material technically is a descriptor file, we need to add it to the list
					m_Assets.emplace_back(newAsset);
				}
				else
				{
					//Set the path and filename
					newAsset.m_Path = path;
					newAsset.m_FileName = filenameString;
				}
				
				//Add the asset to the list if not a descriptor file
				if(!isDesc)
					m_Assets.emplace_back(newAsset);
			}
		}

		//Add material instances in to see on content browser if it is in m_AssetDirectory
		/*if (m_CurrentDirectory == m_AssetDirectory)
		{
			for (const auto& mat : AssetManager::Instance().GetAssetsOfType<Material>())
			{
				Asset materialAsset{};
				materialAsset.m_TextureID = m_TmpTexturesID;
				materialAsset.m_ResourceType = "m_Material";
				materialAsset.m_FileName = AssetManager::Instance().GetName(mat->GetHandle());
				materialAsset.m_Path = "../Resources/" + mat->GetHandleHex() + ".material";

				m_Assets.emplace_back(materialAsset);
			}
		}*/
	}

	void ContentBrowserPanel::BrowseProjectFiles()
	{
		//Folder List Display
		if (ImGui::BeginChild("Folder List", ImVec2(ImGui::GetContentRegionAvail().x * 0.2f, ImGui::GetContentRegionAvail().y), true))
		{
			if (ImGui::IsWindowHovered())
			{
				if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) || ImGui::IsMouseClicked(ImGuiMouseButton_Right))
				{
					ImGui::SetWindowFocus();
				}
			}
			if (ImGui::Button(m_AssetDirectory.filename().string().c_str()))
			{
				m_CurrentDirectory = m_AssetDirectory;
				PollItems();
			}
			if (ImGui::Button(m_SceneDirectory.filename().string().c_str()))
			{
				m_CurrentDirectory = m_SceneDirectory;
				PollItems();
			}
			if (ImGui::Button(m_ScriptDirectory.filename().string().c_str()))
			{
				m_CurrentDirectory = m_ScriptDirectory;
				PollItems();
			}
		}
		ImGui::EndChild();
		ImGui::SameLine();

		bool isHovered = false;
		//Item List Display
		if (ImGui::BeginChild("ItemList", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y), true))
		{
			if (ImGui::IsWindowHovered())
			{
				if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) || ImGui::IsMouseClicked(ImGuiMouseButton_Right))
				{
					ImGui::SetWindowFocus();
				}
			}

			/*ImGui::Text("Path:[%s]", m_CurrentDirectory.string().data());
			if (ImGui::Button("Open File Explorer"))
			{
				(void)FileExplorer::OpenFileExplorer(nullptr);
			}*/

			if (m_CurrentDirectory.compare(m_AssetDirectory) != 0 && m_CurrentDirectory.compare(m_SceneDirectory) != 0)
			{
				if (ImGui::Button("Back"))
				{
					m_CurrentDirectory = m_CurrentDirectory.parent_path();
					PollItems();
				}
			}
			ImGui::Separator();

			//right click to open popup menu
			if (ImGui::BeginPopupContextWindow())
			{
				if (ImGui::BeginMenu("Create new material"))
				{
					if (ImGui::MenuItem("PBR"))
					{
						MaterialDescriptorFile descriptorFileMaterial;
						descriptorFileMaterial.Generate();
						
						std::unique_ptr<Material> newMaterial = std::make_unique<Material>(PBR::GetShaderHandle());
						newMaterial->SetHandle(descriptorFileMaterial.GetResourceHandle());
						newMaterial->Invalidate();

						AssetManager::Instance().AddAsset(descriptorFileMaterial.GetAssetPath(), std::move(newMaterial));
					}
					if (ImGui::MenuItem("Others"))
					{
	
					}

					ImGui::EndMenu();
				}
				ImGui::EndPopup();
			}

			
			const float panelWidth = ImGui::GetContentRegionAvail().x;
			int cols = static_cast<int>(panelWidth / m_CellSize);
			if (cols < 1)
				cols = 1;

			ImGui::Columns(cols, nullptr, false);

			m_AssetClicked = false;

			isHovered = ImGui::IsWindowHovered();

			for (int count{}; auto & item: m_Assets)
			{
				ImGui::PushID(count++);

				if (item.m_Folder)
				{
					if (ImGui::ImageButton(item.m_TextureID,{m_ImgSize, m_ImgSize}, { 0,0 }, { 1,1 }))
					{
						//step into folder selected
						m_CurrentDirectory /= item.m_Path.filename();
						m_CurrentTimer = m_RefreshRate;
					}
					ImGui::TextWrapped("%s", item.m_FileName.c_str());
				}
				else
				{
					if (ImGui::ImageButton(item.m_TextureID, { m_ImgSize, m_ImgSize }, { 0,0 }, { 1,1 }))
					{
						//No Click Action
						if (item.m_ResourceType == "_Invalid")
						{
							if (!m_InvalidResourcePopUp)
								m_InvalidResourcePopUp = true;
						}
						else if (item.m_ResourceType == "m_Prefab")
						{
							if (!GameLoop::Instance().IsGameRunning() && !GameLoop::Instance().GetGameSimulating())
							{
								EditorSystemManager::Instance().GetSystem<EditorSystem>()->GetSelectionManager()->ClearSelectedEntity();
								PrefabSystem* prefabsystem{ ECSSystemManager::Instance().GetSystem<PrefabSystem>() };
								std::string prefabGUID{ prefabsystem->ReadPrefabAssetFile(item.m_Path.string()) };
								Entity prefabInstance = prefabsystem->DisplayPrefabInNewScene(prefabGUID);
								m_SelectionManager->SelectEntity(prefabInstance);
							}
						}
						else if (item.m_ResourceType == "m_Scene")
						{
							/*if (!GameLoop::Instance().IsGameRunning() && !GameLoop::Instance().GetGameSimulating())
							{
								SceneManager::Instance().LoadScene(item.m_Path.string());
							}*/
							lastSceneClicked = item.m_Path.string();
						}
						else
						{
							const AssetSelector::AssetType assetType = m_AssetSelector->FindAssetType(item.m_ResourceType);
							m_AssetSelector->SelectAsset(item.m_FileName, assetType);
						}
						m_AssetClicked = true;
					}

					if (ImGui::BeginDragDropSource())
					{
						if (item.m_ResourceType != "_Invalid")
						{
							const std::string tmp = item.m_Path.string();
							const char* itemPath = tmp.c_str();
							ImGui::SetDragDropPayload(item.m_ResourceType.c_str(), itemPath, strlen(itemPath) * sizeof(char));
							ImGui::Text("Move %s", item.m_FileName.c_str());
						}
						ImGui::EndDragDropSource();
					}
					ImGui::TextWrapped("%s", item.m_FileName.c_str());
				}
				ImGui::PopID();
				ImGui::NextColumn();
			}
			if (m_InvalidResourcePopUp)
			{
				ImGui::OpenPopup("Invalid Resource");
				m_InvalidResourcePopUp = false;
			}

			if(ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && m_AssetClicked == false)
			{
				m_AssetSelector->ClearSelectedAsset();
			}

			//create Pop-up
			ImGui::SetNextWindowSize(ImVec2{ 250.f,70.f });
			ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2{ 0.5f,0.5f });
			if (ImGui::BeginPopupModal("Invalid Resource", nullptr, m_PopUps))
			{
				ImGui::Text("This file type is not supported!");
				if (ImGui::Button("Ok"))
				{
					ImGui::CloseCurrentPopup();
				}
				ImGui::EndPopup();
			}
		}
		ImGui::EndChild();

		//For scene opening
		if (isHovered && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
		{
			if (lastSceneClicked != "" && !GameLoop::Instance().IsGameRunning() && !GameLoop::Instance().GetGameSimulating())
			{
				EditorSystemManager::Instance().GetSystem<EditorSystem>()->GetSelectionManager()->ClearSelectedEntity();
				SceneManager::Instance().LoadScene(lastSceneClicked);
				EditorCamera::Instance().Deserialize();
				lastSceneClicked = "";
			}
		}
	}

	void ContentBrowserPanel::Init()
	{
		// Late April Fools Joke (Activate this for sum humor in Content Browser)
#if false
		m_TmpTextures = Util::CreateIcon("icon-file.png");
		m_TmpTexturesID = Util::GetTextureID(m_TmpTextures->GetDescriptorImageInfo());

		m_TmpTextures = Util::CreateIcon("folder_icon.png");
		m_FolderIconID = Util::GetTextureID(m_TmpTextures->GetDescriptorImageInfo());

		m_TmpTextures = Util::CreateIcon("Lcon_kj.png");
		m_SceneIconID = Util::GetTextureID(m_TmpTextures->GetDescriptorImageInfo());

		m_TmpTextures = Util::CreateIcon("cube_icon.png");
		m_CubeIconID = Util::GetTextureID(m_TmpTextures->GetDescriptorImageInfo());

		m_TmpTextures = Util::CreateIcon("Lcon_pg.png");
		m_PrefabIconID = Util::GetTextureID(m_TmpTextures->GetDescriptorImageInfo());

		m_TmpTextures = Util::CreateIcon("Lcon_eh.png");
		m_ImageIconID = Util::GetTextureID(m_TmpTextures->GetDescriptorImageInfo());

		m_TmpTextures = Util::CreateIcon("Lcon_vk.png");
		m_AudioIconID = Util::GetTextureID(m_TmpTextures->GetDescriptorImageInfo());

		m_TmpTextures = Util::CreateIcon("ttf_icon.png");
		m_FontIconID = Util::GetTextureID(m_TmpTextures->GetDescriptorImageInfo());

		m_TmpTextures = Util::CreateIcon("meta_icon.png");
		m_MetaIconID = Util::GetTextureID(m_TmpTextures->GetDescriptorImageInfo());

		m_TmpTextures = Util::CreateIcon("Lcon_ta.png");
		m_MaterialIconID = Util::GetTextureID(m_TmpTextures->GetDescriptorImageInfo());

		m_TmpTextures = Util::CreateIcon("Lcon_lp.png");
		m_3DObjIconID = Util::GetTextureID(m_TmpTextures->GetDescriptorImageInfo());

		m_TmpTextures = Util::CreateIcon("Lcon_al.png");
		m_CSScriptIconID = Util::GetTextureID(m_TmpTextures->GetDescriptorImageInfo());
#else
		m_TmpTextures = Util::CreateIcon("file_icon.png");
		m_TmpTexturesID = Util::GetTextureID(m_TmpTextures->GetDescriptorImageInfo());

		m_TmpTextures = Util::CreateIcon("folder_icon.png");
		m_FolderIconID = Util::GetTextureID(m_TmpTextures->GetDescriptorImageInfo());

		m_TmpTextures = Util::CreateIcon("json_icon.png");
		m_SceneIconID = Util::GetTextureID(m_TmpTextures->GetDescriptorImageInfo());

		m_TmpTextures = Util::CreateIcon("cube_icon.png");
		m_CubeIconID = Util::GetTextureID(m_TmpTextures->GetDescriptorImageInfo());

		m_TmpTextures = Util::CreateIcon("prefab_icon.png");
		m_PrefabIconID = Util::GetTextureID(m_TmpTextures->GetDescriptorImageInfo());
		
		m_TmpTextures = Util::CreateIcon("png_icon.png");
		m_ImageIconID = Util::GetTextureID(m_TmpTextures->GetDescriptorImageInfo());		
		
		m_TmpTextures = Util::CreateIcon("wav_icon.png");
		m_AudioIconID = Util::GetTextureID(m_TmpTextures->GetDescriptorImageInfo());		
		
		m_TmpTextures = Util::CreateIcon("ttf_icon.png");
		m_FontIconID = Util::GetTextureID(m_TmpTextures->GetDescriptorImageInfo());		
		
		m_TmpTextures = Util::CreateIcon("meta_icon.png");
		m_MetaIconID = Util::GetTextureID(m_TmpTextures->GetDescriptorImageInfo());		
		
		m_TmpTextures = Util::CreateIcon("mat_icon.png");
		m_MaterialIconID = Util::GetTextureID(m_TmpTextures->GetDescriptorImageInfo());		
		
		m_TmpTextures = Util::CreateIcon("fbx_icon.png");
		m_3DObjIconID = Util::GetTextureID(m_TmpTextures->GetDescriptorImageInfo());	
		
		m_TmpTextures = Util::CreateIcon("cs_icon.png");
		m_CSScriptIconID = Util::GetTextureID(m_TmpTextures->GetDescriptorImageInfo());
#endif
	}
	
	void ContentBrowserPanel::Update()
	{
		if (m_CurrentTimer <= m_RefreshRate)
		{
			m_CurrentTimer += Engine::GetInstance().GetWindow()->GetDeltaTime();
		}
		else
		{
			m_CurrentTimer = 0.f;
			PollItems();
		}

		if (ImGui::Begin("Content Browser", nullptr, ImGuiWindowFlags_NoCollapse))
		{
			BrowseProjectFiles();
		}
		ImGui::End();
	}

	void ContentBrowserPanel::Shutdown()
	{
		m_TmpTextures.reset();
	}
}