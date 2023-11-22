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
		glm::mat4 L2W[256];
	};

	class AnimationComponent : property::base
	{
		public:
			AnimationComponent()
			{
				m_UBO = std::make_shared<UniformBuffer>(sizeof(AnimationUBO), 8);
			}

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