#include "Box3D.h"

namespace Collision
{
	Box3D::Box3D(const glm::vec3& center, const glm::vec3& extents) : m_Center(center), m_Extents(extents) {}

	void Box3D::SetCenter(const glm::vec3& center)
	{
		m_Center = center;
	}

	void Box3D::SetExtents(const glm::vec3& extents)
	{
		m_Extents = extents;
	}

	void Box3D::SetMin(const glm::vec3& min)
	{
		const glm::vec3 max = m_Center + m_Extents;
		m_Center = (min + max) * 0.5f;
		m_Extents = (max - min) * 0.5f;
	}

	void Box3D::SetMax(const glm::vec3& max)
	{
		const glm::vec3 min = m_Center - m_Extents;
		m_Center = (min + max) * 0.5f;
		m_Extents = (max - min) * 0.5f;
	}

	const glm::vec3& Box3D::GetCenter() const
	{
		return m_Center;
	}

	const glm::vec3& Box3D::GetExtents() const
	{
		return m_Extents;
	}

	const glm::vec3 Box3D::GetMin() const
	{
		return m_Center - m_Extents;
	}

	const glm::vec3 Box3D::GetMax() const
	{
		return m_Center + m_Extents;
	}
}