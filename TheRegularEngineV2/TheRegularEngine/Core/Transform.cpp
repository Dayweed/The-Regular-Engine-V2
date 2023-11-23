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
		m_Rotation.x = fmodf(m_Rotation.x, 360.0f);
		m_Rotation.y = fmodf(m_Rotation.y, 360.0f);
		m_Rotation.z = fmodf(m_Rotation.z, 360.0f);

		/*glm::vec3 tempRotation1 = glm::radians(m_Rotation);
		glm::vec3 tempRotation2 = glm::radians(m_RotationOld);

		glm::vec3 tempRotation = glm::vec3(tempRotation1.x - tempRotation2.x, tempRotation1.y - tempRotation2.y, tempRotation1.z - tempRotation2.z);

		glm::quat rotationX = glm::angleAxis(tempRotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
		glm::quat rotationY = glm::angleAxis(tempRotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
		glm::quat rotationZ = glm::angleAxis(tempRotation.z, glm::vec3(0.0f, 0.0f, 1.0f));

		glm::quat rotationCombined = m_OldRotation * rotationZ * rotationY * rotationX;

		glm::quat slerpRotation = rotationCombined;

		glm::mat4 rotationMat = glm::toMat4(slerpRotation);
		glm::mat4 scaleMat = glm::scale(glm::identity<glm::mat4>(), m_Scale);
		glm::mat4 translationMat = glm::translate(glm::identity<glm::mat4>(), m_Position);

		m_WorldXform = translationMat * rotationMat * scaleMat;

		m_OldRotation = slerpRotation;
		m_RotationOld = m_Rotation = glm::degrees(glm::eulerAngles(slerpRotation));*/

		glm::vec3 tempRotation = glm::radians(m_Rotation);
		glm::mat4 rotationMat = glm::toMat4(glm::quat(tempRotation));
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

		glm::mat4 inverseParentWorldTransform = glm::inverse(parent.m_WorldXform);
		const glm::mat4 localXform = inverseParentWorldTransform * m_WorldXform;

		m_LocalPosition = glm::vec3(localXform[3]);
		m_LocalScale = glm::vec3(glm::length(localXform[0]), glm::length(localXform[1]), glm::length(localXform[2]));
		m_LocalRotation = glm::degrees(glm::eulerAngles(glm::quat(glm::mat3(localXform))));
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

				// Overwrite prefab ["TRE::Transform/Position"] if it is a prefab
				if (go->HasComponent<Prefabing>())
				{
					go->GetComponent<Prefabing>().m_Overrides["Transform"].emplace("TRE::Transform/Position");
					go->GetComponent<Prefabing>().m_Overrides["Transform"].emplace("TRE::Transform/Rotation");
					go->GetComponent<Prefabing>().m_Overrides["Transform"].emplace("TRE::Transform/Scale");
				}
			}
		}
	}

	glm::vec3 TransformSystem::RotateMatrix(glm::vec3 vector, glm::vec3 rotation)
	{
		glm::quat rotationQuat = glm::quat(glm::radians(rotation));
		glm::mat3 rotationMat = glm::mat3_cast(rotationQuat);

		return  rotationMat * vector;
	}
}