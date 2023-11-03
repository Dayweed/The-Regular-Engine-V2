#include "pch.h"
#include "MenuBarPanel.h"
#include "Imgui/imgui.h"
#include "EventSystem/EventHandler/EventHandler.h"
#include <EventSystem/Events/EditorEvent.h>
#include "Core/Engine.h"
#include "Core/GameLoop.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "Imgui/imgui_internal.h"
#include "Imgui/imgui.h"
#include "EditorSystem.h"
#include "Graphics/EditorCamera.h"

namespace TRE
{
	MenuBarPanel::MenuBarPanel()
	{
		// Custom flag combinations
		m_PopUps |= ImGuiWindowFlags_NoResize;
		m_PopUps |= ImGuiWindowFlags_NoCollapse;
		m_PopUps |= ImGuiWindowFlags_NoMove;
		m_PopUps |= ImGuiWindowFlags_NoSavedSettings;
		m_PopUps |= ImGuiWindowFlags_AlwaysAutoResize;
	}

	MenuBarPanel::~MenuBarPanel()
	{

	}

	void MenuBarPanel::Init()
	{
		EventHandler::getEventHandlerInstance().subscribe(this, &MenuBarPanel::HandleShortcuts);
	}

	void MenuBarPanel::Update()
	{
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("Files"))
			{
				if (ImGui::MenuItem("New", "Ctrl+N"))
				{
					NewScene();
				}
				if (ImGui::MenuItem("Open", "Ctrl+O"))
				{
					OpenScene();
				}
				if (ImGui::MenuItem("Save", "Ctrl+S"))
				{
					SaveScene();
				}

				ImGui::Separator();
				if (ImGui::MenuItem("Exit"))
				{
					m_ExitPrompt = true;
				}

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Edit"))
			{
				if (ImGui::MenuItem("Undo", "Ctrl+Z"))
				{
					//to do
					EventHandler::getEventHandlerInstance().Publish(TypingEvent{ static_cast<int>(KeyButton::Z), static_cast<int>(KeyMods::CONTROL) });
				}
				if (ImGui::MenuItem("Redo", "Ctrl+Y"))
				{
					//to do
					EventHandler::getEventHandlerInstance().Publish(TypingEvent{ static_cast<int>(KeyButton::Y), static_cast<int>(KeyMods::CONTROL) });
				}
				ImGui::EndMenu();
			}


			if (ImGui::BeginMenu("Windows"))
			{
				ImGui::PushItemFlag(ImGuiItemFlags_SelectableDontClosePopup, true);
				ImGui::MenuItem("Hierarchy", nullptr, &m_ShowHierarchyPanel);
				ImGui::MenuItem("Inspector", nullptr, &m_ShowInspectorPanel);
				ImGui::MenuItem("Tool Bar", nullptr, &m_ShowToolBarPanel);
				ImGui::MenuItem("Scene", nullptr, &m_ShowScenePanel);
				ImGui::MenuItem("Game", nullptr, &m_ShowGamePanel);
				ImGui::MenuItem("Console", nullptr, &m_ShowConsolePanel);
				ImGui::MenuItem("Asset", nullptr, &m_ShowAssetPanel);
				ImGui::PopItemFlag();
				ImGui::EndMenu();
			}
			//Commented out till I find a use for it
			if (ImGui::BeginMenu("Options"))
			{
				if (ImGui::BeginMenu("Gizmo"))
				{
					if (ImGui::Checkbox("Local", &m_LocalGizmo))
					{
						EventHandler::getEventHandlerInstance().Publish(LocalGloalGizmoEvent{ m_LocalGizmo });
					}
					if (ImGui::BeginMenu("Grid and Snap"))
					{
						ImGui::MenuItem("Increment Snapping");
						ImGui::Text("Position");
						ImGui::SameLine();
						ImGui::InputFloat("##SnapPosition", &m_PosIncrement);
						ImGui::Text("Rotation");
						ImGui::SameLine();
						ImGui::InputFloat("##SnapRotation", &m_RotIncrement);
						ImGui::Text("Scale");
						ImGui::SameLine();
						ImGui::InputFloat("##SnapScale", &m_ScaleIncrement);
						EventHandler::getEventHandlerInstance().Publish(GridAndSnapEvent{ m_PosIncrement, m_RotIncrement, m_ScaleIncrement });
						ImGui::EndMenu();
					}

					ImGui::EndMenu();
				}
				
				if (ImGui::BeginMenu("Editor Camera"))
				{
					if (ImGui::BeginMenu("Sensitivity"))
					{
						ImGui::Text("Pan");
						ImGui::SameLine();
						ImGui::InputFloat("##EditorPan", &m_PosIncrement);
						ImGui::Text("Rotation");
						ImGui::SameLine();
						ImGui::InputFloat("##EditorRotation", &m_RotIncrement);
						ImGui::Text("Zoom");
						ImGui::SameLine();
						ImGui::InputFloat("##EditorZoom", &m_ScaleIncrement);
						//EventHandler::getEventHandlerInstance().Publish(GridAndSnapEvent{ m_PosIncrement, m_RotIncrement, m_ScaleIncrement });
						ImGui::EndMenu();
					}

					if (ImGui::Button("Assign Editor Camera Values"))
					{
						EditorCamera::Instance().AssignToMainCamera();
					}
					ImGui::EndMenu();
				}

				if (ImGui::Checkbox("Show All Colliders", &m_ShowAllColliders))
				{
					for (Entity& entity : ECSManager::Instance().GetEntities<SphereCollider>())
						entity->GetComponent<SphereCollider>().m_IsVisible = m_ShowAllColliders;

					for (Entity& entity : ECSManager::Instance().GetEntities<BoxCollider>())
						entity->GetComponent<BoxCollider>().m_IsVisible = m_ShowAllColliders;

					for (Entity& entity : ECSManager::Instance().GetEntities<CapsuleCollider>())
						entity->GetComponent<CapsuleCollider>().m_IsVisible = m_ShowAllColliders;

					ECSSystemManager::Instance().GetSystem<PhysicsSystem>()->SetDrawDebug(m_ShowAllColliders);
				}

				ImGui::EndMenu();
			}

			ImGui::EndMainMenuBar();
		}

		if (m_ExitPrompt)
		{
			ImGui::OpenPopup("Exit");
			m_ExitPrompt = false;
		}

		ImGui::SetNextWindowSize(ImVec2(400, 100));
		ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
		if (ImGui::BeginPopupModal("Exit", nullptr, m_PopUps))
		{
			ImGui::Text("Are you sure? Please remember to save before quitting!");
			if (ImGui::Button("Yes"))
				Engine::GetInstance().Shutdown();
			if (ImGui::Button("No"))
				ImGui::CloseCurrentPopup();
			ImGui::EndPopup();
		}

		if (m_ShortcutNewScene)
		{
			NewScene();
			m_ShortcutNewScene = false;
		}
		if (m_ShortcutOpenScene)
		{
			OpenScene();
			m_ShortcutOpenScene = false;
		}
		if (m_ShortcutSaveScene)
		{
			SaveScene();
			m_ShortcutSaveScene = false;
		}
	}

	void MenuBarPanel::Shutdown()
	{

	}

	void MenuBarPanel::NewScene()
	{
		EditorSystemManager::Instance().GetSystem<EditorSystem>()->GetSelectionManager()->ClearSelectedEntity();

		// Only save and load when it is not running
		if (!GameLoop::Instance().IsGameRunning())
		{
			SceneManager::Instance().NewScene();
			EventHandler::getEventHandlerInstance().Publish(ConsoleDebugEvent{ "New Scene Created" });
		}
	}

	void MenuBarPanel::OpenScene()
	{
		EditorSystemManager::Instance().GetSystem<EditorSystem>()->GetSelectionManager()->ClearSelectedEntity();

		// Only save and load when it is not running
		if (!GameLoop::Instance().IsGameRunning())
		{
			//To do
			//SceneManager::Instance().LoadScene();
			const std::string path = FileExplorer::OpenFileExplorer("Scene(*.json)\0*.json\0");
			if (!path.empty())
			{
				SceneManager::Instance().LoadScene(path);

				EditorCamera::Instance().Deserialize();
			}
		}
	}

	void MenuBarPanel::SaveScene()
	{
		// Only save and load when it is not running
		if (!GameLoop::Instance().IsGameRunning())
		{
			const std::string path = FileExplorer::SaveFileExplorer("Scene(*.json)\0*.json\0");
			if (!path.empty())
			{
				SceneManager::Instance().SaveSceneAs(path);
			}
		}
		return;
	}

	void MenuBarPanel::HandleShortcuts(TypingEvent& event)
	{
		const KeyButton key = static_cast<KeyButton>(event.m_Key);
		const KeyMods mods = static_cast<KeyMods>(event.m_Mod);

		if (mods == KeyMods::CONTROL || mods == KeyMods::NUMLOCK_CONTROL)
		{
			m_ShortcutNewScene	= key == KeyButton::N;
			m_ShortcutOpenScene = key == KeyButton::O;
			m_ShortcutSaveScene = key == KeyButton::S;
		}
	}
}