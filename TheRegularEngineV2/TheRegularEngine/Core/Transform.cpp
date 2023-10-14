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
		glm::mat4 scaleMat = glm::scale(glm::mat4(1.0f), m_Scale);
		glm::mat4 translationMat = glm::translate(glm::mat4(1.0f), m_Position);

		m_WorldXform = translationMat * rotationMat * scaleMat;
	}

	void Transform::DecomposeWorldMatrix(const glm::mat4 newWorld)
	{
		m_Position = glm::vec3(newWorld[3]);
		m_Scale = glm::vec3(glm::length(newWorld[0]), glm::length(newWorld[1]), glm::length(newWorld[2]));
		m_Rotation = glm::degrees(glm::eulerAngles(glm::quat(newWorld)));

		m_WorldXform = newWorld;
	}

	const glm::mat4 Transform::CalculateLocalMatrix()
	{
		glm::quat rotation = glm::quat(glm::radians(m_LocalRotation));
		glm::mat4 rotationMat = glm::mat4_cast(rotation);
		glm::mat4 scaleMat = glm::scale(glm::mat4(1.0f), m_LocalScale);
		glm::mat4 translationMat = glm::translate(glm::mat4(1.0f), m_LocalPosition);

		return translationMat * rotationMat * scaleMat;
	}

	void Transform::UpdateLocalMatrix(Entity& parent)
	{
		//For start of scene
		Transform& parentTransform = parent->GetComponent<Transform>();
		parentTransform.CalculateWorldMatrix();
		CalculateWorldMatrix();

		glm::mat4 invertedParentWorldTransform = glm::affineInverse(parentTransform.m_WorldXform);
		const glm::mat4 localXform = invertedParentWorldTransform * m_WorldXform;

		m_LocalPosition = glm::vec3(localXform[3]);
		m_LocalScale = glm::vec3(glm::length(localXform[0]), glm::length(localXform[1]), glm::length(localXform[2]));
		m_LocalRotation = glm::degrees(glm::eulerAngles(glm::quat(localXform)));
	}

	void TransformSystem::Update()
	{

	}

	void TransformSystem::AfterEditor()
	{
		for (Entity& go : ECSManager::Instance().GetEntities<Transform>())
		{
			Transform& transform = go.get()->GetComponent<Transform>();
			if (transform.m_IsDirty)
			{
				transform.CalculateWorldMatrix();

				//Tell mesh renderer to update bounding sphere
				if (go->HasComponent<MeshRenderer>())
				{
					ECSSystemManager::Instance().GetSystem<MeshRendererSystem>()->UpdateBoundingSphere(go);
				}

				transform.m_IsDirty = false;
			}
		}
	}

	void TransformSystem::OnReset()
	{

	}

	void TransformSystem::OnDestroyGO()
	{

	}
	
	void TransformSystem::Shutdown()
	{

	}
}