#pragma once
#include "pch.h"
#include "VulkanTexture.h"
#include "Material.h"

namespace TRE
{
	#define DEFAULT_FONT_PATH "../Resources/Fonts/arial.TREfont"

	class FontManager
	{
		public:
			FontManager();
			~FontManager();

			static void RunCompiler(std::string assetpath);

			static void LoadFont(const std::string& FilePath);
			static bool IsFontLoaded(std::string FontType);

			static FontManager* GetInstance();

		//private:
			static std::unordered_map<std::string, ResourceHandle> m_LoadedFonts;
			static std::vector<std::string> m_AvailableFonts; //Should not have this waste of memory
			static std::string m_DefaultFontFilepath; //Default to this font unless other font is loaded in
			static FontManager* s_Instance;
	};
}