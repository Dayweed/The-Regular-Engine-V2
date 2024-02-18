#include "FontCompiler.h"
#include <fstream>
#include <iostream>
#include <assert.h>
#include <ft2build.h>
#include FT_FREETYPE_H

constexpr int s_DefaultFontSize = 64;

static std::string GetFontType(std::string Filepath)
{
	size_t Start = Filepath.find_last_of('/') + 1;
	size_t End1 = Filepath.find_last_of('.');
	size_t End2 = Filepath.find_last_of('-');
	size_t Final = (End1 < End2) ? End1 : End2;
	std::string FontType = Filepath.substr(Start, Final - Start);

	return FontType;
}

void FontCompiler::CreateNewFontFace(std::string Filepath, std::string FontType)
{
	FT_Library m_FTLibrary;
	if (FT_Error Error = FT_Init_FreeType(&m_FTLibrary); !Error)
	{
		std::cout << "Free Type Library Init" << std::endl;
	}

	FT_Face face;
	if (FT_New_Face(m_FTLibrary, Filepath.c_str(), 0, &face))
	{
		std::cout << "Unable to load font: " << FontType << std::endl;
	}

	FT_Set_Pixel_Sizes(face, 0, s_DefaultFontSize); //Scale the font size using transform comp

	int width = 0;
	unsigned int height = 0;
	for (unsigned char c = 0; c < 128; c++)
	{
		if (FT_Load_Char(face, c, FT_LOAD_RENDER))
		{
			assert(false && "Unable to load Char");
		}

		width += face->glyph->bitmap.width;
		height = std::max(height, face->glyph->bitmap.rows);
	}

	m_TextureData = new uint8_t[width * height * 4];

	std::unordered_map<char, Character> LetterStorage;
	std::vector<uint8_t> Buffer(s_DefaultFontSize);
	int zxcv = 0;
	int offset = 0;
	for (unsigned char c = 0; c < 128; c++)
	{
		if (FT_Load_Char(face, c, FT_LOAD_RENDER) != 0)
		{
			std::cout << "Unable to load Char: " << c << ", this might cause issues!!!" << std::endl;
			continue;
		}

		int TempOffset = offset;
		for (unsigned int x = 0; x < face->glyph->bitmap.rows; x++)
		{
			std::copy(face->glyph->bitmap.buffer + x * face->glyph->bitmap.width, face->glyph->bitmap.buffer + (x + 1) * face->glyph->bitmap.width, Buffer.data());

			for (unsigned int y = 0; y < face->glyph->bitmap.width; y++)
			{
				const uint8_t& letter = Buffer[y];
				int Index = TempOffset + y * 4;
				for (int z = 0; z < 4; z++)
					m_TextureData[Index + z] = letter;
			}
			TempOffset += width * 4;
		}
		offset += face->glyph->bitmap.width * 4;

		float xpos = static_cast<float>(zxcv) / width;
		float w = static_cast<float>(face->glyph->bitmap.width) / static_cast<float>(width);
		float ypos = static_cast<float>(face->glyph->bitmap.rows) / static_cast<float>(height);

		Character character =
		{
			glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
			glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
			{{ xpos,  0.f }, { xpos + w,  0.f }, { xpos + w,  ypos }, {xpos, ypos} },
			face->glyph->advance.x,
			face->size->metrics.height
		};

		LetterStorage[c] = character;

		zxcv += static_cast<int>(face->glyph->bitmap.width);
	}

	m_Characters = LetterStorage;
	m_Width = width;
	m_Height = height;

	FT_Done_Face(face);
	FT_Done_FreeType(m_FTLibrary);
}

FontCompiler::FontCompiler(std::string Path) : m_FilePath(Path)
{
	m_FontType = GetFontType(Path);
	std::cout << "Compiling Font Type: " << m_FontType << std::endl;

	CreateNewFontFace(Path, m_FontType);
}