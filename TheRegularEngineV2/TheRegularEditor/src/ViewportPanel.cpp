#include "pch.h"
#include "ViewportPanel.h"
#include "Graphics/EditorCamera.h"
#include "EventSystem/EventHandler/EventHandler.h"
#include "Editor/ImGuizmo.h"
#include "Ray3D.h"
#include "Utilities.h"
#include "EditorSystem.h"
#include "EditorAssetManager.h"
#include "Graphics/VulkanEditor.h"
#include "Core/Engine.h"
#include "Graphics/RendererContext.h"
#include "ConsolePanel.h"
//To Delete
#include "Scripting/ScriptEngine.h"
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
			if (m_IsViewportFocused && (event._key == (int)KeyButton::mouseButtonLeft) && m_GizmoOperation == -1)
			{
				//Object picking
				//Offset mouse position to the middle of the viewport as if in game
				EditorCamera& editorCamera = EditorCamera::Instance();

				UpdateClickRay();

				const Collision::Ray3D cameraRay = Collision::Ray3D(editorCamera.GetPosition(), m_ClickRay);

				auto ent_mrs = ECSManager::Instance().GetEntities<MeshRenderer>();
				auto meshRendererSystem = ECSSystemManager::Instance().GetSystem<MeshRendererSystem>();
				std::map<float, Entity> entitiesHit;
				for (auto& mr : ent_mrs)
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
	}

	void ViewportPanel::OnKeyboardClick(const InputEvent& event)
	{
		if (m_IsViewportHovered == false)
			return;

		//Gizmo
		if (event._key == (int)KeyButton::Q)
		{
			m_GizmoOperation = -1;
		}
		if (event._key == (int)KeyButton::W)
		{
			m_GizmoOperation = ImGuizmo::OPERATION::TRANSLATE;
		}
		if (event._key == (int)KeyButton::E)
		{
			m_GizmoOperation = ImGuizmo::OPERATION::ROTATE;
		}
		if (event._key == (int)KeyButton::R)
		{
			m_GizmoOperation = ImGuizmo::OPERATION::SCALE;
		}

		//Gizmo snapping
		if ((event._key == (int)KeyButton::LeftControl || event._key == (int)KeyButton::RightControl) && event._state == (int)KeyState::keyHeld)
		{
			m_IsGridAndSnap = true;
		}
		else
			m_IsGridAndSnap = false;

		//Look at
		if (event._key == (int)KeyButton::F)
		{
			if (Entity SelectedEntity = m_SelectionManager->GetSelectedEntity(); SelectedEntity)
			{
				//if(SelectedEntity->HasComponent<MeshRenderer>())
					EditorCamera::Instance().SetDirection(SelectedEntity->GetComponent<Transform>().m_Position);
			}
		}
	}

	void ViewportPanel::OnMouseScroll(const MouseScrollEvent& event)
	{
		if(m_IsViewportHovered == false)
			return;

		EditorCamera& editorCamera = EditorCamera::Instance();
		const BaseCamera& baseCamera = editorCamera.m_BaseCamera;
		const float zoomSpeed = static_cast<float>(event._yoffset) * m_ZoomSensitivity * ImGui::GetIO().DeltaTime;

		editorCamera.SetFocalDistance(baseCamera.m_FocalLength - zoomSpeed);

		if (baseCamera.m_FocalLength < 1.f)
		{
			editorCamera.SetFocalDistance(1.f);
			editorCamera.SetFocalPoint(baseCamera.m_FocalPoint + baseCamera.GetViewDirection());
		}
	}

	void ViewportPanel::OnGridAndSnap(const GridAndSnapEvent& event)
	{
		m_PosIncrement = event.m_PosIncrement;
		m_RotIncrement = event.m_RotIncrement;
		m_ScaleIncrement = event.m_ScaleIncrement;
	}

	void ViewportPanel::OnGizmoLocal(const LocalGloalGizmoEvent& event)
	{
		m_IsGizmoLocal = event.m_IsLocal;
	}

	void ViewportPanel::Init()
	{
		EventHandler::getEventHandlerInstance().subscribe(this, &ViewportPanel::OnMouseMove);
		EventHandler::getEventHandlerInstance().subscribe(this, &ViewportPanel::OnMouseClick);
		EventHandler::getEventHandlerInstance().subscribe(this, &ViewportPanel::OnMouseScroll);
		EventHandler::getEventHandlerInstance().subscribe(this, &ViewportPanel::OnKeyboardClick);
		EventHandler::getEventHandlerInstance().subscribe(this, &ViewportPanel::OnGridAndSnap);
		EventHandler::getEventHandlerInstance().subscribe(this, &ViewportPanel::OnGizmoLocal);
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

		MouseActions();
		ImGui::Image(Engine::GetInstance().GetVulkanImgui()->GetEditorSceneDescriptor(), m_ImageSize, ImVec2(0,0), ImVec2(1, 1));

		if (ImGui::IsWindowHovered())
		{
			if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) || ImGui::IsMouseClicked(ImGuiMouseButton_Right))
				ImGui::SetWindowFocus();
		}

		if (ImGui::BeginDragDropTarget())
		{
			//For 3D Models
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("m_3DObject"))
			{
				std::string assetName = (const char*)payload->Data;
				//assetName = assetName.substr(assetName.find_last_of('\\') + 1);
				//assetName = assetName.substr(0, assetName.find_last_of(".fbx") + 1);

				assetName = assetName.substr(assetName.find_last_of('\\') + 1);
				assetName.erase(assetName.find(".fbx")); 	// This is to remove unneeded data at the end after ".fbx"
				assetName += ".fbx";

				//Spawn object at mouse location
				Entity spawn = ECSManager::Instance().CreateEntity();
				spawn->GetComponent<Properties>().m_Name = assetName.substr(0, assetName.find_last_of('.'));
				Transform& transform{ spawn->GetComponent<Transform>() };

				const EditorCamera& camera = EditorCamera::Instance();
				UpdateClickRay();
				transform.m_Position = camera.GetPosition() + glm::normalize(m_ClickRay) * 40.f;
				transform.m_Scale = glm::vec3(1.f, 1.f, 1.f);
				transform.m_Rotation = glm::vec3(0, 0.f, 0);
				transform.m_IsDirty = true;
				spawn->AddComponent<MeshRenderer>();
				
				//Check if asset is already compiled
				//Compiled before
				if (AssetManager::Instance().Contains(assetName))
				{
					//Load into memory
					if (AssetManager::Instance().GetAsset<RenderObject>(assetName) == nullptr)
					{
						AssetManager::Instance().AddAsset<RenderObject>(assetName);
						ECSSystemManager::Instance().GetSystem<MeshRendererSystem>()->
							SetMeshRenderer(spawn, AssetManager::Instance().GetAsset<RenderObject>(assetName));
					}
					//Assign directly
					else
					{
						ECSSystemManager::Instance().GetSystem<MeshRendererSystem>()->
							SetMeshRenderer(spawn, AssetManager::Instance().GetAsset<RenderObject>(assetName));
					}
				}
				else
				{
					//Compile and load asset
					ECSSystemManager::Instance().GetSystem<MeshRendererSystem>()->
						SetMeshRenderer(spawn, AssetManager::Instance().CompileAndLoad<RenderObject>(assetName));
				}
			}
			// For prefab
			else if (const ImGuiPayload* prefabPayload = ImGui::AcceptDragDropPayload("m_Prefab"))
			{
				std::string assetName = (const char*)prefabPayload->Data;
				std::string filePath = assetName.substr(0, assetName.find_last_of(FILESYS_PREFABASSTYPE) + 1);
				filePath.erase(filePath.find(FILESYS_PREFABASSTYPE));	// This is to remove unneeded data at the end after ".prefab"
				filePath += FILESYS_PREFABASSTYPE;

				// Create Prefab Instance if it is valid
				PrefabSystem* prefabsystem{ ECSSystemManager::Instance().GetSystem<PrefabSystem>() };
				std::string prefabGUID{ prefabsystem->ReadPrefabAssetFile(filePath) };
				if (prefabGUID.empty())
				{
					std::string str{ CONSOLE_DEBUG_WARN };
					str += "Prefab not found!";
					EventHandler::getEventHandlerInstance().Publish(ConsoleDebugEvent{ str.c_str() });
					if (remove(filePath.c_str()))
					{
						std::string funcName{ __FUNCTION__ };
						TRE_CORE_WARN("[" + funcName + "] Unable to delete filePath! (" + filePath + ")");
					}
				}
				else
				{
					Entity prefabInstance{ prefabsystem->CreatePrefabEntityInstance(prefabGUID) };
					EditorSystemManager::Instance().GetSystem<EditorSystem>()->GetSelectionManager()->SelectEntity(prefabInstance);
				}
			}

			ImGui::EndDragDropTarget();
		}

		UpdateGizmo();

		ImGui::End();
	}

	void ViewportPanel::Shutdown()
	{

	}

	glm::vec2 ViewportPanel::PanSensitivity(const float viewportWidth, const float viewportHeight)
	{
		float x = std::min(viewportWidth / 1000.f, 2.4f); //Max is 2.4f
		float xFactor = 0.0366f * (x * x) - 0.1778f * x + 0.3021f;

		float y = std::min(viewportHeight / 1000.f, 2.4f); //Max is 2.4f
		float yFactor = 0.0366f * (y * y) - 0.1778f * y + 0.3021f;

		return { xFactor, yFactor };
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

	void ViewportPanel::UpdateClickRay()
	{
		//Entity entity = ECSSystemManager::Instance().GetSystem<CameraSystem>()->GetMainCamera();
		//CameraSystem* cameraSystem = ECSSystemManager::Instance().GetSystem<CameraSystem>();
		const EditorCamera& camera = EditorCamera::Instance();

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

		glm::mat4 invProjView = camera.GetInverseViewProjectionMatrix();
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
		m_ClickRay = end - start;
	}

	void ViewportPanel::UpdateGizmo()
	{
		Entity SelectedEntity = m_SelectionManager->GetSelectedEntity();
		if (SelectedEntity && m_GizmoOperation != -1)
		{
			ImGuizmo::SetOrthographic(true);
			ImGuizmo::SetDrawlist();

			ImGuizmo::SetRect(m_WindowPos.x + m_ImageOffset.x, m_WindowPos.y + m_ImageOffset.y, m_ImageSize.x, m_ImageSize.y);

			const EditorCamera& camera = EditorCamera::Instance();
			glm::mat4 proj = camera.GetProjectionMatrix();
			//Flip back x and y axis
			//proj[0][0] *= -1.f;
			proj[1][1] *= -1.f;
			glm::mat4 View = camera.GetViewMatrix();

			Transform& transform = SelectedEntity->GetComponent<Transform>();
			glm::mat4 xform = transform.m_WorldXform;

			float snapValue = 1.f;
			switch (m_GizmoOperation)
			{
			case ImGuizmo::OPERATION::TRANSLATE:
				snapValue = m_PosIncrement;
				break;
			case ImGuizmo::OPERATION::ROTATE:
				snapValue = m_RotIncrement;
				break;
			case ImGuizmo::OPERATION::SCALE:
				snapValue = m_ScaleIncrement;
				break;
			}

			ImGuizmo::MODE mode = ImGuizmo::WORLD;
			if(m_IsGizmoLocal)
				mode = ImGuizmo::LOCAL;

			ImGuizmo::Manipulate(glm::value_ptr(View), glm::value_ptr(proj), (ImGuizmo::OPERATION)m_GizmoOperation, mode, glm::value_ptr(xform), nullptr, m_IsGridAndSnap  ? &snapValue : nullptr);

			if (ImGuizmo::IsUsing())
			{
				std::string compName{ ComponentManager::Instance().GetComponentName<Transform>() };
				bool needUpdatingToPrefab{ SelectedEntity->HasComponent<Prefabing>() && SelectedEntity->GetComponent<Prefabing>().m_AddeddComps.find(compName) == SelectedEntity->GetComponent<Prefabing>().m_AddeddComps.end() };

				glm::vec3 Scale;
				glm::vec3 Rotation;
				glm::vec3 Translate;
				ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(xform), glm::value_ptr(Translate), glm::value_ptr(Rotation), glm::value_ptr(Scale));
				switch (m_GizmoOperation)
				{
				case ImGuizmo::OPERATION::SCALE:
					transform.m_Scale = Scale;
					if (needUpdatingToPrefab)
					{
						// See if can emplace back
						Prefabing& prefab{ SelectedEntity->GetComponent<Prefabing>() };
						auto it{ prefab.m_Overrides.find(compName) };
						if (it == prefab.m_Overrides.end())
						{
							prefab.m_Overrides.emplace(std::piecewise_construct, std::forward_as_tuple(compName), std::forward_as_tuple());
						}
						// This has to be hardcoded cos protperty have a specific way of reading variable name data :/
						// If gizmo doesn't update prefab, check Transform.h
						prefab.m_Overrides[compName].emplace("TRE::Transform/Scale");
					}
					break;
				case ImGuizmo::OPERATION::ROTATE:
					transform.m_Rotation = Rotation;
					if (needUpdatingToPrefab)
					{
						// See if can emplace back
						Prefabing& prefab{ SelectedEntity->GetComponent<Prefabing>() };
						auto it{ prefab.m_Overrides.find(compName) };
						if (it == prefab.m_Overrides.end())
						{
							prefab.m_Overrides.emplace(std::piecewise_construct, std::forward_as_tuple(compName), std::forward_as_tuple());
						}
						// This has to be hardcoded cos protperty have a specific way of reading variable name data :/
						// If gizmo doesn't update prefab, check Transform.h
						prefab.m_Overrides[compName].emplace("TRE::Transform/Rotate");
					}
					break;
				case ImGuizmo::OPERATION::TRANSLATE:
					transform.m_Position = Translate;
					if (needUpdatingToPrefab)
					{
						// See if can emplace back
						Prefabing& prefab{ SelectedEntity->GetComponent<Prefabing>() };
						auto it{ prefab.m_Overrides.find(compName) };
						if (it == prefab.m_Overrides.end())
						{
							prefab.m_Overrides.emplace(std::piecewise_construct, std::forward_as_tuple(compName), std::forward_as_tuple());
						}
						// This has to be hardcoded cos protperty have a specific way of reading variable name data :/
						// If gizmo doesn't update prefab, check Transform.h
						prefab.m_Overrides[compName].emplace("TRE::Transform/Position");
					}
					break;
				}
				transform.m_IsDirty = true;
			}
		}
	}

	void ViewportPanel::MouseActions()
	{
		if (m_IsViewportHovered == false)
			return;

		EditorCamera& editorCamera = EditorCamera::Instance();
		const BaseCamera& baseCamera = editorCamera.m_BaseCamera;
		{
			static glm::vec2 panMouseStartPos{};
			static glm::vec2 panMouseEndPos{};
			if (ImGui::IsMouseDown(ImGuiMouseButton_Middle))
			{
				panMouseEndPos = m_MousePos;
				glm::vec2 positionOffset = panMouseEndPos - panMouseStartPos;
				panMouseStartPos = panMouseEndPos;

				if (glm::length(positionOffset) < 0.1f)
					return;
				positionOffset = glm::normalize(positionOffset);
				positionOffset.y *= -1;
				const auto panSensitivity = PanSensitivity(m_ImageSize.x, m_ImageSize.y);
				positionOffset.x *= panSensitivity.x;
				positionOffset.y *= panSensitivity.y;
				positionOffset *= m_PanSpeed * 10.f/*camera.m_FocalLength / 100.f*/;
				positionOffset *= ImGui::GetIO().DeltaTime;

				editorCamera.SetFocalPoint(baseCamera.m_FocalPoint + baseCamera.GetRightVec() * positionOffset.x);
				editorCamera.SetFocalPoint(baseCamera.m_FocalPoint + baseCamera.GetUpVec() * positionOffset.y);
			}
			else
			{
				panMouseStartPos = m_MousePos;
			}
		}

		{
			static glm::vec2 rotMouseStartPos{};
			static glm::vec2 rotMouseEndPos{};

			if (ImGui::IsMouseDown(ImGuiMouseButton_Right))
			{
				rotMouseEndPos = m_MousePos;
				glm::vec2 rotationOffset = rotMouseEndPos - rotMouseStartPos;
				rotMouseStartPos = rotMouseEndPos;

				if (glm::length(rotationOffset) < 0.1f)
					return;
				rotationOffset = glm::normalize(rotationOffset);
				rotationOffset *= -1;
				rotationOffset *= m_RotationSensitivity;
				rotationOffset *= ImGui::GetIO().DeltaTime;

				const float yawSign = baseCamera.GetUpVec().y < 0 ? -1.f : 1.f;
				editorCamera.SetYaw(baseCamera.m_Yaw + yawSign * rotationOffset.x);
				editorCamera.SetPitch(baseCamera.m_Pitch + rotationOffset.y);
			}
			else
			{
				rotMouseStartPos = m_MousePos;
			}


		}
	}
}