#pragma once
#include "glm/glm.hpp"

namespace Collision
{
	class Box3D
	{
	public:
		Box3D() = default;
		Box3D(const glm::vec3& center, const glm::vec3& extents);

		void SetCenter(const glm::vec3& center);
		void SetExtents(const glm::vec3& extents);
		void SetMin(const glm::vec3& min);
		void SetMax(const glm::vec3& max);

		const glm::vec3& GetCenter() const;
		const glm::vec3& GetExtents() const;
		const glm::vec3& GetMin() const;
		const glm::vec3& GetMax() const;
	private:
		glm::vec3 m_Center;
		glm::vec3 m_Extents;
	};
}
