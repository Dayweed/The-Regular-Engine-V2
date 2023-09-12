#include "pch.h"
#include "ToolBarPanel.h"
#include "Imgui/imgui.h"

namespace TRE
{
	ToolBarPanel::ToolBarPanel()
	{

	}

	ToolBarPanel::~ToolBarPanel()
	{

	}

	void ToolBarPanel::Init()
	{

	}

	void ToolBarPanel::Update()
	{
		ImGui::Begin("Tool Bar");
		//ImGui::Text("Testing where the tool bar is");
		//How to pad in imgui
		ImGui::SameLine(ImGui::GetContentRegionAvail().x/2 - 25);

		//if (ImGui::ImageButton(nullptr, ImVec2(20, 20), ImVec2(0, 0), ImVec2(0, 0), 0))
		//{
		//	TRE_CORE_INFO("Play Button Pressed");
		//}

		//ImGui::SameLine(ImGui::GetContentRegionAvail().x / 2);

		//if (ImGui::ImageButton(nullptr, ImVec2(20, 20), ImVec2(0, 0), ImVec2(0, 0), 0))
		//{
		//	TRE_CORE_INFO("Button Pressed");
		//}

		//ImGui::SameLine(ImGui::GetContentRegionAvail().x / 2 + 25);

		//if (ImGui::ImageButton(nullptr, ImVec2(20, 20), ImVec2(0, 0), ImVec2(0, 0), 0))
		//{
		//	TRE_CORE_INFO("Work Button Pressed");
		//}

		ImGui::End();
	}

	void ToolBarPanel::Shutdown()
	{

	}
}