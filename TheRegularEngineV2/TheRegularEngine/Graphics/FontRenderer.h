#pragma once
#include "freetype/freetype.h"
#include FT_FREETYPE_H

namespace TRE
{
	struct Character
	{
		unsigned int TexID;
		glm::ivec2	 Size;
		glm::ivec2	 Bearing;
		unsigned int Advance;
	};

	struct FontVertex {
		glm::vec3 pos;
		glm::vec4 color;
		glm::vec2 uv;
	};

	class FontRenderer
	{
		public:
			FontRenderer();
			~FontRenderer();

			void CreateNewFontFace(std::string Filepath, std::string FontType);
			void PopulateFont(const char*);
			std::string GetFontType(std::string Filepath);

			void RenderFont();

		private:
			FT_Library m_FTLibrary;
			std::unordered_map<std::string, FT_Face> m_Faces{};
			std::vector<std::string> m_AvailableFonts;
			std::unordered_map<std::string, std::map<char, Character>> m_Characters{};

			std::string m_DefaultFontFilepath = "";
	};
}