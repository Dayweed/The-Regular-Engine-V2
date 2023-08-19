#include "EditorSystem.h"
#include "Imgui/imgui.h"
#include "imgui_impl_vulkan.h"

namespace TRE
{
	EditorSystem::EditorSystem()
	{
		std::cout << "Editor Init" << std::endl; //Replace with Logging
	}
	
	EditorSystem::~EditorSystem()
	{

	}

	void EditorSystem::Update()
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("Viewport");
		ImGui::PopStyleVar();

		ImVec2 ViewportSize = ImGui::GetContentRegionAvail();
		ImGui::Image(Engine::GetInstance().GetVulkanImgui()->GetDset(), ViewportSize);

		ImGui::End();
		ImGui::ShowDemoWindow();
	}

	void EditorSystem::Shutdown()
	{
		std::cout << "Editor Shutdown" << std::endl; //Replace with Logging
	}
}