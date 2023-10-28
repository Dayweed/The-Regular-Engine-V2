#include "pch.h"
#include "EditorCamera.h"
#include "Core/SystemManager.h"
#include "Core/Transform.h"
#include <iostream>

namespace TRE
{
	void EditorCamera::Init()
	{
		m_IsDirty = true;
	}

	void EditorCamera::Update()
	{
		if (m_IsDirty)
		{
			CameraHelper::UpdateViewMatrix(m_BaseCamera, m_Position);
			CameraHelper::UpdateProjectionMatrix(m_BaseCamera);
			m_IsDirty = false;
		}
	}

	void EditorCamera::SetFocalPoint(const glm::vec3& focalPoint)
	{
		m_BaseCamera.m_FocalPoint = focalPoint;
		SetPosition(focalPoint - m_BaseCamera.m_FocalLength * m_BaseCamera.GetViewDirection());
		m_IsDirty = true;
	}

	void EditorCamera::SetFocalDistance(const float distance)
	{
		m_BaseCamera.m_FocalLength = distance;
		SetPosition(m_BaseCamera.m_FocalPoint - m_BaseCamera.m_FocalLength * m_BaseCamera.GetViewDirection());
		m_IsDirty = true;
	}

	void EditorCamera::SetPitch(const float pitch)
	{
		m_BaseCamera.m_Pitch = pitch;
		m_BaseCamera.m_FocalPoint = m_Position + m_BaseCamera.m_FocalLength * m_BaseCamera.GetViewDirection();
		m_IsDirty = true;
	}

	void EditorCamera::SetYaw(const float yaw)
	{
		m_BaseCamera.m_Yaw = yaw;
		m_BaseCamera.m_FocalPoint = m_Position + m_BaseCamera.m_FocalLength * m_BaseCamera.GetViewDirection();
		m_IsDirty = true;
	}

	void EditorCamera::SetRoll(const float roll)
	{
		m_BaseCamera.m_Roll = roll;
		m_BaseCamera.m_FocalPoint = m_Position + m_BaseCamera.m_FocalLength * m_BaseCamera.GetViewDirection();
		m_IsDirty = true;
	}

	void EditorCamera::SetPosition(const glm::vec3& position)
	{
		m_Position = position;
		m_IsDirty = true;
	}

	void EditorCamera::SetDirection(const glm::vec3& position)
	{
		glm::vec3 direction = glm::normalize(position - m_Position);
		SetFocalDistance(glm::length(position - m_Position) > 200.f ? glm::length(position - m_Position)/2.f : 100.f);
		SetFocalPoint(position);
		m_IsDirty = true;
	}

	void EditorCamera::AssignToMainCamera()
	{
		CameraSystem* cameraSystem = ECSSystemManager::Instance().GetSystem<CameraSystem>();
		Entity mainCamera = cameraSystem->GetMainCamera();
		if (mainCamera)
		{
			Transform& transform = mainCamera->GetComponent<Transform>();
			Camera& camera = mainCamera->GetComponent<Camera>();
			BaseCamera& baseCamera = mainCamera->GetComponent<Camera>().m_BaseCamera;
			baseCamera.m_Pitch = m_BaseCamera.m_Pitch;
			baseCamera.m_Yaw = m_BaseCamera.m_Yaw;
			baseCamera.m_Roll = m_BaseCamera.m_Roll;
			//baseCamera.m_FocalPoint = m_BaseCamera.m_FocalPoint;
			//baseCamera.m_FocalLength = m_BaseCamera.m_FocalLength;
			camera.m_IsDirty = true;

			transform.m_Position = m_Position;
			transform.m_Rotation = m_Rotation;
			transform.m_IsDirty = true;
		}
	}
}