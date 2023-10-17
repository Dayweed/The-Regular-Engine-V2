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
				if (ImGui::BeginMenu("Grid and Snap"))
				{
					ImGui::MenuItem("Increament Snapping");
					ImGui::InputFloat("Position", &m_PosIncreament);
					ImGui::InputFloat("Rotation", &m_RotIncreament);
					ImGui::InputFloat("Scale", &m_ScaleIncreament);
					EventHandler::getEventHandlerInstance().Publish(GridAndSnapEvent{ m_PosIncreament, m_RotIncreament, m_ScaleIncreament });
					ImGui::EndMenu();
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
		// Only save and load when it is not running
		if (!GameLoop::Instance().IsGameRunning())
		{
			SceneManager::Instance().NewScene();
			EventHandler::getEventHandlerInstance().Publish(ConsoleDebugEvent{ "New Scene Created" });
		}
	}

	void MenuBarPanel::OpenScene()
	{
		// Only save and load when it is not running
		if (!GameLoop::Instance().IsGameRunning())
		{
			//To do
			//SceneManager::Instance().LoadScene();
			const std::string path = FileExplorer::OpenFileExplorer("Scene(*.json)\0*.json\0");
			if (!path.empty())
			{
				SceneManager::Instance().LoadScene(path);
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