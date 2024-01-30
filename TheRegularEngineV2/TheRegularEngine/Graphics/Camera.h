#pragma once
#include "pch.h"
#include "Properties.h"
#include "Core/System.h"
#include "Core/ECS.h"

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

	class Camera;

	namespace CameraHelper
	{
		void UpdateViewMatrix(BaseCamera& camera, const glm::vec3 position);
		void UpdateProjectionMatrix(BaseCamera& camera);
		void SetViewDirection(BaseCamera& camera, const glm::vec3& direction, const glm::vec3& position);
		void SetViewTarget(BaseCamera& camera, const glm::vec3& target, const glm::vec3& position);
		void CalculateQuarternions(BaseCamera& camera, const glm::vec3& rotation);
	}

	class Camera : property::base
	{
	public:
		BaseCamera m_BaseCamera;
		bool m_IsMainCamera{ false }; // SSSS
		//For transition
		glm::vec3 m_StartPosition{};
		glm::vec3 m_TransitionPosition{};
		glm::vec3 m_StartRotation{};
		glm::vec3 m_TransitionRotation{};
		float m_InterpolationValue{};
		float m_InterpolationSpeed{ 0.5f };
		bool m_IsTransitioning{ false };
		//End For transition
		bool m_IsDirty{ false };

		// MUST Use BOTH of this if have variables that are struct/class to serialize
		friend void to_json(nlohmann::json& j, const Camera& t) // Serialize
		{
			const float* vpsize = glm::value_ptr(t.m_BaseCamera.m_ViewportSize);
			std::vector<float> v_vpsize{ vpsize[0], vpsize[1] };
			const float* focal = glm::value_ptr(t.m_BaseCamera.m_FocalPoint);
			std::vector<float> v_focal{ focal[0], focal[1], focal[2] };

			j = nlohmann::json{
				{ "m_ViewportSize", v_vpsize },
				{ "m_Fov", t.m_BaseCamera.m_Fov },
				{ "m_Near", t.m_BaseCamera.m_Near },
				{ "m_Far", t.m_BaseCamera.m_Far },
				{ "m_FocalLength", t.m_BaseCamera.m_FocalLength },
				{ "m_FocalPoint", v_focal },
				{ "m_Left", t.m_BaseCamera.m_Left },
				{ "m_Right", t.m_BaseCamera.m_Right },
				{ "m_Bottom", t.m_BaseCamera.m_Bottom },
				{ "m_Top", t.m_BaseCamera.m_Top },
				{ "m_AspectRatio", t.m_BaseCamera.m_AspectRatio },
				{ "m_IsPerspective", t.m_BaseCamera.m_IsPerspective },
				{ "m_IsMainCamera", t.m_IsMainCamera },
			};
		}
		friend void from_json(const nlohmann::json& j, Camera& t) // Deserialize
		{
			if (j.contains("m_ViewportSize"))
			{
				std::vector<float> v_vpsize{ j.at("m_ViewportSize").get<std::vector<float>>() };
				float a_vpsize[2]{ v_vpsize[0], v_vpsize[1] };
				t.m_BaseCamera.m_ViewportSize = glm::make_vec2(a_vpsize);
			}
			if(j.contains("m_Fov"))
				t.m_BaseCamera.m_Fov = j.at("m_Fov").get<float>();
			if (j.contains("m_Near"))
				t.m_BaseCamera.m_Near = j.at("m_Near").get<float>();
			if (j.contains("m_Far"))
				t.m_BaseCamera.m_Far = j.at("m_Far").get<float>();
			if (j.contains("m_FocalLength"))
				t.m_BaseCamera.m_FocalLength = j.at("m_FocalLength").get<float>();
			if (j.contains("m_FocalPoint"))
			{
				std::vector<float> v_pnt{ j.at("m_FocalPoint").get<std::vector<float>>() };
				float a_pnt[3]{ v_pnt[0], v_pnt[1], v_pnt[2] };
				t.m_BaseCamera.m_FocalPoint = glm::make_vec3(a_pnt);
			}
			if(j.contains("m_Left"))
				t.m_BaseCamera.m_Left = j.at("m_Left").get<float>();
			if (j.contains("m_Right"))
						t.m_BaseCamera.m_Right = j.at("m_Right").get<float>();
			if (j.contains("m_Bottom"))
				t.m_BaseCamera.m_Bottom = j.at("m_Bottom").get<float>();
			if (j.contains("m_Top"))
				t.m_BaseCamera.m_Top = j.at("m_Top").get<float>();
			if (j.contains("m_AspectRatio"))
				t.m_BaseCamera.m_AspectRatio = j.at("m_AspectRatio").get<float>();
			if (j.contains("m_IsPerspective"))
				t.m_BaseCamera.m_IsPerspective = j.at("m_IsPerspective").get<bool>();
			if (j.contains("m_IsMainCamera"))
				t.m_IsMainCamera = j.at("m_IsMainCamera").get<bool>();

			t.m_IsDirty = true;
		}

		property_vtable()
	};
	
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
		void TransitionCamera(const glm::vec3& targetPosition, const glm::vec3& targetRotation, const float speed);
		void TransitionCameraPosition(const glm::vec3& targetPosition, const float speed);
		void TransitionCameraRotation(const glm::vec3& targetRotation, const float speed);

		Entity GetMainCamera();

		void SetIsDirty(const bool isDirty);
		const bool GetIsDirty() const;
	private:
		bool m_IsDirty{ false }; //Bool to update descriptor set
	};
}

property_begin(TRE::Camera)
{
		//property_var(m_BaseCamera.m_ViewportSize).Name("Viewport Size"),
		property_var(m_BaseCamera.m_Fov).Name("FOV"),
		property_var(m_BaseCamera.m_FocalLength).Name("Focal Length"),
		property_var(m_BaseCamera.m_FocalPoint).Name("Focal Point"),
		property_var(m_BaseCamera.m_Near).Name("Near"),
		property_var(m_BaseCamera.m_Far).Name("Far"),
		//property_var(m_BaseCamera.m_Left).Name("Left"),
		//property_var(m_BaseCamera.m_Right).Name("Right"),
		//property_var(m_BaseCamera.m_Bottom).Name("Bottom"),
		//property_var(m_BaseCamera.m_Top).Name("Top"),
		//property_var(m_BaseCamera.m_AspectRatio).Name("Aspect Ratio"),
		property_var(m_BaseCamera.m_IsPerspective).Name("IsPerspective"),
		property_var(m_IsMainCamera).Name("IsMainCamera")
} property_vend_h(TRE::Camera)