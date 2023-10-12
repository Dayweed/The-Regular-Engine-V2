#include "pch.h"
#include "Transform.h"
#include "Graphics/MeshRenderer.h"
#include "SystemManager.h"
#include "TREIncludes.h"

namespace TRE
{
	const glm::mat4 Transform::GetModelMatrix() const
	{
		const glm::vec3 rotation = glm::radians(m_Rotation);

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
		};
	}

	void TransformSystem::Update()
	{
		/*if (m_IsDirty == false)
			return;*/
	}

	void TransformSystem::OnReset()
	{

	}

	void TransformSystem::AfterEditor()
	{
		for (Entity& go : ECSManager::Instance().GetEntities<Transform>())
		{
			Transform& transform = go.get()->GetComponent<Transform>();
			if (transform.m_IsDirty)
			{
				//Update model matrix or sth (sth / sumteang/: Postions, Transforms, Rotation is set here again for the potential children)
				SetPosition(go, transform.m_Position);
				SetRotation(go, transform.m_Rotation);
				SetScale(go, transform.m_Scale);
				//Tell mesh renderer to update bounding sphere
				if (go->HasComponent<MeshRenderer>())
				{
					ECSSystemManager::Instance().GetSystem<MeshRendererSystem>()->UpdateBoundingSphere(go);
				}
				transform.m_IsDirty = false;
			}
		}
	}

	void TransformSystem::OnDestroyGO()
	{

	}
	
	void TransformSystem::Shutdown()
	{

	}
		
	void TransformSystem::SetPosition(Entity& go, const glm::vec3& position)
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
		transform.m_OldRotation = rotation;
		transform.m_IsDirty = true;

		// Update Children Rotation
		for (Entity& obj : ECSSystemManager::Instance().GetSystem<ParentingSystem>()->GetChildren(go))
		{
			glm::vec3 childRot = obj->GetComponent<Transform>().m_Rotation;
			SetRotation(obj, childRot + rotDiff);
		}
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
}