#pragma once
#include "pch.h"
#include "Core/entt.hpp"
#include "Core/System.h"
#include "Core/ECS.h"

namespace TRE
{
	class Camera
	{
	public:
		glm::vec3 m_Position{ 0.f, 0.f, 0.f };
		glm::vec3 m_Rotation{ 0.f, 0.f, 0.f };
		glm::mat4 m_ViewMatrix{ 1.f };
		glm::mat4 m_ProjectionMatrix{ 1.f };
		glm::vec2 m_ViewportSize{ 1920.f, 1080.f };
		float m_Fov{ 60.f };	//Vertical fov - has to be converted to radians
		float m_Near{ 0.3f };
		float m_Far{ 1000.f };
		float m_Left{ -1.f };
		float m_Right{ 1.f };
		float m_Bottom{ -1.f };
		float m_Top{ 1.f };
		float m_AspectRatio{ 16.f / 9.f };
		bool m_IsPerspective{ true };
		bool m_IsMainCamera{ false };
		bool m_IsDirty{ false };
	};
	
	class CameraSystem : public ECSSystem
	{
	public:

		void Update() override;
		void OnDestroyGO() override;
		void Shutdown() override;

		void SetPosition(GO& go, const glm::vec3& position);
		void SetRotation(GO& go, const glm::vec3& rotation);
		void SetViewportSize(GO& go, const glm::vec2& viewportSize);
		void SetFov(GO& go, const float fov);
		void SetNear(GO& go, const  float near);
		void SetFar(GO& go, const float far);
		void SetLeft(GO& go, const float left);
		void SetRight(GO& go, const float right);
		void SetBottom(GO& go, const float bottom);
		void SetTop(GO& go, const float top);
		void SetAspectRatio(GO& go, const float aspectRatio);
		void SetIsPerspective(GO& go, const bool isPerspective);
		void SetIsMainCamera(GO& go, const bool isMainCamera);

		const glm::vec3& GetPosition(const GO& go) const;
		const glm::vec3& GetRotation(const GO& go) const;
		const glm::mat4& GetViewMatrix(const GO& go) const;
		const glm::mat4& GetProjectionMatrix(const GO& go) const;
		const glm::vec2& GetViewportSize(const GO& go) const;
		const float GetFov(const GO& go) const;
		const float GetNear(const GO& go) const;
		const float GetFar(const GO& go) const;
		const float GetLeft(const GO& go) const;
		const float GetRight(const GO& go) const;
		const float GetBottom(const GO& go) const;
		const float GetTop(const GO& go) const;
		const float GetAspectRatio(const GO& go) const;
		const bool IsPerspective(const GO& go) const;
		const bool IsMainCamera(const GO& go) const;

		GO GetMainCamera() const;

		void SetIsDirty(const bool isDirty);
		const bool GetIsDirty() const;
	private:
		void UpdateViewMatrix(Camera& camera);
		void UpdateProjectionMatrix(Camera& camera);
	private:
		bool m_IsDirty{ false };
	};
}
