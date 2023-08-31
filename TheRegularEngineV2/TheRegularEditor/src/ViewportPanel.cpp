#include "ViewportPanel.h"
#include "Imgui/imgui.h"

namespace TRE
{
	ViewportPanel::ViewportPanel()
	{

	}

	ViewportPanel::~ViewportPanel()
	{

	}

	void ViewportPanel::Init()
	{

	}

	void ViewportPanel::Update()
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("Viewport");
		ImGui::PopStyleVar();

		ImVec2 ViewportSize = ImGui::GetContentRegionAvail();
		ImGui::Image(Engine::GetInstance().GetVulkanImgui()->GetDset(), ViewportSize);

		ImGui::End();
	}

	void ViewportPanel::Shutdown()
	{

	}
}