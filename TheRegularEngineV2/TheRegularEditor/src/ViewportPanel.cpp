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
		m_MousePos.x = static_cast<float>(event._xpos);
		m_MousePos.y = static_cast<float>(event._ypos);

		//Center mouse to the middle of Window
		m_MousePos.x = m_MousePos.x - Engine::GetInstance().GetWindow()->GetWindowConfig().width / 2.f;
		m_MousePos.y = m_MousePos.y - Engine::GetInstance().GetWindow()->GetWindowConfig().height / 2.f;
		m_MousePos.y = -m_MousePos.y;

		if (m_IsViewportFocused == false)
			return;

		Entity entity = ECSSystemManager::Instance().GetSystem<CameraSystem>()->GetMainCamera();
		const Camera& camera = entity->GetComponent<Camera>();
		if (ImGui::IsMouseClicked(ImGuiMouseButton_Middle, true))
		{
			m_MouseEndPos = m_MousePos;
			glm::vec2 positionOffset = m_MouseEndPos - m_MouseStartPos;
			positionOffset.x *= -1;
			positionOffset *= m_PanSensitivity;
			positionOffset *= Engine::GetInstance().GetWindow()->GetDeltaTime();

			ECSSystemManager::Instance().GetSystem<CameraSystem>()->SetPosition(entity, camera.m_Position + camera.m_RightVec * positionOffset.x);
			ECSSystemManager::Instance().GetSystem<CameraSystem>()->SetPosition(entity, camera.m_Position + camera.m_UpVec * positionOffset.y);
		}
		else
		{
			m_MouseStartPos = m_MousePos;
		}

		if (ImGui::IsMouseClicked(ImGuiMouseButton_Right, true))
		{
			m_MouseEndPos = m_MousePos;
			glm::vec2 rotationOffset = m_MouseEndPos - m_MouseStartPos;
			rotationOffset = glm::normalize(rotationOffset);
			rotationOffset.x *= -1;
			rotationOffset *= m_RotationSensitivity;
			rotationOffset *= Engine::GetInstance().GetWindow()->GetDeltaTime();

			ECSSystemManager::Instance().GetSystem<CameraSystem>()->SetRotation(entity, camera.m_Rotation + camera.m_RightVec * rotationOffset.y);
			ECSSystemManager::Instance().GetSystem<CameraSystem>()->SetRotation(entity, camera.m_Rotation + camera.m_UpVec * rotationOffset.x);
		}
		else
		{
			//m_MouseStartPos = m_MousePos;
		}
	}

	void ViewportPanel::OnMouseClick(const InputEvent& event)
	{
		if ((event._key == (int)KeyButton::mouseButtonMiddle || event._key == (int)KeyButton::mouseButtonRight) && event._state == (int)KeyState::keyPressed)
		{
			m_IsViewportFocused = m_IsViewportHovered;
		}
		else if ((event._key == (int)KeyButton::mouseButtonMiddle || event._key == (int)KeyButton::mouseButtonRight) && event._state == (int)KeyState::keyHeld)
		{
		}
		else if ((event._key == (int)KeyButton::mouseButtonMiddle || event._key == (int)KeyButton::mouseButtonRight) && event._state == (int)KeyState::keyReleased)
		{
			m_IsViewportFocused = false;
		}
	}

	void ViewportPanel::OnMouseScroll(const MouseScrollEvent& event)
	{
		if(m_IsViewportHovered == false)
			return;

		Entity entity = ECSSystemManager::Instance().GetSystem<CameraSystem>()->GetMainCamera();
		const Camera& camera = entity.get()->GetComponent<Camera>();
		CameraSystem* cameraSystem = ECSSystemManager::Instance().GetSystem<CameraSystem>();
		glm::vec3 scrollOffset = camera.m_ForwardVec;
		scrollOffset *= m_ZoomSensitivity;

		if (event._yoffset > 0)
		{	
			cameraSystem->SetPosition(entity, camera.m_Position + scrollOffset);
		}
		else if (event._yoffset < 0)
		{
			cameraSystem->SetPosition(entity, camera.m_Position - scrollOffset);
		}
	}

	void ViewportPanel::Init()
	{
		EventHandler::getEventHandlerInstance().subscribe(this, &ViewportPanel::OnMouseMove);
		EventHandler::getEventHandlerInstance().subscribe(this, &ViewportPanel::OnMouseClick);
		EventHandler::getEventHandlerInstance().subscribe(this, &ViewportPanel::OnMouseScroll);
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