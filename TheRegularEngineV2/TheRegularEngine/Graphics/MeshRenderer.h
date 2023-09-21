#pragma once
#include "Core/ECS.h"
#include "Core/System.h"
#include "Core/SystemManager.h"
#include "RenderObject.h"
#include "Sphere3D.h"
#include "Material.h"
#include "Resource/ResourceManager.h"

namespace TRE
{
	class MeshRenderer
	{
	public:
		std::shared_ptr<RenderObject>	m_RenderObject;
		std::shared_ptr<Material>		m_MaterialInstance;
		Collision::Sphere3D				m_BoundingSphere;
		bool							m_IsVisible{ true };
		bool							m_IsCulled{ false };
		bool							m_IsDirty{ false };

		friend void to_json(nlohmann::json& j, const MeshRenderer& t)
		{
			j = nlohmann::json{
				{ "ASSET_GEOM_m_RenderObject", t.m_RenderObject->GetHandleHex()},
				{ "ASSET_MAT_m_MaterialInstance", t.m_MaterialInstance->GetHandleHex()},
				{ "m_IsVisible", t.m_IsVisible },	
			};
		}
		friend void from_json(const nlohmann::json& j, MeshRenderer& t)
		{
			std::string roString = j.at("ASSET_GEOM_m_RenderObject").get<std::string>();
			ResourceHandle roHandle = Resource::GetGUIDFromHex(roString);
			std::string matString = j.at("ASSET_MAT_m_MaterialInstance").get<std::string>();
			ResourceHandle matHandle = Resource::GetGUIDFromHex(matString);

			if (auto renderObject = ResourceManager::Instance().GetResource<RenderObject>(roHandle); renderObject)
			{
				t.m_RenderObject = renderObject;
			}
			else
			{
				t.m_RenderObject = RenderObject::Deserialize(roString);

				if(t.m_RenderObject == nullptr)
					TRE_CORE_CRITICAL(roString + ".geom not found!");
			}

			if (auto material = ResourceManager::Instance().GetResource<Material>(matHandle); material)
			{
				t.m_MaterialInstance = material;
			}
			else
			{
				t.m_MaterialInstance = Material::Deserialize(matString);

				if(t.m_MaterialInstance == nullptr)
					TRE_CORE_CRITICAL(matString + ".mat not found!");
			}

			t.m_IsVisible = j.at("m_IsVisible").get<bool>();
			t.m_IsDirty = true;
		}
	};

	class MeshRendererSystem : public ECSSystem
	{
	public:
		void Update() override;
		void OnDestroyGO() override;
		void Shutdown() override;
		
		void UpdateBoundingSphere(Entity& go);

		void SetMeshRenderer(Entity& go, const std::shared_ptr<RenderObject>& renderObject);
		void SetMaterial(Entity& go, const std::shared_ptr<Material>& material);
		void SetVisible(Entity& go, bool isVisible);
		void SetCulled(Entity& go, bool isCulled);

		//const std::shared_ptr<RenderObject>& GetRenderObject(Entity& go) const;
		const bool IsVisible(Entity& go) const;
		const bool IsCulled(Entity& go) const;
		const Collision::Sphere3D& GetBoundingSphere(Entity& go) const;

		void SetDirty(const bool dirty){m_IsDirty = dirty;}
		const bool IsDirty() const { return m_IsDirty; }

		friend MeshRenderer;
	private:
		bool m_IsDirty{ false };
	};
}