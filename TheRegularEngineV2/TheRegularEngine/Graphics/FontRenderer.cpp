#include "pch.h"
#include "FontRenderer.h"
#include "Core/Logger.h"

namespace TRE
{
	FontRenderer::FontRenderer()
	{
		if (FT_Error Error = FT_Init_FreeType(&m_FTLibrary); !Error)
		{
			TRE_CORE_INFO("Free Type Init");
		}

		//Initialize a default font to render with
		std::string FontType = GetFontType(m_DefaultFontFilepath);
		CreateNewFontFace(m_DefaultFontFilepath, FontType);
	}

	void FontRenderer::CreateNewFontFace(std::string Filepath, std::string FontType)
	{
		FT_Face NewFace;
		if (FT_Error Error = FT_New_Face(m_FTLibrary, Filepath.c_str(), 0, &NewFace); Error)
		{
			assert(false && "Unable to create new FT Face");
		}

		m_Faces[FontType] = NewFace;
		m_AvailableFonts.push_back(FontType);

		FT_Set_Pixel_Sizes(m_Faces[FontType], 0, 48);

		uint32_t bmpWidth = 0;
		std::vector<uint8_t> pixels;
		std::unordered_map<char, std::vector<uint8_t>> data;

		for (unsigned char c = 0; c < 128; c++)
		{
			FT_Error Error = FT_Load_Char(NewFace, c, FT_LOAD_RENDER);
			assert(Error == 0);

			uint32_t bmpHeight = std::max(bmpHeight, NewFace->glyph->bitmap.rows);

			unsigned int pitch = NewFace->glyph->bitmap.pitch;

			Character character =
			{
				glm::ivec2(NewFace->glyph->bitmap.width, NewFace->glyph->bitmap.rows),
				glm::ivec2(NewFace->glyph->bitmap_left, NewFace->glyph->bitmap_top),
				bmpWidth,
				static_cast<unsigned int>(NewFace->glyph->advance.x)
			};

			m_Characters.insert(std::pair<char, Character>(c, character));
			if (NewFace->glyph->bitmap.width > 0)
			{
				void* ptr = NewFace->glyph->bitmap.buffer;

				std::vector<uint8_t> Data(NewFace->glyph->bitmap.width * NewFace->glyph->bitmap.rows);

				int rows = NewFace->glyph->bitmap.rows;
				int width = NewFace->glyph->bitmap.width;
				for (int x = 0; x < rows; x++)
				{
					for (int y = 0; y < width; y++)
					{
						uint8_t byte = NewFace->glyph->bitmap.buffer[x * pitch + y];
						Data[x * pitch + y] = byte;
					}
				}

				data.insert(std::pair<char, std::vector<uint8_t>>(c, Data));
			}
			bmpWidth += NewFace->glyph->bitmap.width;
		}

		FT_Error Error = FT_Done_Face(NewFace);
		assert(Error == 0 && "Error calling Done Face on font");

		invBmpWidth = 1 / (float)bmpWidth;

		uint8_t* buffer = new uint8_t[bmpHeight * bmpWidth];
		memset(buffer, 0, bmpHeight * bmpWidth);

		uint32_t xpos = 0;
		for (unsigned char c = 0; c < 128; c++)
		{
			Character& character = m_Characters[c];

			std::vector<uint8_t>& charData = data[c];
			uint32_t width = character.Size.x;
			uint32_t height = character.Size.y;
			for (uint32_t i = 0; i < height; i++) {
				for (uint32_t j = 0; j < width; j++) {
					uint8_t byte = charData[i * width + j];
					buffer[i * bmpWidth + xpos + j] = byte;
				}
			}
			xpos += width;
		}


	}

	std::string FontRenderer::GetFontType(std::string Filepath)
	{
		size_t Start = Filepath.find_last_of('/') + 1;
		size_t End1 = Filepath.find_last_of('.');
		size_t End2 = Filepath.find_last_of('-');
		size_t Final = (End1 < End2) ? End1 : End2;
		std::string FontType = Filepath.substr(Start, Final - Start);
		TRE_CORE_INFO("Font Type Loaded: {0}", FontType);

		return FontType;
	}
}