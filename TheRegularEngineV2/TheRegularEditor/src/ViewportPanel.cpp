#include "pch.h"
#include "ViewportPanel.h"
#include "EditorCamera.h"
#include "EventSystem/EventHandler/EventHandler.h"
#include "Editor/ImGuizmo.h"
#include "Ray3D.h"
#include "Utilities.h"

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
				positionOffset = glm::normalize(positionOffset);
				positionOffset *= -1;
				const auto panSensitivity = PanSensitivity(m_ImageSize.x, m_ImageSize.y);
				positionOffset.x *= panSensitivity.x;
				positionOffset.y *= panSensitivity.y;
				positionOffset *= m_PanSpeed * camera.m_FocalLength / 10.f;
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
				Entity entity = ECSSystemManager::Instance().GetSystem<CameraSystem>()->GetMainCamera();
				const Camera& camera = entity->GetComponent<Camera>();
				CameraSystem* cameraSystem = ECSSystemManager::Instance().GetSystem<CameraSystem>();

				ImVec2 worldSpaceMousePos = ImGui::GetMousePos();
				worldSpaceMousePos -= m_WindowPos;
				worldSpaceMousePos -= m_ImageOffset;
				worldSpaceMousePos.x /= m_ImageSize.x;
				worldSpaceMousePos.x -= 0.5f;
				worldSpaceMousePos.x *= 2.f;
				worldSpaceMousePos.y /= m_ImageSize.y;
				worldSpaceMousePos.y = 1.f - worldSpaceMousePos.y;
				worldSpaceMousePos.y -= 0.5f;
				worldSpaceMousePos.y *= 2.f;

				glm::mat4 invProjView = cameraSystem->GetInverseViewProjectionMatrix(entity);
				glm::vec4 start = glm::vec4(worldSpaceMousePos.x, -worldSpaceMousePos.y, 0.f, 1.f);
				glm::vec4 end = glm::vec4(worldSpaceMousePos.x, -worldSpaceMousePos.y, 1.f, 1.f);
				start = invProjView * start;
				end = invProjView * end;
				start.x /= start.w;
				start.y /= start.w;
				start.z /= start.w;
				end.x /= end.w;
				end.y /= end.w;
				end.z /= end.w;
				glm::vec3 ray = end - start;
				
				const Collision::Ray3D cameraRay = Collision::Ray3D(camera.m_Position, ray);

				auto ent_mrs = ECSManager::Instance().GetEntities<MeshRenderer>();
				auto meshRendererSystem = ECSSystemManager::Instance().GetSystem<MeshRendererSystem>();
				std::map<float, Entity> entitiesHit;
				for (auto mr : ent_mrs)
				{
					const Collision::Sphere3D& sphere = meshRendererSystem->GetBoundingSphere(mr);
					float t = 0.f;
					if (cameraRay.Collision::Ray3D::Intersects(sphere, &t))
					{
						entitiesHit[t] = mr;
					}
				}

				if (entitiesHit.size() > 0)
				{
					//Single click for now
					m_SelectionManager->SelectEntity(entitiesHit.begin()->second);
				}
				else
				{
					//Clear
					m_SelectionManager->ClearSelectedEntity();
				}
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

	void ViewportPanel::OnKeyboardClick(const InputEvent& event)
	{
		if (event._key == (int)KeyButton::Q)
		{
			m_GizmoOperation = ImGuizmo::OPERATION::SCALE;
		}
		if (event._key == (int)KeyButton::W)
		{
			m_GizmoOperation = ImGuizmo::OPERATION::ROTATE;
		}
		if (event._key == (int)KeyButton::E)
		{
			m_GizmoOperation = ImGuizmo::OPERATION::TRANSLATE;
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
			cameraSystem->SetFocalPoint(entity, camera.m_FocalPoint + camera.GetViewDirection());
			cameraSystem->SetFocalLength(entity, 1.f);
		}
	}

	void ViewportPanel::Init()
	{
		EventHandler::getEventHandlerInstance().subscribe(this, &ViewportPanel::OnMouseMove);
		EventHandler::getEventHandlerInstance().subscribe(this, &ViewportPanel::OnMouseClick);
		EventHandler::getEventHandlerInstance().subscribe(this, &ViewportPanel::OnMouseScroll);
		EventHandler::getEventHandlerInstance().subscribe(this, &ViewportPanel::OnKeyboardClick);
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

		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Content Browser item"))
			{
				std::string assetName = (const char*)payload->Data;
				std::cout << "drag and dropped " << assetName << " from Content Browser Panel\n";
			}

			ImGui::EndDragDropTarget();
		}

		Entity SelectedEntity = m_SelectionManager->GetSelectedEntity();
		if (SelectedEntity && m_GizmoOperation != -1)
		{
			ImGuizmo::SetOrthographic(true);
			ImGuizmo::SetDrawlist();

			float WindowWith = (float)ImGui::GetWindowWidth();
			float WindowHeight = (float)ImGui::GetWindowHeight();
			ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, WindowWith, WindowHeight);

			Entity entity = ECSSystemManager::Instance().GetSystem<CameraSystem>()->GetMainCamera();
			const Camera& camera = entity->GetComponent<Camera>();
			CameraSystem* cameraSystem = ECSSystemManager::Instance().GetSystem<CameraSystem>();
			glm::mat4 proj = cameraSystem->GetProjectionMatrix(entity);
			proj[1][1] *= -1.f;
			glm::mat4 View = cameraSystem->GetViewMatrix(entity);

			glm::mat4 xform = SelectedEntity->GetComponent<Transform>().GetModelMatrix();

			TransformSystem* XformSystem = ECSSystemManager::Instance().GetSystem<TransformSystem>();

			ImGuizmo::Manipulate(glm::value_ptr(View), glm::value_ptr(proj), (ImGuizmo::OPERATION)m_GizmoOperation, ImGuizmo::WORLD, glm::value_ptr(xform));
			
			if (ImGuizmo::IsUsing())
			{
				glm::vec3 Scale;
				glm::vec3 Rotation;
				glm::vec3 Translate;
				ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(xform), glm::value_ptr(Translate), glm::value_ptr(Rotation), glm::value_ptr(Scale));
				switch (m_GizmoOperation)
				{
					case ImGuizmo::OPERATION::SCALE:
					{
						XformSystem->SetScale(SelectedEntity, Scale);
						break;
					}
					case ImGuizmo::OPERATION::ROTATE:
					{
						XformSystem->SetRotation(SelectedEntity, Rotation);
						break;
					}
					case ImGuizmo::OPERATION::TRANSLATE:
					{
						XformSystem->SetPosition(SelectedEntity, Translate);
						break;
					}
				}
				
			}
		}

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