#pragma once
#include "Core/ECS.h"
#include "Core/System.h"
#include "Core/SystemManager.h"
#include "RenderObject.h"
#include "Sphere3D.h"
#include "Material.h"
#include "Assets/AssetManager.h"

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
			std::stringstream ss;
			ss << std::hex << t.m_RenderObject->GetHandle();
			std::string roHandle = ss.str();
			ss.str("");
			ss << std::hex << t.m_MaterialInstance->GetHandle();
			std::string matHandle = ss.str();

			j = nlohmann::json{
				{ "ASSET_GEOM_m_RenderObject", roHandle},
				{ "ASSET_MAT_m_MaterialInstance", matHandle},
				{ "m_IsVisible", t.m_IsVisible },	
			};
		}
		friend void from_json(const nlohmann::json& j, MeshRenderer& t)
		{
			/*std::string roString = j.at("ASSET_GEOM_m_RenderObject").get<std::string>();
			AssetHandle roHandle = std::stoul(roString, nullptr, 16);
			std::string matString = j.at("ASSET_MAT_m_MaterialInstance").get<std::string>();
			AssetHandle matHandle = std::stoul(matString, nullptr, 16);

			if (auto renderObject = AssetManager::Instance().GetAsset<RenderObject>(roHandle); renderObject)
			{
				t.m_RenderObject = renderObject;
			}
			else
			{
				AssetManager::Instance().LoadAsset<RenderObject>(roHandle);
				t.m_RenderObject = AssetManager::Instance().GetAsset<RenderObject>(roHandle);
			}

			if (auto material = AssetManager::Instance().GetAsset<Material>(matHandle); material)
			{
				t.m_MaterialInstance = material;
			}
			else
			{
				AssetManager::Instance().LoadAsset<Material>(matHandle);
				t.m_MaterialInstance = AssetManager::Instance().GetAsset<Material>(matHandle);
			}*/

			t.m_IsVisible = j.at("m_IsVisible").get<bool>();

			//ECSSystemManager::Instance().GetSystem<MeshRendererSystem>()->SetDirty(true);
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