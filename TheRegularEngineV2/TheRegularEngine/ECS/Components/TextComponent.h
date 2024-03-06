#pragma once
#include "ECS/ECS.h"
#include "Resource/ResourceManager.h"
#include "glm/gtc/type_ptr.hpp"
#include "Graphics/FontManager.h"

namespace TRE
{
	class TextComponent : property::base
	{
	public:
		bool m_IsVisible = false;
		bool m_IsDialogue = false;
		bool m_IsFading = false;
		float m_Timer = 0.f; //Hidden variable from designers
		float m_Speed = 0.f;
		float m_FadingSpeed = 0.f;
		MultiLineString m_TextContent = { "Text" };
		glm::vec4 m_Color = { 0.f, 0.f, 0.f, 1.f };
		FontType m_FontName = { "arial" };

		property_vtable()

		friend void to_json(nlohmann::json& j, const TextComponent& t)
		{
			const float* Color = glm::value_ptr(t.m_Color);
			std::vector<float> StoredColor{ Color[0], Color[1], Color[2], Color[3] };

			j = nlohmann::json
			{
				{ "m_IsVisible", t.m_IsVisible },
				{ "m_IsDialogue", t.m_IsDialogue },
				{ "m_IsFading", t.m_IsFading },
				{ "m_Speed", t.m_Speed },
				{ "m_FadingSpeed", t.m_FadingSpeed },
				{ "m_TextContent", t.m_TextContent.Text },
				{ "m_Color", StoredColor },
				{ "m_FontType", t.m_FontName.m_FontType }
			};
		}

		friend void from_json(const nlohmann::json& j, TextComponent& t)
		{
			if (j.contains("m_IsVisible"))
			{
				t.m_IsVisible = j.at("m_IsVisible").get<bool>();
			}
			if (j.contains("m_IsDialogue"))
			{
				t.m_IsDialogue = j.at("m_IsDialogue").get<bool>();
			}
			if (j.contains("m_IsFading"))
			{
				t.m_IsFading = j.at("m_IsFading").get<bool>();
			}
			if (j.contains("m_Speed"))
			{
				t.m_Speed = j.at("m_Speed").get<float>();
			}
			if (j.contains("m_FadingSpeed"))
			{
				t.m_FadingSpeed = j.at("m_FadingSpeed").get<float>();
			}
			if (j.contains("m_TextContent"))
			{
				t.m_TextContent.Text = j.at("m_TextContent").get<std::string>();
			}
			if (j.contains("m_Color"))
			{
				std::vector<float> Color{ j.at("m_Color").get<std::vector<float>>() };
				float LoadedColor[4]{ Color[0], Color[1], Color[2], Color[3] };
				t.m_Color = glm::make_vec4(LoadedColor);
			}
			if (j.contains("m_FontType"))
			{
				t.m_FontName.m_FontType = j.at("m_FontType").get<std::string>();
				std::string Filepath = "../Resources/Font/" + t.m_FontName.m_FontType + ".TREfont"; //This should not be hardcoded
				FontManager::LoadFont(Filepath); //Try to load in case its not loaded yet, auto skips the loading if its loaded
			}
		}
	};
}

property_begin(TRE::TextComponent)
{
	property_var(m_IsVisible),
	property_var(m_IsDialogue),
	property_var(m_IsFading),
	property_var(m_Speed),
	property_var(m_FadingSpeed),
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
	property_var(m_FontName)

} property_vend_h(TRE::TextComponent)