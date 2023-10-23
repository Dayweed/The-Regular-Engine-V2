#include "pch.h"
#include "MeshRenderer.h"
#include "Core/Transform.h"

namespace TRE
{
	void MeshRendererSystem::Update()
	{
		/*if (m_IsDirty == false)
			return;*/

		for (Entity& go : ECSManager::Instance().GetEntities<MeshRenderer>())
		{
			MeshRenderer& meshRenderer = go->GetComponent<MeshRenderer>();
			Transform& transform = go->GetComponent<Transform>();
			if (meshRenderer.m_IsDirty)
			{
				meshRenderer.m_IsDirty = false;
			}

			if (transform.m_IsDirty)
			{
				UpdateBoundingSphere(go);
				//transform.m_IsDirty = false;
			}
		}
	}

	void MeshRendererSystem::OnReset()
	{

	}

	void MeshRendererSystem::OnDestroyEntities()
	{

	}

	void MeshRendererSystem::Shutdown()
	{

	}

	void MeshRendererSystem::UpdateBoundingSphere(Entity& go)
	{
		MeshRenderer& meshRenderer = go.get()->GetComponent<MeshRenderer>();
		Transform& transform = go.get()->GetComponent<Transform>();
		
		if (meshRenderer.m_RenderObject == nullptr)
			return;

		meshRenderer.m_BoundingSphere = meshRenderer.m_RenderObject->GetBoundingSphere();
		meshRenderer.m_BoundingSphere.SetCenter(transform.m_Position);

		float maxScale = std::max(transform.m_Scale.x, std::max(transform.m_Scale.y, transform.m_Scale.z));

		meshRenderer.m_BoundingSphere.SetRadius(meshRenderer.m_RenderObject->GetBoundingSphere().GetRadius() * maxScale);

		meshRenderer.m_IsDirty = true;
	}

	void MeshRendererSystem::SetMeshRenderer(Entity& go, const std::shared_ptr<RenderObject>& renderObject)
	{
		m_IsDirty = true;

		MeshRenderer& meshRenderer = go->GetComponent<MeshRenderer>();
		meshRenderer.m_RenderObject = renderObject;
		meshRenderer.m_IsDirty = true;

		//I think i need to update vertex & index buffer

		UpdateBoundingSphere(go);
	}

	void MeshRendererSystem::SetMaterial(Entity& go, const std::shared_ptr<Material>& material)
	{
		m_IsDirty = true;
	
		MeshRenderer& meshRenderer = go->GetComponent<MeshRenderer>();
		meshRenderer.m_MaterialInstance = material;
		meshRenderer.m_IsDirty = true;
	}

	void MeshRendererSystem::SetVisible(Entity& go, bool isVisible)
	{
		m_IsDirty = true;

		MeshRenderer& meshRenderer = go->GetComponent<MeshRenderer>();
		meshRenderer.m_IsVisible = isVisible;
		meshRenderer.m_IsDirty = true;
	}
	
	void MeshRendererSystem::SetCulled(Entity& go, bool isCulled)
	{
		m_IsDirty = true;

		MeshRenderer& meshRenderer = go->GetComponent<MeshRenderer>();
		meshRenderer.m_IsCulled = isCulled;
		meshRenderer.m_IsDirty = true;
	}

	const bool MeshRendererSystem::IsVisible(Entity& go) const
	{
		return go->GetComponent<MeshRenderer>().m_IsVisible;
	}

	const bool MeshRendererSystem::IsCulled(Entity& go) const
	{
		return go->GetComponent<MeshRenderer>().m_IsCulled;
	}

	const Collision::Sphere3D& MeshRendererSystem::GetBoundingSphere(Entity& go) const
	{
		return go->GetComponent<MeshRenderer>().m_BoundingSphere;
	}
}