#pragma once
#include "glm/glm.hpp"

namespace Collision
{
	extern class Box3D;
	extern class Sphere3D;

	class Ray3D
	{
	public:
		Ray3D(const glm::vec3& origin, const glm::vec3& direction);

		void SetOrigin(const glm::vec3& origin);
		void SetDirection(const glm::vec3& direction);
		
		const glm::vec3& GetOrigin() const;
		const glm::vec3& GetDirection() const;

		glm::vec3 GetPoint(float distance) const;

		bool Intersects(const glm::vec3& point, float* rt = nullptr) const;
		bool Intersects(const Box3D& box, float* rt = nullptr) const;
		bool Intersects(const Sphere3D& sphere, float* rt = nullptr) const;
	private:
		glm::vec3 m_Origin;
		glm::vec3 m_Direction;
	};
}