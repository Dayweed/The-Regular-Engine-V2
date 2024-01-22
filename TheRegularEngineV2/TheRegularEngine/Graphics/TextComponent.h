#pragma once
#include "Core/ECS.h"
#include "Resource/ResourceManager.h"
#include "glm/gtc/type_ptr.hpp"

namespace TRE
{
	class TextComponent : property::base
	{
	public:
		bool m_IsVisible = false;
		std::string m_TextContent;
		glm::vec4 m_Color;
		std::string m_FontType;


		property_vtable()

		friend void to_json(nlohmann::json& j, const TextComponent& t)
		{
			const float* Color = glm::value_ptr(t.m_Color);
			std::vector<float> StoredColor{ Color[0], Color[1], Color[2], Color[3] };

			j = nlohmann::json
			{
				{ "m_IsVisible", t.m_IsVisible },
				{ "m_TextContent", t.m_TextContent },
				{ "m_Color", StoredColor },
				{ "m_FontType", t.m_FontType }
			};
		}

		friend void from_json(const nlohmann::json& j, TextComponent& t)
		{
			if (j.contains("m_IsVisible"))
			{
				t.m_IsVisible = j.at("m_IsVisible").get<bool>();
			}
			if (j.contains("m_TextContent"))
			{
				t.m_TextContent = j.at("m_TextContent").get<std::string>();
			}
			if (j.contains("m_Color"))
			{
				std::vector<float> Color{ j.at("Color").get<std::vector<float>>() };
				float LoadedColor[4]{ Color[0], Color[1], Color[2], Color[3] };
				t.m_Color = glm::make_vec4(LoadedColor);
			}
			if (j.contains("m_FontType"))
			{
				t.m_FontType = j.at("m_FontType").get<std::string>();
			}
		}
	};
}

property_begin(TRE::TextComponent)
{
	property_var(m_IsVisible),
	property_var(m_TextContent),
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
	property_var_fnbegin("Font Type", FontType)
	{
		InOut.m_Type = "Font Type";
		
	} property_var_fnend()

} property_vend_h(TRE::TextComponent)