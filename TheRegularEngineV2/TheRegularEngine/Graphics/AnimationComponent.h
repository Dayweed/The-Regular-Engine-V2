#pragma once
#include "Core/ECS.h"
#include "Resource/ResourceManager.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "Material.h"
#include "UniformBuffer.h"

#ifndef UINT32_T_CAST // a bit funky that I had to do this but alright...
#define UINT32_T_CAST(n) static_cast<uint32_t>((n))
#endif

namespace TRE
{
	struct AnimationUBO
	{
		glm::mat4 L2W[256];
	};

	class AnimationComponent : property::base
	{
		public:
			AnimationComponent()
			{
				m_UBO = std::make_shared<UniformBuffer>(UINT32_T_CAST(sizeof(AnimationUBO)), 8);
				m_ShadowAnimationMaterial = std::make_shared<Material>(ResourceManager::Instance().GetResource<Shader>(10));
				m_ShadowAnimationMaterial->Invalidate();
			}

			std::shared_ptr<UniformBuffer> m_UBO;
			std::shared_ptr<Material> m_ShadowAnimationMaterial;
			
			int m_FPS = 60;
			AnimationUBO m_BufferData;
			bool m_IsAnimating = true;

			property_vtable()

			friend void to_json(nlohmann::json& j, const AnimationComponent& t)
			{
				j = nlohmann::json
				{
					{ "m_IsAnimating", t.m_IsAnimating },
					{ "m_FPS", t.m_FPS },
				};
			}

			friend void from_json(const nlohmann::json& j, AnimationComponent& t)
			{
				if (j.contains("m_IsAnimating"))
				{
					t.m_IsAnimating = j.at("m_IsAnimating").get<bool>();
				}
				if (j.contains("m_FPS"))
				{
					t.m_FPS = j.at("m_FPS").get<int>();
				}
			}
	};
}


property_begin(TRE::AnimationComponent)
{
	property_var(m_IsAnimating),
	property_var(m_FPS)
} property_vend_h(TRE::AnimationComponent)