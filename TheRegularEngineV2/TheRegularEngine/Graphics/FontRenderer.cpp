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
	}

	std::string FontRenderer::GetFontType(std::string Filepath)
	{
		size_t Start = Filepath.find_last_of('/') + 1;
		size_t End1 = Filepath.find_last_of('.');
		size_t End2 = Filepath.find_last_of('-');
		size_t Final = (End1 < End2) ? End1 : End2;
		std::string FontType = Filepath.substr(Start, Final - Start);
		TRE_CORE_INFO("Font Type Loaded: {0}", FontType);
	}
}