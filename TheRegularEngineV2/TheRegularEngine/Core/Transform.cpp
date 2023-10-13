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

		/*const glm::vec3 rotation = glm::radians(m_Rotation);

		const float c3 = glm::cos(rotation.z);
		const float s3 = glm::sin(rotation.z);
		const float c2 = glm::cos(rotation.x);
		const float s2 = glm::sin(rotation.x);
		const float c1 = glm::cos(rotation.y);
		const float s1 = glm::sin(rotation.y);
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
			{m_Position.x, m_Position.y, m_Position.z, 1.0f}
		};*/
	}

	void Transform::DecomposeWorldMatrix(const glm::mat4& newWorld)
	{
		/*glm::vec3 skew;
		glm::vec4 perspective;
		glm::quat orientation;
		glm::decompose(newWorld, m_Scale, orientation, m_Position, skew, perspective);
		m_Rotation = glm::eulerAngles(orientation);*/

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

	void Transform::UpdateLocalData(const Transform& parent)
	{
		m_LocalScale = m_Scale / parent.m_Scale;
		glm::quat parentRot = glm::quat(glm::radians(parent.m_Rotation));
		glm::quat rot = glm::quat(glm::radians(m_Rotation));
		glm::quat localRot = glm::inverse(parentRot) * rot;
		m_LocalRotation = glm::eulerAngles(localRot);
		m_LocalPosition = m_Position - parent.m_Position;
		m_LocalPosition = glm::rotate(parentRot, m_LocalPosition);
		m_LocalPosition *= m_LocalScale;

		std::cout << "Local Position: " << m_LocalPosition.x << ", " << m_LocalPosition.y << ", " << m_LocalPosition.z << std::endl;
		std::cout << "Local Rotation: " << m_LocalRotation.x << ", " << m_LocalRotation.y << ", " << m_LocalRotation.z << std::endl;
		std::cout << "Local Scale: " << m_LocalScale.x << ", " << m_LocalScale.y << ", " << m_LocalScale.z << std::endl;
	}

	void Transform::UpdateLocalMatrix(Entity& parent)
	{
		//For start of scene
		Transform& parentTransform = parent->GetComponent<Transform>();
		parentTransform.CalculateWorldMatrix();
		CalculateWorldMatrix();

		glm::mat4 invertedParentWorldTransform = glm::affineInverse(parentTransform.m_WorldXform);
		m_LocalXform = invertedParentWorldTransform * m_WorldXform;

		m_LocalPosition = glm::vec3(m_LocalXform[3]);
		m_LocalScale = glm::vec3(glm::length(m_LocalXform[0]), glm::length(m_LocalXform[1]), glm::length(m_LocalXform[2]));
		m_LocalRotation = glm::eulerAngles(glm::quat(m_LocalXform));

		std::cout << "Local Position: " << m_LocalPosition.x << ", " << m_LocalPosition.y << ", " << m_LocalPosition.z << std::endl;
		std::cout << "Local Rotation: " << m_LocalRotation.x << ", " << m_LocalRotation.y << ", " << m_LocalRotation.z << std::endl;
		std::cout << "Local Scale: " << m_LocalScale.x << ", " << m_LocalScale.y << ", " << m_LocalScale.z << std::endl;
	}

	void TransformSystem::Update()
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

	void TransformSystem::OnReset()
	{

	}

	void TransformSystem::OnDestroyGO()
	{

	}
	
	void TransformSystem::Shutdown()
	{

	}
		
	/*void TransformSystem::SetPosition(Entity& go, const glm::vec3& position)
	{
		m_IsDirty = true;

		Transform& transform = go.get()->GetComponent<Transform>();
		glm::vec3 posDiff = position - transform.m_OldPosition;
		transform.m_Position = position;
		transform.m_OldPosition = position;
		transform.m_IsDirty = true;

		// Update Children Position
		for (Entity& obj : ECSSystemManager::Instance().GetSystem<ParentingSystem>()->GetChildren(go))
		{
			glm::vec3 childPos = obj->GetComponent<Transform>().m_Position;
			SetPosition(obj, childPos + posDiff);
		}
	}

	void TransformSystem::SetRotation(Entity& go, const glm::vec3& rotation)
	{
		m_IsDirty = true;
		Transform& transform = go.get()->GetComponent<Transform>();
		glm::vec3 rotDiff = rotation - transform.m_OldRotation;
		transform.m_Rotation = rotation;
		transform.m_IsDirty = true;

		// Update Children Rotation
		for (Entity& obj : ECSSystemManager::Instance().GetSystem<ParentingSystem>()->GetChildren(go))
		{
			Transform& childTransform{ obj->GetComponent<Transform>() };
			glm::vec3 childRot = childTransform.m_Rotation;
			SetRotation(obj, childRot + rotDiff);
			// Update Position
			if (transform.m_Rotation != transform.m_OldRotation)
			{
				glm::vec4 localPos{ childTransform.m_Position - transform.m_Position, 1 };
				const glm::vec3 scale = {1, 1, 1};// (transform.m_Scale);
				const glm::vec3 rotation = glm::radians(transform.m_Rotation);
				const float c3 = glm::cos(rotation.z);
				const float s3 = glm::sin(rotation.z);
				const float c2 = glm::cos(rotation.x);
				const float s2 = glm::sin(rotation.x);
				const float c1 = glm::cos(rotation.y);
				const float s1 = glm::sin(rotation.y);
				glm::mat4 rotMat
				{
					{
						scale.x * (c1 * c3 + s1 * s2 * s3),
						scale.x * (c2 * s3),
						scale.x * (c1 * s2 * s3 - c3 * s1),
						0.0f,
					},
					{
						scale.y * (c3 * s1 * s2 - c1 * s3),
						scale.y * (c2 * c3),
						scale.y * (c1 * c3 * s2 + s1 * s3),
						0.0f,
					},
					{
						scale.z * (c2 * s1),
						scale.z * (-s2),
						scale.z * (c1 * c2),
						0.0f,
					},
					{0, 0, 0, 1.0f}
				};
				SetPosition(obj, transform.m_Position + glm::vec3{ glm::inverse(rotMat) * localPos });
			}
		}

		transform.m_OldRotation = rotation;
	}

	void TransformSystem::SetScale(Entity& go, const glm::vec3& scale)
	{
		m_IsDirty = true;

		Transform& transform = go.get()->GetComponent<Transform>();
		glm::vec3 scaDiff = scale / transform.m_OldScale;
		transform.m_Scale = scale;
		transform.m_OldScale = scale;
		transform.m_IsDirty = true;

		// Update Children Scale
		for (Entity& obj : ECSSystemManager::Instance().GetSystem<ParentingSystem>()->GetChildren(go))
		{
			glm::vec3 childSca = obj->GetComponent<Transform>().m_Scale;
			SetScale(obj, childSca * scaDiff);
		}
	}

	const glm::vec3& TransformSystem::GetPosition(const Entity& go) const
	{
		return go.get()->GetComponent<Transform>().m_Position;
	}

	const glm::vec3& TransformSystem::GetRotation(const Entity& go) const
	{
		return go.get()->GetComponent<Transform>().m_Rotation;
	}

	const glm::vec3& TransformSystem::GetScale(const Entity& go) const
	{
		return go.get()->GetComponent<Transform>().m_Scale;
	}

	const glm::mat4 TransformSystem::GetModelMatrix(const Entity& go) const
	{
		//Next time then i do this
		(void)go;
		return glm::mat4(1.f);
	}
	*/
}