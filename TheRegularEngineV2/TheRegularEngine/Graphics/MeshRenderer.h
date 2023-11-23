#pragma once
#include "Core/ECS.h"
#include "Core/System.h"
#include "RenderObject.h"
#include "Sphere3D.h"
#include "Material.h"
#include "Resource/ResourceManager.h"

#include "Resource/Resource.h"

namespace TRE
{
	class MeshRenderer : property::base
	{
	public:
		std::shared_ptr<RenderObject>	m_RenderObject;
		std::shared_ptr<Material>		m_MaterialInstance;
		std::shared_ptr<Material>		m_AnimationMaterialInstance;
		Collision::Sphere3D				m_BoundingSphere;
		bool							m_IsVisible{ true };
		bool							m_IsCulled{ false };
		bool							m_IsDirty{ false };

		property_vtable()

		friend void to_json(nlohmann::json& j, const MeshRenderer& t)
		{
			j = nlohmann::json{
				{ "ASSET_GEOM_m_RenderObject", t.m_RenderObject ? t.m_RenderObject->GetHandleHex() : "0" },
				{ "ASSET_MAT_m_MaterialInstance", t.m_MaterialInstance ? t.m_MaterialInstance->GetHandleHex() : "0" },
				{ "ASSET_MAT_m_AnimationMaterialInstance", t.m_AnimationMaterialInstance ? t.m_AnimationMaterialInstance->GetHandleHex() : "0" },
				{ "m_IsVisible", t.m_IsVisible },	
			};
		}
		friend void from_json(const nlohmann::json& j, MeshRenderer& t)
		{
			if (j.contains("ASSET_GEOM_m_RenderObject"))
			{
				std::string roString = j.at("ASSET_GEOM_m_RenderObject").get<std::string>();
				ResourceHandle roHandle = Resource::GetGUIDFromHex(roString);

				if (roHandle != 0)
				{
					if (auto renderObject = ResourceManager::Instance().GetResource<RenderObject>(roHandle); renderObject)
					{
						t.m_RenderObject = renderObject;
					}
					else
					{
						t.m_RenderObject = RenderObject::Deserialize(roString);

						if (t.m_RenderObject == nullptr)
							TRE_CORE_CRITICAL(roString + ".geom not found!");
					}
				}
				else
				{
					t.m_RenderObject = nullptr;
				}
			}
			if (j.contains("ASSET_MAT_m_MaterialInstance"))
			{
				std::string matString = j.at("ASSET_MAT_m_MaterialInstance").get<std::string>();
				ResourceHandle matHandle = Resource::GetGUIDFromHex(matString);

				if (matHandle != 0)
				{
					if (auto material = ResourceManager::Instance().GetResource<Material>(matHandle); material)
					{
						t.m_MaterialInstance = material;
					}
					else
					{
						t.m_MaterialInstance = Material::Deserialize(matString);

						if (t.m_MaterialInstance == nullptr)
							TRE_CORE_CRITICAL(matString + ".mat not found!");
					}
				}
				//Else most likely default material
				else
				{
					t.m_MaterialInstance = nullptr;
				}
			}

			if (j.contains("ASSET_MAT_m_AnimationMaterialInstance"))
			{
				std::string matString = j.at("ASSET_MAT_m_AnimationMaterialInstance").get<std::string>();
				ResourceHandle matHandle = Resource::GetGUIDFromHex(matString);

				if (matHandle != 0)
				{
					if (auto material = ResourceManager::Instance().GetResource<Material>(matHandle); material)
					{
						t.m_AnimationMaterialInstance = material;
					}
					else
					{
						t.m_AnimationMaterialInstance = Material::Deserialize(matString);

						if (t.m_AnimationMaterialInstance == nullptr)
							TRE_CORE_CRITICAL(matString + ".mat not found!");
					}
				}
				//Else most likely default material
				else
				{
					t.m_AnimationMaterialInstance = nullptr;
				}
			}

			if(j.contains("m_IsVisible"))
				t.m_IsVisible = j.at("m_IsVisible").get<bool>();
			t.m_IsDirty = true;
		}
	};

	class MeshRendererSystem : public ECSSystem
	{
	public:
		void LateUpdate() override;
		void AfterReset() override;
		void OnDestroyEntities() override;
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

property_begin(TRE::MeshRenderer)
{
	property_var_fnbegin("Mesh", resource_list)
	{
		InOut.m_Type = "MESH";

		if (isRead)
		{
			if (Self.m_RenderObject)
				InOut.m_Value = Self.m_RenderObject->GetHandle();
			else
				InOut.m_Value = 0;
		}
		else
		{
			if(InOut.m_Value)
				Self.m_RenderObject = TRE::ResourceManager::Instance().GetResource<TRE::RenderObject>(InOut.m_Value);
			else
				Self.m_RenderObject = nullptr;
		}

	} property_var_fnend(),
	property_var_fnbegin("Material Instance", resource_list)
	{
		InOut.m_Type = "MATERIAL";

		if (isRead)
		{
			if (Self.m_MaterialInstance)
				InOut.m_Value = Self.m_MaterialInstance->GetHandle();
			else
				InOut.m_Value = 0;
		}
		else
		{
			if(InOut.m_Value)
				Self.m_MaterialInstance = TRE::ResourceManager::Instance().GetResource<TRE::Material>(InOut.m_Value);
			else
				Self.m_MaterialInstance = nullptr;
		}
		
	} property_var_fnend(),
	property_var_fnbegin("Animation Material Instance", resource_list)
	{
		InOut.m_Type = "MATERIAL";

		if (isRead)
		{
			if (Self.m_AnimationMaterialInstance)
				InOut.m_Value = Self.m_AnimationMaterialInstance->GetHandle();
			else
				InOut.m_Value = 0;
		}
		else
		{
			if (InOut.m_Value)
				Self.m_AnimationMaterialInstance = TRE::ResourceManager::Instance().GetResource<TRE::Material>(InOut.m_Value);
			else
				Self.m_AnimationMaterialInstance = nullptr;
		}

	} property_var_fnend(),
	property_var(m_IsVisible)

} property_vend_h(TRE::MeshRenderer)