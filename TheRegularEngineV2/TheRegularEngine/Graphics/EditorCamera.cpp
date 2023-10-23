#include "pch.h"
#include "EditorCamera.h"
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
}