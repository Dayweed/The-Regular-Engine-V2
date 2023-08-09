#include "EditorSystem.h"
#include "Imgui/imgui.h"

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
		ImGui::Begin("Test Window");
		ImGui::End();
		ImGui::ShowDemoWindow();
	}

	void EditorSystem::RenderImgui()
	{

	}

	void EditorSystem::Shutdown()
	{
		std::cout << "Editor Shutdown" << std::endl; //Replace with Logging
	}
}