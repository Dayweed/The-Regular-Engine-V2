#include "Graphics/Camera.h"

namespace TRE
{
	class EditorCamera : private Camera
	{
	public:
		static EditorCamera& Instance()
		{
			static EditorCamera instance;
			return instance;
		}

		void Init();
		void Update();
		void Shutdown();

		void SetPosition(const glm::vec3& position) { m_Position = position; }
		void SetRotation(const glm::vec3& rotation) { m_Rotation = rotation; }

		const glm::vec3& GetPosition() const { return m_Position; }
		const glm::vec3& GetRotation() const { return m_Rotation; }
		const glm::mat4& GetViewMatrix() const { return m_ViewMatrix; }
		const glm::mat4& GetProjectionMatrix() const { return m_ProjectionMatrix; }

	private:
		EditorCamera() {};
		EditorCamera(EditorCamera const&) = delete;
		void operator=(EditorCamera const&) = delete;
		void* operator new(size_t) = delete;
	};
}