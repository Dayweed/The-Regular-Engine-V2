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
	public:
		const glm::quat GetOrientation() const;
		const glm::vec3 GetUpVec() const;
		const glm::vec3 GetRightVec() const;
		const glm::vec3 GetForwardVec() const;
		const glm::vec3 GetViewDirection() const;
	};

	class Camera;

	namespace CameraHelper
	{
		void UpdateViewMatrix(Camera& camera);
		void UpdateProjectionMatrix(Camera& camera);
		void SetViewDirection(Camera& camera, const glm::vec3& direction);
		void SetViewTarget(Camera& camera, const glm::vec3& target);
	}

	class Camera : property::base, public BaseCamera
	{
	public:
		glm::vec3 m_Position{ 0.f, 0.f, 0.f }; // SSSS
		glm::vec3 m_Rotation{ 0.f, 0.f, 0.f }; // SSSS
		bool m_IsMainCamera{ false }; // SSSS
		bool m_IsDirty{ false };

		// MUST Use BOTH of this if have variables that are struct/class to serialize
		friend void to_json(nlohmann::json& j, const Camera& t) // Serialize
		{
			const float* pos = glm::value_ptr(t.m_Position);
			std::vector<float> v_pos{ pos[0], pos[1], pos[2] };
			const float* rot = glm::value_ptr(t.m_Rotation);
			std::vector<float> v_rot{ rot[0], rot[1], rot[2] };
			const float* vpsize = glm::value_ptr(t.m_ViewportSize);
			std::vector<float> v_vpsize{ vpsize[0], vpsize[1] };
			const float* focal = glm::value_ptr(t.m_FocalPoint);
			std::vector<float> v_focal{ focal[0], focal[1], focal[2] };

			j = nlohmann::json{
				{ "m_ViewportSize", v_vpsize },
				{ "m_Pitch", t.m_Pitch },
				{ "m_Yaw", t.m_Yaw },
				{ "m_Roll", t.m_Roll },
				{ "m_Fov", t.m_Fov },
				{ "m_Near", t.m_Near },
				{ "m_Far", t.m_Far },
				{ "m_FocalLength", t.m_FocalLength },
				{ "m_FocalPoint", v_focal },
				{ "m_Left", t.m_Left },
				{ "m_Right", t.m_Right },
				{ "m_Bottom", t.m_Bottom },
				{ "m_Top", t.m_Top },
				{ "m_AspectRatio", t.m_AspectRatio },
				{ "m_IsPerspective", t.m_IsPerspective },
				{ "m_IsMainCamera", t.m_IsMainCamera },
			};
		}
		friend void from_json(const nlohmann::json& j, Camera& t) // Deserialize
		{
			std::vector<float> v_vpsize{ j.at("m_ViewportSize").get<std::vector<float>>() };
			float a_vpsize[2]{ v_vpsize[0], v_vpsize[1] };
			t.m_ViewportSize = glm::make_vec2(a_vpsize);
			t.m_Pitch = j.at("m_Pitch").get<float>();
			t.m_Yaw = j.at("m_Yaw").get<float>();
			t.m_Roll = j.at("m_Roll").get<float>();
			t.m_Fov = j.at("m_Fov").get<float>();
			t.m_Near = j.at("m_Near").get<float>();
			t.m_Far = j.at("m_Far").get<float>();
			t.m_FocalLength = j.at("m_FocalLength").get<float>();
			std::vector<float> v_pnt{ j.at("m_FocalPoint").get<std::vector<float>>() };
			float a_pnt[3]{ v_pnt[0], v_pnt[1], v_pnt[2] };
			t.m_FocalPoint = glm::make_vec3(a_pnt);
			t.m_Left = j.at("m_Left").get<float>();
			t.m_Right = j.at("m_Right").get<float>();
			t.m_Bottom = j.at("m_Bottom").get<float>();
			t.m_Top = j.at("m_Top").get<float>();
			t.m_AspectRatio = j.at("m_AspectRatio").get<float>();
			t.m_IsPerspective = j.at("m_IsPerspective").get<bool>();
			t.m_IsMainCamera = j.at("m_IsMainCamera").get<bool>();

			t.m_IsDirty = true;
		}

		property_vtable()
	};
	
	class CameraSystem : public ECSSystem
	{
	public:
		void LateUpdate() override;
		void OnReset() override;
		void OnDestroyEntities() override;
		void Shutdown() override;

		void SetPosition(Entity& go, const glm::vec3& position);
		void SetRotation(Entity& go, const glm::vec3& rotation);
		void SetViewportSize(Entity& go, const glm::vec2& viewportSize);
		void SetFocalPoint(Entity& go, const glm::vec3& focalPoint);
		void SetFocalLength(Entity& go, const float focalLength);
		void SetPitch(Entity& go, const float pitch);
		void SetYaw(Entity& go, const float yaw);
		void SetRoll(Entity& go, const float roll);
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
		const glm::mat4 GetInverseViewMatrix(const Entity& go) const;
		const glm::mat4 GetInverseProjectionMatrix(const Entity& go) const;
		const glm::mat4 GetInverseViewProjectionMatrix(const Entity& go) const;
		const glm::vec2& GetViewportSize(const Entity& go) const;
		const glm::vec3& GetFocalPoint(const Entity& go) const;
		const float GetFocalLength(const Entity& go) const;
		const float GetPitch(const Entity& go) const;
		const float GetYaw(const Entity& go) const;
		const float GetRoll(const Entity& go) const;
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
		//void NormalizeOrientation(Entity& go);
	private:
		bool m_IsDirty{ false }; //Bool to update descriptor set
	};
}

property_begin(TRE::Camera)
{
		property_var(m_Position).Name("Position"),
		property_var(m_Rotation).Name("Rotation"),
		property_var(m_ViewportSize).Name("Viewport Size"),
		property_var(m_Pitch).Name("Pitch"),
		property_var(m_Yaw).Name("Yaw"),
		property_var(m_Roll).Name("Roll"),
		property_var(m_Fov).Name("FOV"),
		property_var(m_Near).Name("Near"),
		property_var(m_Far).Name("Far"),
		property_var(m_FocalLength).Name("Focal Length"),
		property_var(m_FocalPoint).Name("Focal Point"),
		property_var(m_Left).Name("Left"),
		property_var(m_Right).Name("Right"),
		property_var(m_Bottom).Name("Bottom"),
		property_var(m_Top).Name("Top"),
		property_var(m_AspectRatio).Name("Aspect Ratio"),
		property_var(m_IsPerspective).Name("IsPerspective"),
		property_var(m_IsMainCamera).Name("IsMainCamera")
} property_vend_h(TRE::Camera)