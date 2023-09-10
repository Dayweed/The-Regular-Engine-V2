#include "Sphere3D.h"

namespace Collision
{
	Sphere3D::Sphere3D(const glm::vec3& center, float radius) : m_Center(center), m_Radius(radius) {}

	void Sphere3D::SetCenter(const glm::vec3& center)
	{
		m_Center = center;
	}

	void Sphere3D::SetRadius(float radius)
	{
		m_Radius = radius;
	}

	const glm::vec3& Sphere3D::GetCenter() const
	{
		return m_Center;
	}

	float Sphere3D::GetRadius() const
	{
		return m_Radius;
	}
}