#include "ViewportPanel.h"
#include "Imgui/imgui.h"
#include "EditorCamera.h"
#include "Core/Engine.h"
#include "EventSystem/EventHandler/EventHandler.h"


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

	void ViewportPanel::OnMouseMove(const MouseMoveEvent& event)
	{
		m_MousePos.x = event._xpos;
		m_MousePos.y = event._ypos;

		//Center mouse to the middle of Window
		m_MousePos.x = m_MousePos.x - Engine::GetInstance().GetWindow()->GetWindowConfig().width / 2.f;
		m_MousePos.y = m_MousePos.y - Engine::GetInstance().GetWindow()->GetWindowConfig().height / 2.f;
		m_MousePos.y = -m_MousePos.y;

		if (m_IsViewportHovered == false)
			return;
		if (ImGui::IsMouseClicked(ImGuiMouseButton_Right, true))
		{
			Entity entity = ECSSystemManager::Instance().GetSystem<CameraSystem>()->GetMainCamera();
			const Camera& camera = entity.get()->GetComponent<Camera>();

			m_MouseEndPos = m_MousePos;
			glm::vec2 offset = m_MouseEndPos - m_MouseStartPos;
			offset *= Engine::GetInstance().GetWindow()->GetDeltaTime();

			ECSSystemManager::Instance().GetSystem<CameraSystem>()->SetPosition(entity, camera.m_Position + glm::vec3(offset.x, offset.y, 0));
		}
		else
		{
			m_MouseStartPos = m_MousePos;
		}
	}

	void ViewportPanel::OnMouseClick(const InputEvent& event)
	{
		if (event._key == (int)KeyButton::mouseButtonRight && event._state == (int)KeyState::keyHeld)
		{
			std::cout << "hold\n";
		}
		else if (event._key == (int)KeyButton::mouseButtonRight && event._state == (int)KeyState::keyReleased)
		{
			std::cout << "release\n";
		}
	}

	void ViewportPanel::Init()
	{
		EventHandler::getEventHandlerInstance().subscribe(this, &ViewportPanel::OnMouseMove);
		EventHandler::getEventHandlerInstance().subscribe(this, &ViewportPanel::OnMouseClick);
	}

	void ViewportPanel::Update()
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("Viewport");
		ImGui::PopStyleVar();

		ImVec2 ViewportSize = ImGui::GetContentRegionAvail();
		ImGui::Image(Engine::GetInstance().GetVulkanImgui()->GetDset(), ViewportSize);

		m_IsViewportHovered = ImGui::IsWindowHovered();

		ImGui::End();
	}

	void ViewportPanel::Shutdown()
	{

	}
}