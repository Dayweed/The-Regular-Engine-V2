#pragma once
#include "Core/ECS.h"
#include "Resource/ResourceManager.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "Material.h"
#include "UniformBuffer.h"

namespace TRE
{
	struct AnimationUBO
	{
		glm::mat4 ProjView {1.f};
		glm::mat4 L2W[256];
	};

	class AnimationComponent : property::base
	{
		public:
			AnimationComponent()
			{
				//TRE_CORE_INFO("Animation Component Constructor");
				auto AnimationtextureHandle1 = Resource::GetGUIDFromHex("9c6509635ee2d750");
				auto AnimationtextureHandle2 = Resource::GetGUIDFromHex("52ba56f854e86f56");
				auto AnimationtextureHandle3 = Resource::GetGUIDFromHex("547865c1f61ef1f9");
				auto AnimationtextureHandle4 = Resource::GetGUIDFromHex("c076cd64a7491d7");
				auto AnimationtextureHandle5 = Resource::GetGUIDFromHex("6b2822ce3972f53");
				auto Texture1 = ResourceManager::Instance().GetResource<VulkanTexture>(AnimationtextureHandle1);
				auto Texture2 = ResourceManager::Instance().GetResource<VulkanTexture>(AnimationtextureHandle2);
				auto Texture3 = ResourceManager::Instance().GetResource<VulkanTexture>(AnimationtextureHandle3);
				auto Texture4 = ResourceManager::Instance().GetResource<VulkanTexture>(AnimationtextureHandle4);
				auto Texture5 = ResourceManager::Instance().GetResource<VulkanTexture>(AnimationtextureHandle5);

				m_MaterialInstace = std::make_shared<Material>(ResourceManager::Instance().GetResource<Shader>(9));
				m_MaterialInstace->Invalidate();
				m_MaterialInstace->SetTexture("DiffuseMap", Texture1);
				m_MaterialInstace->SetTexture("AOMap", Texture2);
				m_MaterialInstace->SetTexture("NormalMap", Texture3);
				m_MaterialInstace->SetTexture("RoughnessMap", Texture4);
				m_MaterialInstace->SetTexture("Metalness", Texture5);

				m_UBO = std::make_shared<UniformBuffer>(sizeof(AnimationUBO), 0);
			}

			std::shared_ptr<Material> m_MaterialInstace;
			std::shared_ptr<UniformBuffer> m_UBO;
			AnimationUBO m_BufferData;
			bool m_IsAnimating = true;

			property_vtable()

			friend void to_json(nlohmann::json& j, const AnimationComponent& t)
			{
				j = nlohmann::json
				{
					{ "m_IsAnimating", t.m_IsAnimating },
				};
			}

			friend void from_json(const nlohmann::json& j, AnimationComponent& t)
			{
				if (j.contains("m_IsVisible"))
				{
					t.m_IsAnimating = j.at("m_IsAnimating").get<bool>();
				}
			}
	};
}


property_begin(TRE::AnimationComponent)
{
	//property_var_fnbegin("AnimationSource", resource_ref)
	//{
	//	if (isRead)
	//	{
	//		if (Self.m_AnimationSource)
	//			InOut.m_Value = Self.m_AnimationSource->GetHandle();
	//		else
	//			InOut.m_Value = 0;
	//	}
	//	else
	//	{
	//		if (InOut.m_Value)
	//			Self.m_AnimationSource = TRE::ResourceManager::Instance().GetResource<TRE::RenderObject>(InOut.m_Value);
	//		else
	//			Self.m_AnimationSource = nullptr;
	//	}

	//} property_var_fnend(),
	property_var(m_IsAnimating),

} property_vend_h(TRE::AnimationComponent)