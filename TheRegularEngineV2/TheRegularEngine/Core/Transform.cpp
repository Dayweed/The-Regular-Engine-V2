#include "pch.h"
#include "Transform.h"

namespace TRE
{
	const glm::mat4 Transform::GetModelMatrix() const
	{
		const float c3 = glm::cos(m_Rotation.z);
		const float s3 = glm::sin(m_Rotation.z);
		const float c2 = glm::cos(m_Rotation.x);
		const float s2 = glm::sin(m_Rotation.x);
		const float c1 = glm::cos(m_Rotation.y);
		const float s1 = glm::sin(m_Rotation.y);
		return glm::mat4
		{
			{
				m_Scale.x * (c1 * c3 + s1 * s2 * s3),
				m_Scale.x * (c2 * s3),
				m_Scale.x * (c1 * s2 * s3 - c3 * s1),
				0.0f,
			},
			{
				m_Scale.y * (c3 * s1 * s2 - c1 * s3),
				m_Scale.y * (c2 * c3),
				m_Scale.y * (c1 * c3 * s2 + s1 * s3),
				0.0f,
			},
			{
				m_Scale.z * (c2 * s1),
				m_Scale.z * (-s2),
				m_Scale.z * (c1 * c2),
				0.0f,
			},
			{m_Position.x, -m_Position.y, m_Position.z, 1.0f} // VUlkan's y axis points down, so we need to flip it
		};
	}

	const glm::mat4 Transform::GetNormalMatrix() const
	{
		const float c3 = glm::cos(m_Rotation.z);
		const float s3 = glm::sin(m_Rotation.z);
		const float c2 = glm::cos(m_Rotation.x);
		const float s2 = glm::sin(m_Rotation.x);
		const float c1 = glm::cos(m_Rotation.y);
		const float s1 = glm::sin(m_Rotation.y);
		const glm::vec3 invScale = 1.0f / m_Scale;

		return glm::mat4
		{
			{
				invScale.x * (c1 * c3 + s1 * s2 * s3),
				invScale.x * (c2 * s3),
				invScale.x * (c1 * s2 * s3 - c3 * s1),
				0.f,
			},
			{
				invScale.y * (c3 * s1 * s2 - c1 * s3),
				invScale.y * (c2 * c3),
				invScale.y * (c1 * c3 * s2 + s1 * s3),
				0.f,
			},
			{
				invScale.z * (c2 * s1),
				invScale.z * (-s2),
				invScale.z * (c1 * c2),
				0.f,
			},
			{0.f, 0.f, 0.f, 1.f }
		};
	}
}