#include "pch.h"
#include "Camera.h"
#include "Core/Transform.h"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/matrix_access.hpp"

namespace TRE
{
	void CameraHelper::UpdateViewMatrix(BaseCamera& camera, const glm::vec3 position)
	{
		camera.m_ViewMatrix = glm::translate(glm::mat4(1.f), position) * glm::toMat4(camera.GetOrientation());
		camera.m_ViewMatrix = glm::inverse(camera.m_ViewMatrix);
	}

	void CameraHelper::UpdateProjectionMatrix(BaseCamera& camera)
	{
		if (camera.m_IsPerspective)
		{
			assert(camera.m_Fov > 0.f);
			assert(camera.m_Fov < std::numeric_limits<float>::max());
			assert(camera.m_AspectRatio > 0.f);
			assert(camera.m_AspectRatio < std::numeric_limits<float>::max());
			assert(camera.m_Far > camera.m_Near);
			camera.m_ProjectionMatrix = glm::mat4(1.f);
			const float tanHalfFov = glm::tan(glm::radians(camera.m_Fov) / 2.f);
			camera.m_ProjectionMatrix[0][0] = 1.f / (tanHalfFov * camera.m_AspectRatio);
			camera.m_ProjectionMatrix[1][1] = 1.f / tanHalfFov;
			camera.m_ProjectionMatrix[2][2] = camera.m_Far / (camera.m_Far - camera.m_Near);
			camera.m_ProjectionMatrix[2][3] = 1.f;
			camera.m_ProjectionMatrix[3][2] = -(camera.m_Far * camera.m_Near) / (camera.m_Far - camera.m_Near);
			camera.m_ProjectionMatrix[3][3] = 0.f;
			//Fip x and y axis
			//camera.m_ProjectionMatrix[0][0] *= -1.f;
			camera.m_ProjectionMatrix[1][1] *= -1.f;
		}
		else
		{
			assert(camera.m_Right > camera.m_Left);
			assert(camera.m_Top > camera.m_Bottom);
			assert(camera.m_Far > camera.m_Near);
			camera.m_ProjectionMatrix = glm::mat4(1.f);
			camera.m_ProjectionMatrix[0][0] = 2.f / (camera.m_Right - camera.m_Left);
			camera.m_ProjectionMatrix[1][1] = 2.f / (camera.m_Top - camera.m_Bottom);
			camera.m_ProjectionMatrix[2][2] = 1.f / (camera.m_Far - camera.m_Near);
			camera.m_ProjectionMatrix[3][0] = -(camera.m_Right + camera.m_Left) / (camera.m_Right - camera.m_Left);
			camera.m_ProjectionMatrix[3][1] = -(camera.m_Top + camera.m_Bottom) / (camera.m_Top - camera.m_Bottom);
			camera.m_ProjectionMatrix[3][2] = -camera.m_Near / (camera.m_Far - camera.m_Near);
		}
	}

	void CameraHelper::SetViewDirection(BaseCamera& camera, const glm::vec3& direction, const glm::vec3& position)
	{
		assert(direction.length() > 0.f);
		const glm::vec3 w{ glm::normalize(direction) };
		const glm::vec3 u{ glm::normalize(glm::cross(w, camera.GetUpVec())) };
		const glm::vec3 v{ glm::cross(w, u) };

		camera.m_ViewMatrix = glm::mat4{ 1.f };
		camera.m_ViewMatrix[0][0] = u.x;
		camera.m_ViewMatrix[1][0] = u.y;
		camera.m_ViewMatrix[2][0] = u.z;
		camera.m_ViewMatrix[0][1] = v.x;
		camera.m_ViewMatrix[1][1] = v.y;
		camera.m_ViewMatrix[2][1] = v.z;
		camera.m_ViewMatrix[0][2] = w.x;
		camera.m_ViewMatrix[1][2] = w.y;
		camera.m_ViewMatrix[2][2] = w.z;
		camera.m_ViewMatrix[3][0] = -glm::dot(u, position);
		camera.m_ViewMatrix[3][1] = -glm::dot(v, position);
		camera.m_ViewMatrix[3][2] = -glm::dot(w, position);
	}

	void CameraHelper::SetViewTarget(BaseCamera& camera, const glm::vec3& target, const glm::vec3& position)
	{
		CameraHelper::SetViewDirection(camera, target - position, position);
	}

	void CameraHelper::CalculateQuarternions(BaseCamera& camera, const glm::vec3& rotation)
	{
		camera.m_Pitch = glm::radians(rotation.x);
		camera.m_Yaw = glm::radians(rotation.y);
		camera.m_Roll = glm::radians(rotation.z);
	}

	const glm::quat BaseCamera::GetOrientation() const
	{
		return glm::quat(glm::vec3(-m_Pitch, -m_Yaw, -m_Roll));
	}

	const glm::vec3 BaseCamera::GetUpVec() const
	{
		return glm::rotate(GetOrientation(), glm::vec3(0.f, 1.f, 0.f));
	}

	const glm::vec3 BaseCamera::GetRightVec() const
	{
		return glm::rotate(GetOrientation(), glm::vec3(1.f, 0.f, 0.f));
	}

	const glm::vec3 BaseCamera::GetForwardVec() const
	{
		return glm::rotate(GetOrientation(), glm::vec3(0.f, 0.f, -1.f));
	}

	const glm::vec3 BaseCamera::GetViewDirection() const
	{
		return -GetForwardVec();
	}

	void CameraSystem::LateUpdate()
	{
		for (Entity& go : ECSManager::Instance().GetEntities<Camera>())
		{
			const Transform& transform = go->GetComponent<Transform>();
			Camera& camera = go->GetComponent<Camera>();
			if (transform.m_IsDirty)
			{
				//Recalculate view direction, quaternion, right, up, forward
				CameraHelper::CalculateQuarternions(camera.m_BaseCamera, transform.m_Rotation);
				SetFocalPoint(go, transform.m_Position + camera.m_BaseCamera.GetForwardVec() * camera.m_BaseCamera.m_FocalLength);
				camera.m_BaseCamera.m_FocalLength = glm::distance(transform.m_Position, camera.m_BaseCamera.m_FocalPoint);
				CameraHelper::UpdateViewMatrix(camera.m_BaseCamera, transform.m_Position);
			}
			if (camera.m_IsDirty)
			{
				CameraHelper::UpdateViewMatrix(camera.m_BaseCamera, transform.m_Position);
				CameraHelper::UpdateProjectionMatrix(camera.m_BaseCamera);
				camera.m_IsDirty = false;

				m_IsDirty = true;	//To update descriptor set then reset back after
			}
		}
	}

	void CameraSystem::AfterReset()
	{

	}

	void CameraSystem::OnDestroyEntities()
	{

	}

	void CameraSystem::Shutdown()
	{

	}

	void CameraSystem::SetViewportSize(Entity& go, const glm::vec2& viewportSize)
	{
		Camera& camera = go.get()->GetComponent<Camera>();
		camera.m_BaseCamera.m_ViewportSize = viewportSize;
		camera.m_IsDirty = true;
	}

	void CameraSystem::SetFocalPoint(Entity& go, const glm::vec3& focalPoint)
	{
		Camera& camera = go.get()->GetComponent<Camera>();
		camera.m_BaseCamera.m_FocalPoint = focalPoint;
		camera.m_IsDirty = true;
	}

	void CameraSystem::SetFocalLength(Entity& go, const float focalLength)
	{
		Camera& camera = go.get()->GetComponent<Camera>();
		camera.m_BaseCamera.m_FocalLength = focalLength;
		camera.m_IsDirty = true;
	}

	/*void CameraSystem::SetPitch(Entity& go, const float pitch)
	{
		Camera& camera = go.get()->GetComponent<Camera>();
		camera.m_BaseCamera.m_Pitch = pitch;
		camera.m_IsDirty = true;

		camera.m_FocalPoint = camera.m_Position + camera.GetViewDirection() * camera.m_FocalLength;
	}

	void CameraSystem::SetYaw(Entity& go, const float yaw)
	{
		Camera& camera = go.get()->GetComponent<Camera>();
		camera.m_Yaw = yaw;
		camera.m_IsDirty = true;

		camera.m_FocalPoint = camera.m_Position + camera.GetViewDirection() * camera.m_FocalLength;
	}

	void CameraSystem::SetRoll(Entity& go, const float roll)
	{
		Camera& camera = go.get()->GetComponent<Camera>();
		camera.m_Roll = roll;
		camera.m_IsDirty = true;
		
		camera.m_FocalPoint = camera.m_Position + camera.GetViewDirection() * camera.m_FocalLength;
	}*/

	void CameraSystem::SetFov(Entity& go, const float fov)
	{
		Camera& camera = go.get()->GetComponent<Camera>();
		camera.m_BaseCamera.m_Fov = fov;
		camera.m_IsDirty = true;
	}
#undef near
	void CameraSystem::SetNear(Entity& go, const float near)
	{
		Camera& camera = go.get()->GetComponent<Camera>();
		camera.m_BaseCamera.m_Near = near;
		camera.m_IsDirty = true;
	}
#define near

#undef far
	void CameraSystem::SetFar(Entity& go, const float far)
	{
		Camera& camera = go.get()->GetComponent<Camera>();
		camera.m_BaseCamera.m_Far = far;
		camera.m_IsDirty = true;
	}
#define far

	void CameraSystem::SetLeft(Entity& go, const float left)
	{
		Camera& camera = go.get()->GetComponent<Camera>();
		camera.m_BaseCamera.m_Left = left;
		camera.m_IsDirty = true;
	}

	void CameraSystem::SetRight(Entity& go, const float right)
	{
		Camera& camera = go.get()->GetComponent<Camera>();
		camera.m_BaseCamera.m_Right = right;
		camera.m_IsDirty = true;
	}

	void CameraSystem::SetBottom(Entity& go, const float bottom)
	{
		Camera& camera = go.get()->GetComponent<Camera>();
		camera.m_BaseCamera.m_Bottom = bottom;
		camera.m_IsDirty = true;
	}

	void CameraSystem::SetTop(Entity& go, const float top)
	{
		Camera& camera = go.get()->GetComponent<Camera>();
		camera.m_BaseCamera.m_Top = top;
		camera.m_IsDirty = true;
	}

	void CameraSystem::SetAspectRatio(Entity& go, const float aspectRatio)
	{
		Camera& camera = go.get()->GetComponent<Camera>();
		camera.m_BaseCamera.m_AspectRatio= aspectRatio;
		camera.m_IsDirty = true;
	}

	void CameraSystem::SetIsPerspective(Entity& go, const bool isPerspective)
	{
		Camera& camera = go.get()->GetComponent<Camera>();
		camera.m_BaseCamera.m_IsPerspective= isPerspective;
		camera.m_IsDirty = true;
	}

	void CameraSystem::SetIsMainCamera(Entity& go, const bool isMainCamera)
	{
		Camera& camera = go.get()->GetComponent<Camera>();
		camera.m_IsMainCamera = isMainCamera;
		camera.m_IsDirty = true;
	}

	const glm::mat4& CameraSystem::GetViewMatrix(const Entity& go) const
	{
		return go.get()->GetComponent<Camera>().m_BaseCamera.m_ViewMatrix;
	}

	const glm::mat4& CameraSystem::GetProjectionMatrix(const Entity& go) const
	{
		return go.get()->GetComponent<Camera>().m_BaseCamera.m_ProjectionMatrix;
	}

	const glm::mat4 CameraSystem::GetInverseViewMatrix(const Entity& go) const
	{
		return glm::inverse(go.get()->GetComponent<Camera>().m_BaseCamera.m_ViewMatrix);
	}

	const glm::mat4 CameraSystem::GetInverseProjectionMatrix(const Entity& go) const
	{
		return glm::inverse(go.get()->GetComponent<Camera>().m_BaseCamera.m_ProjectionMatrix);
	}

	const glm::mat4 CameraSystem::GetInverseViewProjectionMatrix(const Entity& go) const
	{
		return GetInverseViewMatrix(go) * GetInverseProjectionMatrix(go);
	}

	const glm::vec2& CameraSystem::GetViewportSize(const Entity& go) const
	{
		return go.get()->GetComponent<Camera>().m_BaseCamera.m_ViewportSize;
	}

	//const glm::vec3& CameraSystem::GetFocalPoint(const Entity& go) const
	//{
	//	return go.get()->GetComponent<Camera>().m_BaseCamera.m_FocalPoint;
	//}

	//const float CameraSystem::GetFocalLength(const Entity& go) const
	//{
	//	return go.get()->GetComponent<Camera>().m_BaseCamera.m_FocalLength;
	//}

	//const float CameraSystem::GetPitch(const Entity& go) const
	//{
	//	return go.get()->GetComponent<Camera>().m_Pitch;
	//}

	//const float CameraSystem::GetYaw(const Entity& go) const
	//{
	//	return go.get()->GetComponent<Camera>().m_Yaw;
	//}

	//const float CameraSystem::GetRoll(const Entity& go) const
	//{
	//	return go.get()->GetComponent<Camera>().m_Roll;
	//}

	const float CameraSystem::GetFov(const Entity& go) const
	{
		return go.get()->GetComponent<Camera>().m_BaseCamera.m_Fov;
	}

	const float CameraSystem::GetNear(const Entity& go) const
	{
		return go.get()->GetComponent<Camera>().m_BaseCamera.m_Near;
	}

	const float CameraSystem::GetFar(const Entity& go) const
	{
		return go.get()->GetComponent<Camera>().m_BaseCamera.m_Far;
	}

	const float CameraSystem::GetLeft(const Entity& go) const
	{
		return go.get()->GetComponent<Camera>().m_BaseCamera.m_Left;
	}

	const float CameraSystem::GetRight(const Entity& go) const
	{
		return go.get()->GetComponent<Camera>().m_BaseCamera.m_Right;
	}

	const float CameraSystem::GetBottom(const Entity& go) const
	{
		return go.get()->GetComponent<Camera>().m_BaseCamera.m_Bottom;
	}

	const float CameraSystem::GetTop(const Entity& go) const
	{
		return go.get()->GetComponent<Camera>().m_BaseCamera.m_Top;
	}

	const float CameraSystem::GetAspectRatio(const Entity& go) const
	{
		return go.get()->GetComponent<Camera>().m_BaseCamera.m_AspectRatio;
	}

	const bool CameraSystem::IsPerspective(const Entity& go) const
	{
		return go.get()->GetComponent<Camera>().m_BaseCamera.m_IsPerspective;
	}

	const bool CameraSystem::IsMainCamera(const Entity& go) const
	{
		return go.get()->GetComponent<Camera>().m_IsMainCamera;
	}

	Entity CameraSystem::GetMainCamera() const
	{
		//Can only have one main camera
		Entity mainCamera;
		int count = 0;
		for (Entity& go : ECSManager::Instance().GetEntities<Camera>())
		{
			Camera& camera = go.get()->GetComponent<Camera>();
			if (camera.m_IsMainCamera)
			{
				++count;
				mainCamera = go;
			}
		}
		return mainCamera;
	}


	void CameraSystem::SetIsDirty(const bool isDirty)
	{
		m_IsDirty = isDirty;
	}

	const bool CameraSystem::GetIsDirty() const
	{
		return m_IsDirty;
	}
}