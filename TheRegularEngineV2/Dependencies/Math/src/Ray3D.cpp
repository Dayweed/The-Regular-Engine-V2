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
		float a = glm::dot(m_Direction, m_Direction);
		float b = 2.f * glm::dot(m_Origin - sphere.GetCenter(), m_Direction);
		float c = glm::dot(m_Origin - sphere.GetCenter(), m_Origin - sphere.GetCenter()) - (sphere.GetRadius() * sphere.GetRadius());

		//Denominator zero
		if (abs(a) < FLT_EPSILON)
			return false;

		float discriminant = static_cast<float>(pow(b, 2)) - 4.f * a * c;

		if (discriminant < 0.f)
			return false;

		if (abs(discriminant) < FLT_EPSILON)
			return true;

		float t0 = (-b + sqrt(discriminant)) / (2.f * a);
		float t1 = (-b - sqrt(discriminant)) / (2.f * a);

		if (t0 >= 0.f && t1 >= 0.f)
		{
			if (rt)
				*rt = std::min(t0, t1);
			return true;
		}
		else if (t0 >= 0.f)
		{
			if (rt)
				*rt = t0;
			return true;
		}
		else if (t1 >= 0.f)
		{
			if (rt)
				*rt = t1;
			return true;
		}

		return false;
	}
}