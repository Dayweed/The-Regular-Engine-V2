#pragma once
#include "glm/glm.hpp"

namespace Collision
{
	class Sphere3D
	{
	public:
		Sphere3D() = default;
		Sphere3D(const glm::vec3& center, float radius);

		void SetCenter(const glm::vec3& center);
		void SetRadius(float radius);

		const glm::vec3& GetCenter() const;
		float GetRadius() const;
	private:
		glm::vec3 m_Center;
		float m_Radius;
	};
}