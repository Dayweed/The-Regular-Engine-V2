#include "pch.h"
#include "Camera.h"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/matrix_access.hpp"

namespace TRE
{
	void CameraHelper::UpdateViewMatrix(Camera& camera)
	{
		const glm::quat orientation = camera.GetOrientation();
		const glm::vec3 position = camera.m_FocalPoint - camera.m_FocalLength * camera.GetForwardVec();
		camera.m_ViewMatrix = glm::translate(glm::mat4(1.f), position) * glm::toMat4(orientation);
		//std::cout << "Camera Position: " << position.x << ", " << position.y << ", " << position.z << std::endl;
		//std::cout << "focal point: " << camera.m_FocalPoint.x << ", " << camera.m_FocalPoint.y << ", " << camera.m_FocalPoint.z << std::endl;

		camera.m_ViewMatrix = glm::inverse(camera.m_ViewMatrix);

		glm::mat4 x = glm::mat4(1.f);
		x[1][1] = -1;
		x[2][2] = -1;
		//x[1] = glm::vec4(camera.GetUpVec(), 0.f);
		//x[2] = glm::vec4(camera.GetForwardVec(), 0.f);
		//camera.m_ViewMatrix = glm::inverse(x) * camera.m_ViewMatrix;
	}

	void CameraHelper::UpdateProjectionMatrix(Camera& camera)
	{
		if (camera.m_IsPerspective)
		{
			assert(camera.m_Fov > 0.f);
			assert(camera.m_Fov < std::numeric_limits<float>::max());
			assert(camera.m_AspectRatio > 0.f);
			assert(camera.m_AspectRatio < std::numeric_limits<float>::max());
			assert(camera.m_Far > camera.m_Near);
			camera.m_ProjectionMatrix = glm::mat4(1.f);
			const float tanHalfFov = glm::tan(glm::radians(camera.m_Fov));
			camera.m_ProjectionMatrix[0][0] = 1.f / (tanHalfFov * camera.m_AspectRatio);
			camera.m_ProjectionMatrix[1][1] = 1.f / tanHalfFov;
			camera.m_ProjectionMatrix[2][2] = camera.m_Far / (camera.m_Far - camera.m_Near);
			camera.m_ProjectionMatrix[2][3] = 1.f;
			camera.m_ProjectionMatrix[3][2] = -(camera.m_Far * camera.m_Near) / (camera.m_Far - camera.m_Near);
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

	void CameraHelper::SetViewDirection(Camera& camera, const glm::vec3& direction)
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
		camera.m_ViewMatrix[3][0] = -glm::dot(u, camera.m_Position);
		camera.m_ViewMatrix[3][1] = -glm::dot(v, camera.m_Position);
		camera.m_ViewMatrix[3][2] = -glm::dot(w, camera.m_Position);
	}

	void CameraHelper::SetViewTarget(Camera& camera, const glm::vec3& target)
	{
		CameraHelper::SetViewDirection(camera, target - camera.m_Position);
	}

	const glm::quat Camera::GetOrientation() const
	{
		return glm::quat(glm::vec3(-m_Pitch, -m_Yaw, 0.0f));
	}

	const glm::vec3 Camera::GetUpVec() const
	{
		return glm::rotate(GetOrientation(), glm::vec3(0.f, 1.f, 0.f));
	}

	const glm::vec3 Camera::GetRightVec() const
	{
		return glm::rotate(GetOrientation(), glm::vec3(1.f, 0.f, 0.f));
	}

	const glm::vec3 Camera::GetForwardVec() const
	{
		return glm::rotate(GetOrientation(), glm::vec3(0.f, 0.f, 1.f));
	}

	void CameraSystem::Update()
	{
		for (Entity& go : ECSManager::Instance().GetEntities<Camera>())
		{
			Camera& camera = go.get()->GetComponent<Camera>();
			if (camera.m_IsDirty)
			{
				CameraHelper::UpdateViewMatrix(camera);
				CameraHelper::UpdateProjectionMatrix(camera);
				camera.m_IsDirty = false;
				m_IsDirty = true;	//To update descriptor set then reset back after
			}
		}
	}

	void CameraSystem::OnDestroyGO()
	{

	}

	void CameraSystem::Shutdown()
	{

	}

	void CameraSystem::SetPosition(Entity& go, const glm::vec3& position)
	{
		Camera& camera = go.get()->GetComponent<Camera>();
		camera.m_Position = position;
		camera.m_IsDirty = true;
	}

	void CameraSystem::SetRotation(Entity& go, const glm::vec3& rotation)
	{
		Camera& camera = go.get()->GetComponent<Camera>();
		camera.m_Rotation = rotation;
		camera.m_IsDirty = true;
	}

	void CameraSystem::SetViewportSize(Entity& go, const glm::vec2& viewportSize)
	{
		Camera& camera = go.get()->GetComponent<Camera>();
		camera.m_ViewportSize = viewportSize;
		camera.m_IsDirty = true;
	}

	void CameraSystem::SetFocalPoint(Entity& go, const glm::vec3& focalPoint)
	{
		Camera& camera = go.get()->GetComponent<Camera>();
		camera.m_FocalPoint = focalPoint;
		camera.m_IsDirty = true;
		
		SetPosition(go, focalPoint - camera.GetForwardVec() * camera.m_FocalLength);
	}

	void CameraSystem::SetFocalLength(Entity& go, const float focalLength)
	{
		Camera& camera = go.get()->GetComponent<Camera>();
		camera.m_FocalLength = focalLength;
		camera.m_IsDirty = true;

		SetPosition(go, camera.m_FocalPoint - camera.GetForwardVec() * focalLength);
	}

	void CameraSystem::SetPitch(Entity& go, const float pitch)
	{
		Camera& camera = go.get()->GetComponent<Camera>();
		camera.m_Pitch = pitch;
		camera.m_IsDirty = true;
	}

	void CameraSystem::SetYaw(Entity& go, const float yaw)
	{
		Camera& camera = go.get()->GetComponent<Camera>();
		camera.m_Yaw = yaw;
		camera.m_IsDirty = true;
	}

	void CameraSystem::SetFov(Entity& go, const float fov)
	{
		Camera& camera = go.get()->GetComponent<Camera>();
		camera.m_Fov = fov;
		camera.m_IsDirty = true;
	}
#undef near
	void CameraSystem::SetNear(Entity& go, const float near)
	{
		Camera& camera = go.get()->GetComponent<Camera>();
		camera.m_Near = near;
		camera.m_IsDirty = true;
	}
#define near

#undef far
	void CameraSystem::SetFar(Entity& go, const float far)
	{
		Camera& camera = go.get()->GetComponent<Camera>();
		camera.m_Far = far;
		camera.m_IsDirty = true;
	}
#define far

	void CameraSystem::SetLeft(Entity& go, const float left)
	{
		Camera& camera = go.get()->GetComponent<Camera>();
		camera.m_Left = left;
		camera.m_IsDirty = true;
	}

	void CameraSystem::SetRight(Entity& go, const float right)
	{
		Camera& camera = go.get()->GetComponent<Camera>();
		camera.m_Right = right;
		camera.m_IsDirty = true;
	}

	void CameraSystem::SetBottom(Entity& go, const float bottom)
	{
		Camera& camera = go.get()->GetComponent<Camera>();
		camera.m_Bottom = bottom;
		camera.m_IsDirty = true;
	}

	void CameraSystem::SetTop(Entity& go, const float top)
	{
		Camera& camera = go.get()->GetComponent<Camera>();
		camera.m_Top = top;
		camera.m_IsDirty = true;
	}

	void CameraSystem::SetAspectRatio(Entity& go, const float aspectRatio)
	{
		Camera& camera = go.get()->GetComponent<Camera>();
		camera.m_AspectRatio= aspectRatio;
		camera.m_IsDirty = true;
	}

	void CameraSystem::SetIsPerspective(Entity& go, const bool isPerspective)
	{
		Camera& camera = go.get()->GetComponent<Camera>();
		camera.m_IsPerspective= isPerspective;
		camera.m_IsDirty = true;
	}

	void CameraSystem::SetIsMainCamera(Entity& go, const bool isMainCamera)
	{
		Camera& camera = go.get()->GetComponent<Camera>();
		camera.m_IsMainCamera = isMainCamera;
		camera.m_IsDirty = true;
	}

	const glm::vec3& CameraSystem::GetPosition(const Entity& go) const
	{
		return go.get()->GetComponent<Camera>().m_Position;
	}

	const glm::vec3& CameraSystem::GetRotation(const Entity& go) const
	{
		return go.get()->GetComponent<Camera>().m_Rotation;
	}

	const glm::mat4& CameraSystem::GetViewMatrix(const Entity& go) const
	{
		return go.get()->GetComponent<Camera>().m_ViewMatrix;
	}

	const glm::mat4& CameraSystem::GetProjectionMatrix(const Entity& go) const
	{
		return go.get()->GetComponent<Camera>().m_ProjectionMatrix;
	}

	const glm::vec2& CameraSystem::GetViewportSize(const Entity& go) const
	{
		return go.get()->GetComponent<Camera>().m_ViewportSize;
	}

	const glm::vec3& CameraSystem::GetFocalPoint(const Entity& go) const
	{
		return go.get()->GetComponent<Camera>().m_FocalPoint;
	}

	const float CameraSystem::GetFocalLength(const Entity& go) const
	{
		return go.get()->GetComponent<Camera>().m_FocalLength;
	}

	const float CameraSystem::GetPitch(const Entity& go) const
	{
		return go.get()->GetComponent<Camera>().m_Pitch;
	}

	const float CameraSystem::GetYaw(const Entity& go) const
	{
		return go.get()->GetComponent<Camera>().m_Yaw;
	}

	const float CameraSystem::GetFov(const Entity& go) const
	{
		return go.get()->GetComponent<Camera>().m_Fov;
	}

	const float CameraSystem::GetNear(const Entity& go) const
	{
		return go.get()->GetComponent<Camera>().m_Near;
	}

	const float CameraSystem::GetFar(const Entity& go) const
	{
		return go.get()->GetComponent<Camera>().m_Far;
	}

	const float CameraSystem::GetLeft(const Entity& go) const
	{
		return go.get()->GetComponent<Camera>().m_Left;
	}

	const float CameraSystem::GetRight(const Entity& go) const
	{
		return go.get()->GetComponent<Camera>().m_Right;
	}

	const float CameraSystem::GetBottom(const Entity& go) const
	{
		return go.get()->GetComponent<Camera>().m_Bottom;
	}

	const float CameraSystem::GetTop(const Entity& go) const
	{
		return go.get()->GetComponent<Camera>().m_Top;
	}

	const float CameraSystem::GetAspectRatio(const Entity& go) const
	{
		return go.get()->GetComponent<Camera>().m_AspectRatio;
	}

	const bool CameraSystem::IsPerspective(const Entity& go) const
	{
		return go.get()->GetComponent<Camera>().m_IsPerspective;
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

		assert(count == 1 && "There can only be one main camera");
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

	//void CameraSystem::NormalizeOrientation(Entity& go)
	//{
	//	auto camera = go->GetComponent<Camera>();
	//	camera.m_UpVec = glm::normalize(camera.m_UpVec);
	//	camera.m_RightVec = glm::normalize(camera.m_RightVec);
	//	camera.m_ForwardVec = glm::normalize(camera.m_ForwardVec);
	//}
}