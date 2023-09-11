#include "Sphere3D.h"

namespace Collision
{
	Sphere3D::Sphere3D(const glm::vec3& center, float radius) : m_Center(center), m_Radius(radius) {}
	
	void Sphere3D::Create(const glm::vec3* vertices, const std::size_t count)
	{
		//Ritters method
		glm::vec3 minX, maxX, minY, maxY, minZ, maxZ;
		minX.x = minY.y = minZ.z = std::numeric_limits<float>::max();
		maxX.x = maxY.y = maxZ.z = std::numeric_limits<float>::min();

		for (std::size_t i = 0; i < count; ++i)
		{
			if (vertices[i].x < minX.x)
				minX = vertices[i];
			if (vertices[i].x > maxX.x)
				maxX = vertices[i];
			if (vertices[i].y < minY.y)
				minY = vertices[i];
			if (vertices[i].y > maxY.y)
				maxY = vertices[i];
			if (vertices[i].z < minZ.z)
				minZ = vertices[i];
			if (vertices[i].z > maxZ.z)
				maxZ = vertices[i];
		}

		const float distX = glm::distance(minX, maxX);
		const float distY = glm::distance(minY, maxY);
		const float distZ = glm::distance(minZ, maxZ);

		glm::vec3 center = glm::vec3(0, 0, 0);
		float radius = 0.f;

		//X biggest
		if (distX >= distY && distX >= distZ)
		{
			center = 0.5f * (minX + maxX);
			radius = glm::length(maxX - center);
		}
		//Y biggest
		else if (distY > distX && distY > distZ)
		{
			center = 0.5f * (minY + maxY);
			radius = glm::length(maxY - center);
		}
		//Z biggest
		else if (distZ > distX && distZ > distY)
		{
			center = 0.5f * (minZ + maxZ);
			radius = glm::length(maxZ - center);
		}

		//Pass 2
		for (std::size_t i = 0; i < count; ++i)
		{
			glm::vec3 p = vertices[i];
			glm::vec3 uVec = p - center;
			//Outside of circle
			if (glm::length(uVec) > radius)
			{
				glm::vec3 uHat = glm::normalize(uVec);
				glm::vec3 pPrime = center - radius * uHat;
				center = 0.5f * (pPrime + p);
				radius = glm::length(p - center);
			}
		}

		m_Center = center;
		m_Radius = radius;
	}

	void Sphere3D::Create(const std::vector<glm::vec3>& vertices)
	{
		Create(vertices.data(), vertices.size());
	}

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