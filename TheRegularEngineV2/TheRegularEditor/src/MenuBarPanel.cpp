#include "pch.h"
#include "MenuBarPanel.h"
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
				}
				if (ImGui::MenuItem("Redo", "Ctrl+Y"))
				{
					//to do
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

			if (ImGui::BeginMenu("Options"))
			{
				ImGui::EndMenu();
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
					TRE_CORE_INFO("Closing window (fake)");
				if (ImGui::Button("No"))
					ImGui::CloseCurrentPopup();
				ImGui::EndPopup();
			}

			ImGui::EndMainMenuBar();
		}
	}

	void MenuBarPanel::Shutdown()
	{

	}

	void MenuBarPanel::NewScene()
	{
		return;
	}

	void MenuBarPanel::OpenScene()
	{
		return;
	}

	void MenuBarPanel::SaveScene()
	{
		return;
	}
}