#include "Ray3D.h"
#include "Box3D.h"
#include "Sphere3D.h"

namespace Collision
{
	Ray3D::Ray3D(const glm::vec3& origin, const glm::vec3& direction) : m_Origin(origin), m_Direction(direction) {}

	void Ray3D::SetOrigin(const glm::vec3& origin)
	{
		m_Origin = origin;
	}

	void Ray3D::SetDirection(const glm::vec3& direction)
	{
		m_Direction = direction;
	}

	const glm::vec3& Ray3D::GetOrigin() const
	{
		return m_Origin;
	}

	const glm::vec3& Ray3D::GetDirection() const
	{
		return m_Direction;
	}

	glm::vec3 Ray3D::GetPoint(float distance) const
	{
		return m_Origin + (m_Direction * distance);
	}

	bool Ray3D::Intersects(const glm::vec3& point, float* rt) const
	{
		const glm::vec3 v = point - m_Origin;
		if (glm::dot(v, m_Direction) >= 0.f)
		{
			if (rt != nullptr)
				*rt = glm::length(v) / glm::length(m_Direction);
			return true;
		}
		return false;
	}

	bool Ray3D::Intersects(const Box3D& box, float* rt) const
	{
		const glm::vec3 min = box.GetMin();
		const glm::vec3 max = box.GetMax();
		const glm::vec3 invDir = 1.f / m_Direction;
		const glm::vec3 t0 = (min - m_Origin) * invDir;
		const glm::vec3 t1 = (max - m_Origin) * invDir;
		const glm::vec3 tmin = glm::min(t0, t1);
		const glm::vec3 tmax = glm::max(t0, t1);
		const float tNear = glm::max(glm::max(tmin.x, tmin.y), tmin.z);
		const float tFar = glm::min(glm::min(tmax.x, tmax.y), tmax.z);
		if (tNear > 0.f && tNear <= tFar)
		{
			if (rt != nullptr)
				*rt = tNear;
			return true;
		}
		return false;
	}

	bool Ray3D::Intersects(const Sphere3D& sphere, float* rt) const
	{
		const glm::vec3 v = sphere.GetCenter() - m_Origin;
		const float b = glm::dot(v, m_Direction);
		const float c = glm::dot(v, v) - (sphere.GetRadius() * sphere.GetRadius());
		if (c > 0.f && b > 0.f)
			return false;
		const float discriminant = (b * b) - c;
		if (discriminant < 0.f)
			return false;
		if (rt != nullptr)
			*rt = -b - sqrt(discriminant);
		return true;
	}
}