#include "pch.h"
#include "ViewportPanel.h"
#include "EditorCamera.h"
#include "EventSystem/EventHandler/EventHandler.h"
#include "Editor/ImGuizmo.h"
#include "Ray3D.h"

//To Delete
#include "Graphics/Camera.h"
namespace TRE
{
	ViewportPanel::ViewportPanel(const std::shared_ptr<SelectionManager>& selection_Manager)
	{
		m_SelectionManager = selection_Manager;
	}

	ViewportPanel::~ViewportPanel()
	{

	}

	void ViewportPanel::OnMouseMove(const MouseMoveEvent& event)
	{
		m_MousePos.x = static_cast<float>(event._xpos);
		m_MousePos.y = static_cast<float>(event._ypos);

		//Center mouse to the middle of Window
		const WindowConfig& windowConfig = Engine::GetInstance().GetWindow()->GetWindowConfig();
		m_MousePos.x = m_MousePos.x - windowConfig.width / 2.f;
		m_MousePos.y = m_MousePos.y - windowConfig.height / 2.f;
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
				//positionOffset.x *= -1;
				positionOffset = glm::normalize(positionOffset);
				const auto panSensitivity = PanSensitivity(m_ImageSize.x, m_ImageSize.y);
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
				rotationOffset.x *= -1;
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
		if (m_IsViewportHovered == false)
			return;

		if((event._key != (int)KeyButton::mouseButtonLeft) 
			&& (event._key != (int)KeyButton::mouseButtonMiddle) 
			&& (event._key != (int)KeyButton::mouseButtonRight))
			return;

		if (event._state == (int)KeyState::keyPressed)
		{
			m_IsViewportFocused = m_IsViewportHovered;
			if ((event._key == (int)KeyButton::mouseButtonLeft))
			{
				//Object picking
				//Offset mouse position to the middle of the viewport as if in game
				const WindowConfig& windowConfig = Engine::GetInstance().GetWindow()->GetWindowConfig();

				ImVec2 worldSpaceMousePos = ImGui::GetMousePos();
				worldSpaceMousePos -= m_WindowPos;
				worldSpaceMousePos -= m_ImageOffset;
				worldSpaceMousePos.x /= m_ImageSize.x;
				worldSpaceMousePos.x -= 0.5f;
				worldSpaceMousePos.y /= m_ImageSize.y;
				worldSpaceMousePos.y = 1.f - worldSpaceMousePos.y;
				worldSpaceMousePos.y -= 0.5f;
				worldSpaceMousePos.x *= windowConfig.width;
				worldSpaceMousePos.y *= windowConfig.height;
				const glm::vec3 mousePos = { worldSpaceMousePos.x, worldSpaceMousePos.y, 0.f };

				Entity entity = ECSSystemManager::Instance().GetSystem<CameraSystem>()->GetMainCamera();
				const Camera& camera = entity->GetComponent<Camera>();
				const Collision::Ray3D cameraRay = Collision::Ray3D(camera.m_Position + mousePos, camera.GetForwardVec());
				//std::cout <<"ray position " << cameraRay.GetOrigin().x << " " << cameraRay.GetOrigin().y << " " << cameraRay.GetOrigin().z << std::endl;

				//Entity test2 = ECSManager::Instance().CreateEntity();
				//test2->GetComponent<Properties>().m_Name = "ray";
				//test2->GetComponent<Transform>().m_Position = cameraRay.GetOrigin() + cameraRay.GetDirection() * 5.f;
				////std::cout << "ray position " << test2->GetComponent<Transform>().m_Position.x << " " << test2->GetComponent<Transform>().m_Position.y << " " << test2->GetComponent<Transform>().m_Position.z << std::endl;
				//test2->GetComponent<Transform>().m_Scale = glm::vec3(100.f, 100.f, 100.f);
				//test2->GetComponent<Transform>().m_Rotation = glm::vec3(0.f, 0.f, 0.f);
				//test2->AddComponent<MeshRenderer>();
				//auto geom = Geom::Deserialize("../Assets/smooth_vase.geom");
				//std::shared_ptr<RenderObject> vase = RenderObject::CreateFromGeom(std::move(geom));
				//test2->GetComponent<MeshRenderer>().m_RenderObject = vase;
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
		const float zoomSpeed = static_cast<float>(event._yoffset) * m_ZoomSensitivity * Engine::GetInstance().GetWindow()->GetDeltaTime();

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

		m_IsViewportHovered = ImGui::IsWindowHovered();
		m_ImageSize = m_ViewportSize = ImGui::GetContentRegionAvail();
		m_WindowPos = ImGui::GetWindowPos();
		//Window resize -- force to follow 16:9 aspect ratio
		UpdateViewportSize();
		ImGui::Image(Engine::GetInstance().GetVulkanImgui()->GetDset(), m_ImageSize);

		ImGui::End();
	}

	void ViewportPanel::Shutdown()
	{

	}

	void ViewportPanel::UpdateViewportSize()
	{
		const WindowConfig& windowConfig = Engine::GetInstance().GetWindow()->GetWindowConfig();
		const float aspectRatio = static_cast<float>(windowConfig.width) / windowConfig.height;
		if ((m_ViewportSize.x / m_ViewportSize.y) < aspectRatio)
		{
			m_ImageSize.x = m_ViewportSize.x;
			m_ImageSize.y = m_ViewportSize.x / aspectRatio;
		}
		else
		{
			m_ImageSize.y = m_ViewportSize.y;
			m_ImageSize.x = m_ViewportSize.y * aspectRatio;

		}
		//Center the image
		m_ImageOffset = m_ViewportSize - m_ImageSize;
		m_ImageOffset.x *= 0.5f;
		m_ImageOffset.y *= 0.5f;
		m_ImageOffset += ImGui::GetWindowSize() - m_ViewportSize;
		ImGui::SetCursorPos(m_ImageOffset);
	}
}