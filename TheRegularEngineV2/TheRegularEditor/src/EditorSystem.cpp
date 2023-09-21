#include "pch.h"
#include "EditorSystem.h"
#include "Imgui/imgui.h"
#include "imgui_impl_vulkan.h"
#include "TREIncludes.h"
#include "SceneHierarchyPanel.h"
#include "ViewportPanel.h"
#include "MenuBarPanel.h"
#include "InspectorPanel.h"
#include "ContentBrowserPanel.h"
#include "ConsolePanel.h"
#include "ProfilerPanel.h"
#include "ToolBarPanel.h"
#include "EditorAssetManager.h"

namespace TRE
{
	EditorSystem::EditorSystem()
	{
		TRE_INFO("Editor Init");

		m_PanelManager = std::make_unique<PanelManager>();
		m_SelectionManager = std::make_shared<SelectionManager>();

		m_PanelManager->InsertPanel<SceneHierarchyPanel>("Scene Hierarchy", m_SelectionManager);
		m_PanelManager->InsertPanel<ViewportPanel>("Viewport", m_SelectionManager);
		m_PanelManager->InsertPanel<MenuBarPanel>("Menu Bar");
		m_PanelManager->InsertPanel<InspectorPanel>("Inspector", m_SelectionManager);
		m_PanelManager->InsertPanel<ContentBrowserPanel>("Content Browser", m_SelectionManager);
		m_PanelManager->InsertPanel<ConsolePanel>("Console");
		m_PanelManager->InsertPanel<ProfilerPanel>("Profiler");
		m_PanelManager->InsertPanel<ToolBarPanel>("Tool Bar");
		m_PanelManager->Init();

		EditorAssetManager::Instance().Initialize();
	}
	
	EditorSystem::~EditorSystem()
	{

	}

	void EditorSystem::RenderImgui()
	{

	}

	void EditorSystem::Update()
	{
		static bool OpenDockSpace = true;
		static bool opt_fullscreen = true;
		static bool opt_padding = false;
		static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking; //Prevent Parent Window to be dockable
		//	ENGINE_INFO("TESTS");
		if (opt_fullscreen)
		{
			const ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(viewport->WorkPos);
			ImGui::SetNextWindowSize(viewport->WorkSize);
			ImGui::SetNextWindowViewport(viewport->ID);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
			window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
		}

		// When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
		// and handle the pass-thru hole, so we ask Begin() to not render a background.
		if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
			window_flags |= ImGuiWindowFlags_NoBackground;

		// Important: note that we proceed even if Begin() returns false (aka window is collapsed).
		// This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
		// all active windows docked into it will lose their parent and become undocked.
		// We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
		// any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("DockSpace", &OpenDockSpace, window_flags); //Dockspace
		ImGui::PopStyleVar();

		if (opt_fullscreen)
			ImGui::PopStyleVar(2);

		// Submit the DockSpace
		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}

		m_PanelManager->Update();

		//ImGui::ShowDemoWindow();
		ImGui::End(); //Dockspace
	}

	void EditorSystem::Shutdown()
	{
		TRE_INFO("Editor Shutdown");
	}
}