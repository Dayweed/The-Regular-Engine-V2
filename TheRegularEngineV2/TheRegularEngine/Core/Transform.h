#pragma once
#include "pch.h"
#include "ECS.h"
#include "System.h"

namespace TRE
{
	class Transform
	{
	public:
		glm::vec3	m_Position{ 0.f,0.f,0.f };
		glm::vec3	m_Rotation{ 0.f,0.f,0.f };
		glm::vec3	m_Scale{ 1.f,1.f,1.f };
		bool		m_IsDirty{ false };
		const glm::mat4 GetModelMatrix() const;
		const glm::mat4 GetNormalMatrix() const;
	};

	class TransformSystem : public ECSSystem
	{
	public:
		void Update() override;
		void OnDestroyGO() override;
		void Shutdown() override;

		void SetPosition(Entity& go, const glm::vec3& position);
		void SetRotation(Entity& go, const glm::vec3& rotation);
		void SetScale(Entity& go, const glm::vec3& scale);

		const glm::vec3& GetPosition(const Entity& go) const;
		const glm::vec3& GetRotation(const Entity& go) const;
		const glm::vec3& GetScale(const Entity& go) const;
		const glm::mat4 GetModelMatrix(const Entity& go) const;
	private:
		bool m_IsDirty{ false };
	};
}