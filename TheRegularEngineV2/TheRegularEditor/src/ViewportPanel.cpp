#include "ViewportPanel.h"
#include "Imgui/imgui.h"
#include "EditorCamera.h"

//To Delete
#include "Graphics/Camera.h"

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

		//Editor Camera
		if (ImGui::IsWindowFocused())
		{
			if (ImGui::IsMouseClicked(ImGuiMouseButton_Left, true))
			{

				//EditorCamera::Instance().SetPosition();
			}

			if (ImGui::IsMouseClicked(ImGuiMouseButton_Left, true))
			{
				Entity entity = ECSSystemManager::Instance().GetSystem<CameraSystem>()->GetMainCamera();
				Camera& camera = entity.get()->GetComponent<Camera>();
				ECSSystemManager::Instance().GetSystem<CameraSystem>()->SetPosition(entity, camera.m_Position += 0.1f);
				//EditorCamera::Instance().SetPosition();
			}

			//EditorCamera::Instance().Update();
		}

		ImGui::End();
	}

	void ViewportPanel::Shutdown()
	{

	}
}