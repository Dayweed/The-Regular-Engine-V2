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
		ImGui::Begin("Viewport");

		ImVec2 ViewportSize = ImGui::GetContentRegionAvail();
		ImGui::Image(VulkanEditor::GetFinalImage(), ViewportSize, ImVec2(0, 1), ImVec2(1, 0));

		ImGui::End();
		ImGui::ShowDemoWindow();
	}

	void EditorSystem::Shutdown()
	{
		std::cout << "Editor Shutdown" << std::endl; //Replace with Logging
	}
}