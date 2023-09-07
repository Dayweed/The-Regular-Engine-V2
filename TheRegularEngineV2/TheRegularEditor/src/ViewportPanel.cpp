#include "pch.h"
#include "ViewportPanel.h"
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
		CameraSystem* cameraSystem = ECSSystemManager::Instance().GetSystem<CameraSystem>();
		{
			static glm::vec2 panMouseStartPos{};
			static glm::vec2 panMouseEndPos{};
			if (ImGui::IsMouseClicked(ImGuiMouseButton_Middle, true))
			{
				panMouseEndPos = m_MousePos;
				glm::vec2 positionOffset = panMouseEndPos - panMouseStartPos;
				positionOffset.x *= -1;
				positionOffset = glm::normalize(positionOffset);
				const auto panSensitivity = PanSensitivity(m_ViewportSize.x, m_ViewportSize.y);
				positionOffset.x *= panSensitivity.x;
				positionOffset.y *= panSensitivity.y;
				positionOffset *= m_PanSpeed;
				positionOffset *= Engine::GetInstance().GetWindow()->GetDeltaTime();

				cameraSystem->SetFocalPoint(entity, camera.m_FocalPoint + camera.GetRightVec() * positionOffset.x);
				cameraSystem->SetFocalPoint(entity, camera.m_FocalPoint + camera.GetUpVec() * positionOffset.y);
			}
			else
			{
				panMouseStartPos = m_MousePos;
			}
		}

		{
			static glm::vec2 rotMouseStartPos{};
			static glm::vec2 rotMouseEndPos{};
			if (ImGui::IsMouseClicked(ImGuiMouseButton_Right, true))
			{
				rotMouseEndPos = m_MousePos;
				glm::vec2 rotationOffset = rotMouseEndPos - rotMouseStartPos;
				rotationOffset = glm::normalize(rotationOffset);
				rotationOffset *= -1;
				rotationOffset *= m_RotationSensitivity;
				rotationOffset *= Engine::GetInstance().GetWindow()->GetDeltaTime();

				const float yawSign = camera.GetUpVec().y < 0 ? -1.f : 1.f;
				cameraSystem->SetPitch(entity, camera.m_Pitch + rotationOffset.y);
				cameraSystem->SetYaw(entity, camera.m_Yaw + yawSign * rotationOffset.x);
			}
			else
			{
				rotMouseStartPos = m_MousePos;
			}
		}
	}

	void ViewportPanel::OnMouseClick(const InputEvent& event)
	{
		
		if((event._key != (int)KeyButton::mouseButtonLeft) 
			&& (event._key != (int)KeyButton::mouseButtonMiddle) 
			&& (event._key != (int)KeyButton::mouseButtonRight))
			return;
		if (event._state == (int)KeyState::keyPressed)
		{
			m_IsViewportFocused = m_IsViewportHovered;
			if ((event._key != (int)KeyButton::mouseButtonLeft))
			{
				//Object picking

			}
		}
		else if (event._state == (int)KeyState::keyHeld)
		{
		}
		else if (event._state == (int)KeyState::keyReleased)
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
		const float zoomSpeed = event._yoffset * m_ZoomSensitivity * Engine::GetInstance().GetWindow()->GetDeltaTime();

		cameraSystem->SetFocalLength(entity, camera.m_FocalLength - zoomSpeed);

		if (camera.m_FocalLength < 1.f)
		{
			cameraSystem->SetFocalPoint(entity, camera.m_FocalPoint + camera.GetForwardVec());
			cameraSystem->SetFocalLength(entity, 1.f);
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

		m_ViewportSize = ImGui::GetContentRegionAvail();
		ImGui::Image(Engine::GetInstance().GetVulkanImgui()->GetDset(), m_ViewportSize);

		m_IsViewportHovered = ImGui::IsWindowHovered();

		ImGui::End();
	}

	void ViewportPanel::Shutdown()
	{

	}
}