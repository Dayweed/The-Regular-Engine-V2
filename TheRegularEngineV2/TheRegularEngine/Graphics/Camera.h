#pragma once
#include "pch.h"
#include "Properties.h"
#include "ECS/System.h"
#include "ECS/ECS.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/quaternion.hpp"

#include <glm/gtc/type_ptr.hpp>

namespace TRE
{
	class BaseCamera
	{
	public:
		glm::mat4 m_ViewMatrix{ 1.f };
		glm::mat4 m_ProjectionMatrix{ 1.f };
		glm::vec2 m_ViewportSize{ 1600.f, 900.f }; // SSSS
		float m_Pitch{ 0.f }; // SSSS
		float m_Yaw{ 0.f }; // SSSS
		float m_Roll{ 0.f }; // SSSS
		float m_Fov{ 60.f }; // SSSS	//Vertical fov - has to be converted to radians
		float m_Near{ 0.1f }; // SSSS
		float m_Far{ 1000.f }; // SSSS
		float m_Left{ -1.f }; // SSSS
		float m_Right{ 1.f }; // SSSS
		float m_Bottom{ -1.f }; // SSSS
		float m_Top{ 1.f }; // SSSS
		float m_FocalLength{ 100.f }; // SSSS
		glm::vec3 m_FocalPoint{ 0.f, 0.f, m_FocalLength }; // SSSS
		float m_AspectRatio{ 16.f / 9.f }; // SSSS
		bool m_IsPerspective{ true }; // SSSS
		
		const glm::vec3 GetUpVec() const;
		const glm::vec3 GetRightVec() const;
		const glm::vec3 GetForwardVec() const;
		const glm::vec3 GetViewDirection() const;
		std::array<glm::vec3, 8> GetFrustumCorners(const bool useRenderRatio, const float ratio) const;
	public:
		const glm::quat GetOrientation() const;
	};

	namespace CameraHelper
	{
		void UpdateViewMatrix(BaseCamera& camera, const glm::vec3 position);
		void UpdateProjectionMatrix(BaseCamera& camera);
		void SetViewDirection(BaseCamera& camera, const glm::vec3& direction, const glm::vec3& position);
		void SetViewTarget(BaseCamera& camera, const glm::vec3& target, const glm::vec3& position);
		void CalculateQuarternions(BaseCamera& camera, const glm::vec3& rotation);
	}
	
	class CameraSystem : public ECSSystem
	{
	public:
		void LateUpdate() override;
		void AfterReset() override;
		void OnDestroyEntities() override;
		void Shutdown() override;

		void SetViewportSize(Entity& go, const glm::vec2& viewportSize);
		void SetFocalPoint(Entity& go, const glm::vec3& focalPoint);
		void SetFocalLength(Entity& go, const float focalLength);
		/*void SetPitch(Entity& go, const float pitch);
		void SetYaw(Entity& go, const float yaw);
		void SetRoll(Entity& go, const float roll);*/
		void SetFov(Entity& go, const float fov);
		void SetNear(Entity& go, const  float near);
		void SetFar(Entity& go, const float far);
		void SetLeft(Entity& go, const float left);
		void SetRight(Entity& go, const float right);
		void SetBottom(Entity& go, const float bottom);
		void SetTop(Entity& go, const float top);
		void SetAspectRatio(Entity& go, const float aspectRatio);
		void SetIsPerspective(Entity& go, const bool isPerspective);
		void SetIsMainCamera(Entity& go, const bool isMainCamera);

		const glm::mat4& GetViewMatrix(const Entity& go) const;
		const glm::mat4& GetProjectionMatrix(const Entity& go) const;
		const glm::mat4 GetInverseViewMatrix(const Entity& go) const;
		const glm::mat4 GetInverseProjectionMatrix(const Entity& go) const;
		const glm::mat4 GetInverseViewProjectionMatrix(const Entity& go) const;
		const glm::vec2& GetViewportSize(const Entity& go) const;
		/*const glm::vec3& GetFocalPoint(const Entity& go) const;
		const float GetFocalLength(const Entity& go) const;
		const float GetPitch(const Entity& go) const;
		const float GetYaw(const Entity& go) const;
		const float GetRoll(const Entity& go) const;*/
		const float GetFov(const Entity& go) const;
		const float GetNear(const Entity& go) const;
		const float GetFar(const Entity& go) const;
		const float GetLeft(const Entity& go) const;
		const float GetRight(const Entity& go) const;
		const float GetBottom(const Entity& go) const;
		const float GetTop(const Entity& go) const;
		const float GetAspectRatio(const Entity& go) const;
		const bool IsPerspective(const Entity& go) const;
		const bool IsMainCamera(const Entity& go) const;

		void MainCameraLookAt(const glm::vec3& target);
		void MainCameraFollow(const glm::vec3& target, const float distance);
		void TransitionCamera(const glm::vec3& targetPosition, const glm::vec3& targetRotation, const float duration);
		void TransitionCameraPosition(const glm::vec3& targetPosition, const float duration);
		void TransitionCameraRotation(const glm::vec3& targetRotation, const float duration);

		Entity GetMainCamera();

		void SetIsDirty(const bool isDirty);
		const bool GetIsDirty() const;
	private:
		bool m_IsDirty{ false }; //Bool to update descriptor set
	};
}