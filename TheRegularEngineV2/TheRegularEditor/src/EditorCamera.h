#include "Graphics/Camera.h"

namespace TRE
{
	class EditorCamera : public Camera
	{
	public:
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

		const glm::mat4& GetViewMatrix() const { return m_ViewMatrix; }
		const glm::mat4& GetProjectionMatrix() const { return m_ProjectionMatrix; }

	private:
		EditorCamera() {};
		EditorCamera(EditorCamera const&) = delete;
		void operator=(EditorCamera const&) = delete;
		void* operator new(size_t) = delete;
	};
}