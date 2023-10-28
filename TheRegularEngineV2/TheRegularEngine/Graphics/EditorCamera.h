#include "Graphics/Camera.h"

namespace TRE
{
	class EditorCamera
	{
	public:
		BaseCamera m_BaseCamera;
	private:
		glm::vec3 m_Position{ 0.f, 0.f, 0.f };
		glm::vec3 m_Rotation{ 0.f, 0.f, 0.f };

		bool m_IsDirty{ false };
	public:
		static EditorCamera& Instance()
		{
			static EditorCamera instance;
			return instance;
		}

		void Init();
		void Update();
		void Shutdown();

		void SetFocalPoint(const glm::vec3& focalPoint);
		void SetFocalDistance(const float distance);
		void SetPitch(const float pitch);
		void SetYaw(const float yaw);
		void SetRoll(const float roll);

		const glm::mat4& GetViewMatrix() const { return m_BaseCamera.m_ViewMatrix; }
		const glm::mat4 GetInverseViewMatrix() const { return glm::inverse(m_BaseCamera.m_ViewMatrix); }
		const glm::mat4& GetProjectionMatrix() const { return m_BaseCamera.m_ProjectionMatrix; }
		const glm::mat4 GetInverseProjectionMatrix() const { return glm::inverse(m_BaseCamera.m_ProjectionMatrix); }
		const glm::mat4 GetViewProjectionMatrix() const { return m_BaseCamera.m_ProjectionMatrix * m_BaseCamera.m_ViewMatrix; }
		const glm::mat4 GetInverseViewProjectionMatrix() const { return glm::inverse(m_BaseCamera.m_ProjectionMatrix * m_BaseCamera.m_ViewMatrix); }
	
		const glm::vec3& GetPosition() const { return m_Position; }
		const glm::vec3& GetRotation() const { return m_Rotation; }

		void SetDirection(const glm::vec3& position);
		void AssignToMainCamera();

		void Serialize();
		void Deserialize();
	private:
		void SetPosition(const glm::vec3& position);
	private:
		EditorCamera() {};
		EditorCamera(EditorCamera const&) = delete;
		void operator=(EditorCamera const&) = delete;
		void* operator new(size_t) = delete;
	};
}