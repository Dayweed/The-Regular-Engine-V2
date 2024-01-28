#pragma once
#include "Core/ECS.h"
#include "Resource/ResourceManager.h"
#include "glm/gtc/type_ptr.hpp"

namespace TRE
{
	class UIComponent : property::base
	{
		public:
		UIComponent()
		{
			m_Material = std::make_shared<Material>(ResourceManager::Instance().GetResource<Shader>(6));
			m_Material->Invalidate();
		}

		std::shared_ptr<VulkanTexture> m_Texture;
		bool m_IsVisible = false;
		glm::vec4 m_Color { 1.f, 1.f, 1.f, 1.f };
		int m_Width = 1;
		int m_Height = 1;
		std::shared_ptr<Material> m_Material; //Should be removed and batched in future
		int m_RenderLayer = 0;

		property_vtable()

		friend void to_json(nlohmann::json& j, const UIComponent& t)
		{
			const float* Color = glm::value_ptr(t.m_Color);
			std::vector<float> StoredColor { Color[0], Color[1], Color[2], Color[3] };

			j = nlohmann::json
			{
				{ "Texture", t.m_Texture ? t.m_Texture->GetHandleHex() : "0" },
				{ "m_IsVisible", t.m_IsVisible },
				{ "Color", StoredColor },
				{ "Width", t.m_Width },
				{ "Height", t.m_Height }, 
				{ "RenderLayer", t.m_RenderLayer }
			};
		}

		friend void from_json(const nlohmann::json& j, UIComponent& t)
		{
			if (j.contains("Texture"))
			{
				std::string String = j.at("Texture").get<std::string>();
				ResourceHandle TextureHandle = Resource::GetGUIDFromHex(String);
				if (!t.m_Material)
				{
					t.m_Material = std::make_shared<Material>(ResourceManager::Instance().GetResource<Shader>(6));
					t.m_Material->Invalidate();
				}
				if (TextureHandle != 0)
				{
					if (auto Texture = ResourceManager::Instance().GetResource<VulkanTexture>(TextureHandle); Texture)
					{
						t.m_Texture = Texture;
					}
					else
					{
						t.m_Texture = VulkanTexture::Deserialize(String);

						if (t.m_Texture == nullptr)
							TRE_CORE_CRITICAL(String + "Texture failed to load in UI Component");
					}
				}
				else
				{
					t.m_Texture = nullptr;
				}
			}
			if (j.contains("m_IsVisible"))
			{
				t.m_IsVisible = j.at("m_IsVisible").get<bool>();
			}
			if (j.contains("Color"))
			{
				std::vector<float> Color { j.at("Color").get<std::vector<float>>() };
				float LoadedColor[4] { Color[0], Color[1], Color[2], Color[3] };
				t.m_Color = glm::make_vec4(LoadedColor);
			}
			if (j.contains("Width"))
			{
				t.m_Width = j.at("Width").get<int>();
			}
			if (j.contains("Height"))
			{
				t.m_Height = j.at("Height").get<int>();
			}
			if (j.contains("RenderLayer"))
			{
				t.m_RenderLayer = j.at("RenderLayer").get<int>();
			}
		}
	};
}

property_begin(TRE::UIComponent)
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
	property_var_fnbegin("Color", Color)
	{
		if (isRead)
		{
			InOut.m_Value = Self.m_Color;
		}
		else
		{
			Self.m_Color = InOut.m_Value;
		}
	} property_var_fnend(),
	property_var(m_Width),
	property_var(m_Height),
	property_var(m_RenderLayer)

} property_vend_h(TRE::UIComponent)