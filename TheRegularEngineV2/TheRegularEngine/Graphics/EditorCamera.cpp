#include "pch.h"
#include "EditorCamera.h"
#include "Core/SystemManager.h"
#include "Core/Transform.h"
#include "Core/SceneManager.h"
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

	void EditorCamera::Shutdown()
	{
		Serialize();
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

	void EditorCamera::SetViewportSize(const float width, const float height)
	{
		m_BaseCamera.m_AspectRatio = width / height;
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
		SetFocalDistance(20.f);
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

	void EditorCamera::Serialize()
	{
		std::string finalPath = "../EditorData/";

		std::filesystem::directory_entry entry(finalPath);
		if (!entry.exists())
		{
			std::filesystem::create_directory(finalPath);
		}

		finalPath += SceneManager::Instance().GetCurrentSceneName() + ".Editor";
		std::ofstream file(finalPath);

		if (!file.is_open())
		{
			std::cout << "Failed to open file" << finalPath << std::endl;
			return;
		}

		file << "Position: " << m_Position.x << " " << m_Position.y << " " << m_Position.z << std::endl;
		file << "Pitch: " << m_BaseCamera.m_Pitch << std::endl;
		file << "Yaw: " << m_BaseCamera.m_Yaw << std::endl;
		file << "Roll: " << m_BaseCamera.m_Roll << std::endl;
		file << "FocalPoint: " << m_BaseCamera.m_FocalPoint.x << " " << m_BaseCamera.m_FocalPoint.y << " " << m_BaseCamera.m_FocalPoint.z << std::endl;
		file << "FocalLength: " << m_BaseCamera.m_FocalLength << std::endl;
		//file << ""


		file.close();
	}

	void EditorCamera::Deserialize()
	{
		std::string finalPath = "../EditorData/";
		finalPath += SceneManager::Instance().GetCurrentSceneName() + ".Editor";
		std::ifstream file(finalPath);
		if (!file.is_open())
		{
			return;
		}

		std::string line;
		std::getline(file, line);
		std::istringstream iss(line);
		std::string position;
		iss >> position;
		iss >> m_Position.x;
		iss >> m_Position.y;
		iss >> m_Position.z;
		std::string pitch;
		std::getline(file, line);
		iss = std::istringstream(line);
		iss >> pitch;
		iss >> m_BaseCamera.m_Pitch;
		std::string yaw;
		std::getline(file, line);
		iss = std::istringstream(line);
		iss >> yaw;
		iss >> m_BaseCamera.m_Yaw;
		std::string roll;
		std::getline(file, line);
		iss = std::istringstream(line);
		iss >> roll;
		iss >> m_BaseCamera.m_Roll;
		std::string focalPoint;
		std::getline(file, line);
		iss = std::istringstream(line);
		iss >> focalPoint;
		iss >> m_BaseCamera.m_FocalPoint.x;
		iss >> m_BaseCamera.m_FocalPoint.y;
		iss >> m_BaseCamera.m_FocalPoint.z;
		std::string focalLength;
		std::getline(file, line);
		iss = std::istringstream(line);
		iss >> focalLength;
		iss >> m_BaseCamera.m_FocalLength;

		file.close();

		m_IsDirty = true;
	}
}