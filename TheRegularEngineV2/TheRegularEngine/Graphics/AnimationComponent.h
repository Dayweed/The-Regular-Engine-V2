#pragma once
#include "Core/ECS.h"
#include "Resource/ResourceManager.h"

namespace TRE
{
	class AnimationComponent : property::base
	{
	public:
		std::shared_ptr<RenderObject> m_ObjectSource;
		bool m_IsVisible = false;

		property_vtable()

		friend void to_json(nlohmann::json& j, const AnimationComponent& t)
		{

			j = nlohmann::json
			{
				{ "Texture", t.m_ObjectSource ? t.m_ObjectSource->GetHandleHex() : "0" },
				{ "m_IsVisible", t.m_IsVisible },
			};
		}

		friend void from_json(const nlohmann::json& j, AnimationComponent& t)
		{
			if (j.contains("Texture"))
			{
				std::string String = j.at("Texture").get<std::string>();
				ResourceHandle TextureHandle = Resource::GetGUIDFromHex(String);

				if (TextureHandle != 0)
				{
					if (auto Texture = ResourceManager::Instance().GetResource<RenderObject>(TextureHandle); Texture)
					{
						t.m_ObjectSource = Texture;
					}
					else
					{
						t.m_ObjectSource = RenderObject::Deserialize(String);

						if (t.m_ObjectSource == nullptr)
							TRE_CORE_CRITICAL(String + "Texture failed to load in UI Component");
					}
				}
				else
				{
					t.m_ObjectSource = nullptr;
				}
			}
			if (j.contains("m_IsVisible"))
			{
				t.m_IsVisible = j.at("m_IsVisible").get<bool>();
			}
		}
	};
}


property_begin(TRE::AnimationComponent)
{
	property_var_fnbegin("Texture", resource_ref)
	{
		if (isRead)
		{
			if (Self.m_ObjectSource)
				InOut.m_Value = Self.m_ObjectSource->GetHandle();
			else
				InOut.m_Value = 0;
		}
		else
		{
			if (InOut.m_Value)
				Self.m_ObjectSource = TRE::ResourceManager::Instance().GetResource<TRE::RenderObject>(InOut.m_Value);
			else
				Self.m_ObjectSource = nullptr;
		}

	} property_var_fnend(),
		property_var(m_IsVisible),

} property_vend_h(TRE::AnimationComponent)