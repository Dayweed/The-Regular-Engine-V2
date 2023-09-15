#pragma once
#include "Core/ECS.h"
#include "Core/System.h"
#include "RenderObject.h"
#include "Sphere3D.h"
#include "Material.h"

namespace TRE
{
	class MeshRenderer
	{
	public:
		std::shared_ptr<RenderObject>	m_RenderObject;
		Collision::Sphere3D				m_BoundingSphere;
		bool							m_IsVisible{ true };
		bool							m_IsCulled{ false };
		bool							m_IsDirty{ false };
		std::shared_ptr<Material> m_MaterialInstance;
	};

	class MeshRendererSystem : public ECSSystem
	{
	public:
		void Update() override;
		void OnDestroyGO() override;
		void Shutdown() override;
		
		void UpdateBoundingSphere(Entity& go);

		void SetMeshRenderer(Entity& go, const std::shared_ptr<RenderObject>& renderObject);
		void SetVisible(Entity& go, bool isVisible);
		void SetCulled(Entity& go, bool isCulled);

		//const std::shared_ptr<RenderObject>& GetRenderObject(Entity& go) const;
		const bool IsVisible(Entity& go) const;
		const bool IsCulled(Entity& go) const;
		const Collision::Sphere3D& GetBoundingSphere(Entity& go) const;
	private:
		bool m_IsDirty{ false };
	};
}