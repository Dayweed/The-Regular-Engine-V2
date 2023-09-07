#pragma once
#include "pch.h"
#include "Core/System.h"
#include "Core/ECS.h"

namespace TRE
{
	class Camera;

	namespace CameraHelper
	{
		void UpdateViewMatrix(Camera& camera);
		void UpdateProjectionMatrix(Camera& camera);
		void SetViewDirection(Camera& camera, const glm::vec3& direction);
		void SetViewTarget(Camera& camera, const glm::vec3& target);
	}

	struct FEL
	{
		std::vector<float> i{};
		/*friend void to_json(nlohmann::json& j, const FEL&f)
		{
			j = nlohmann::json{
				{ "i", f.i }
			};
		}
		friend void from_json(const nlohmann::json& j, FEL& f)
		{
				f.i = j.at("i").get<std::vector<float>>();
		}*/
		NLOHMANN_DEFINE_TYPE_INTRUSIVE(FEL, i)
	};

	class Camera
	{
	public:
		FEL fel;
		glm::vec3 m_Position{ 0.f, 0.f, 0.f };
		glm::vec3 m_Rotation{ 0.f, 0.f, 0.f };
		glm::mat4 m_ViewMatrix{ 1.f };
		glm::mat4 m_ProjectionMatrix{ 1.f };
		glm::vec2 m_ViewportSize{ 1920.f, 1080.f };
		glm::vec3 m_UpVec{ 0.f, 1.f, 0.f };
		glm::vec3 m_RightVec{ 1.f, 0.f, 0.f };
		glm::vec3 m_ForwardVec{ 0.f, 0.f, 1.f };
		glm::vec3 m_FocalPoint{ 0.f, 0.f, 10.f };
		float m_FocalLength{ 10.f };
		float m_Fov{ 30.f };	//Vertical fov - has to be converted to radians
		float m_Near{ 0.1f };
		float m_Far{ 1000.f };
		float m_Left{ -1.f };
		float m_Right{ 1.f };
		float m_Bottom{ -1.f };
		float m_Top{ 1.f };
		float m_AspectRatio{ 16.f / 9.f };
		bool m_IsPerspective{ true };
		bool m_IsMainCamera{ false };
		bool m_IsDirty{ false };

		Camera() = default;
		~Camera() = default;

		NLOHMANN_DEFINE_TYPE_INTRUSIVE(Camera
											//, fel
											//, m_Position, m_Rotation, m_ViewMatrix, m_ProjectionMatrix, m_ViewportSize, m_UpVec, m_RightVec, m_ForwardVec, m_FocalPoint
											, m_Fov, m_Near, m_Far, m_Left, m_Right, m_Bottom, m_Top, m_AspectRatio
											, m_IsPerspective, m_IsMainCamera, m_IsDirty)
	};
	
	class CameraSystem : public ECSSystem
	{
	public:
		void Update() override;
		void OnDestroyGO() override;
		void Shutdown() override;

		void SetPosition(Entity& go, const glm::vec3& position);
		void SetRotation(Entity& go, const glm::vec3& rotation);
		void SetViewportSize(Entity& go, const glm::vec2& viewportSize);
		void SetFocalPoint(Entity& go, const glm::vec3& focalPoint);
		void SetFocalLength(Entity& go, const float focalLength);
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

		const glm::vec3& GetPosition(const Entity& go) const;
		const glm::vec3& GetRotation(const Entity& go) const;
		const glm::mat4& GetViewMatrix(const Entity& go) const;
		const glm::mat4& GetProjectionMatrix(const Entity& go) const;
		const glm::vec2& GetViewportSize(const Entity& go) const;
		const glm::vec3& GetFocalPoint(const Entity& go) const;
		const float GetFocalLength(const Entity& go) const;
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

		Entity GetMainCamera() const;

		void SetIsDirty(const bool isDirty);
		const bool GetIsDirty() const;
	private:
		void NormalizeOrientation(Entity& go);
	private:
		bool m_IsDirty{ false }; //Bool to update descriptor set
	};
}
