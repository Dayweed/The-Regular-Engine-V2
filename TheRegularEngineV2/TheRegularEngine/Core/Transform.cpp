#include "pch.h"
#include "Transform.h"
#include "Graphics/MeshRenderer.h"
#include "SystemManager.h"
#include "TREIncludes.h"

#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/matrix_inverse.hpp>

namespace TRE
{
	void Transform::CalculateWorldMatrix()
	{
		glm::quat rotation = glm::quat(glm::radians(m_Rotation));
		glm::mat4 rotationMat = glm::mat4_cast(rotation);
		glm::mat4 scaleMat = glm::scale(glm::identity<glm::mat4>(), m_Scale);
		glm::mat4 translationMat = glm::translate(glm::identity<glm::mat4>(), m_Position);

		m_WorldXform = translationMat * rotationMat * scaleMat;
	}

	void Transform::DecomposeWorldMatrix(const glm::mat4 newWorld)
	{
		m_Position = glm::vec3(newWorld[3]);
		m_Scale = glm::vec3(glm::length(newWorld[0]), glm::length(newWorld[1]), glm::length(newWorld[2]));
		m_Rotation = glm::degrees(glm::eulerAngles(glm::quat(glm::mat3(newWorld))));

		m_WorldXform = newWorld;
	}

	const glm::mat4 Transform::CalculateLocalMatrix()
	{
		glm::quat rotation = glm::quat(glm::radians(m_LocalRotation));
		glm::mat4 scaleMat = glm::scale(glm::identity<glm::mat4>(), m_LocalScale);
		glm::mat4 rotationMat = glm::mat4_cast(rotation);
		glm::mat4 translationMat = glm::translate(glm::identity<glm::mat4>(), m_LocalPosition);

		return translationMat * rotationMat * scaleMat;
	}

	void Transform::UpdateLocalData(Transform& parent)
	{
		//For start of scene
		parent.CalculateWorldMatrix();
		CalculateWorldMatrix();

		glm::mat4 inverseParentWorldTransform = glm::affineInverse(parent.m_WorldXform);
		const glm::mat4 localXform = inverseParentWorldTransform * m_WorldXform;

		m_LocalPosition = glm::vec3(localXform[3]);
		m_LocalScale = glm::vec3(glm::length(localXform[0]), glm::length(localXform[1]), glm::length(localXform[2]));
		m_LocalRotation = glm::degrees(glm::eulerAngles(glm::quat(localXform)));
	}

	void TransformSystem::LateUpdate()
	{
		for (Entity& go : ECSManager::Instance().GetEntities<Transform>())
		{
			Transform& transform = go.get()->GetComponent<Transform>();
			if (transform.m_IsDirty)
			{
				transform.CalculateWorldMatrix();
				transform.m_IsDirty = false;
			}
		}
	}

	glm::vec3 TransformSystem::RotateMatrix(glm::vec3 vector, glm::vec3 rotation)
	{
		glm::quat rotationQuat = glm::quat(glm::radians(rotation));
		glm::mat4 rotationMat = glm::mat4_cast(rotationQuat);
		glm::mat4 translationMat = glm::translate(glm::identity<glm::mat4>(), vector);
		glm::mat4 newMat = rotationMat * translationMat;

		return glm::vec3(newMat[3]);
	}
}