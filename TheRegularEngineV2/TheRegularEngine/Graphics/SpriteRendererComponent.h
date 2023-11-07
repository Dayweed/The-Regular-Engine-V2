#pragma once
#include "Core/ECS.h"
#include "VulkanTexture.h"
#include "Resource/ResourceManager.h"

namespace TRE
{
	struct SpriteRenderer : property::base
	{
		std::shared_ptr<VulkanTexture> m_Texture;
		bool m_IsVisible = true;



		property_vtable()
	};
}

property_begin(TRE::SpriteRenderer)
{
	property_var_fnbegin("Texture", resource_list)
	{
		InOut.m_Type = "TEXTURE";

		if (isRead)
		{
			if (Self.m_Texture)
				InOut.m_Value = Self.m_Texture->GetHandle();
			else
				InOut.m_Value = 0;
		}
		else
		{
			if (InOut.m_Value)
				Self.m_Texture = TRE::ResourceManager::Instance().GetResource<TRE::VulkanTexture>(InOut.m_Value);
			else
				Self.m_Texture = nullptr;
		}

	} property_var_fnend(),
	property_var(m_IsVisible),

} property_vend_h(TRE::SpriteRenderer)