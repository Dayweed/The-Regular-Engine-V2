#include "pch.h"
#include "FontManager.h"
#include "Font.h"
#include "Core/Logger.h"
#include "Resource/ResourceManager.h"

namespace TRE
{
	std::unordered_map<std::string, ResourceHandle> FontManager::m_LoadedFonts;
	std::vector<std::string> FontManager::m_AvailableFonts;
	std::string FontManager::m_DefaultFontFilepath = DEFAULT_FONT_PATH; //Default to this font unless other font is loaded in
	FontManager* FontManager::s_Instance = nullptr;

	static std::string GetFontType(std::string Filepath)
	{
		size_t Start = Filepath.find_last_of('/') + 1;
		size_t End1 = Filepath.find_last_of('.');
		size_t End2 = Filepath.find_last_of('-');
		size_t Final = (End1 < End2) ? End1 : End2;
		std::string FontType = Filepath.substr(Start, Final - Start);

		return FontType;
	}

	FontManager* FontManager::GetInstance()
	{
		if (s_Instance == nullptr)
		{
			s_Instance = new FontManager();
		}

		return s_Instance;
	}

	FontManager::FontManager()
	{
		s_Instance = this;
		m_LoadedFonts.clear();
		m_AvailableFonts.clear();
		LoadFont(m_DefaultFontFilepath); //Loads the default arial
	}

	FontManager::~FontManager()
	{
		m_LoadedFonts.clear();
	}

	void FontManager::LoadFont(const std::string& FilePath)
	{
		std::string FontType = GetFontType(FilePath);

		if (IsFontLoaded(FontType))
		{
			TRE_CORE_INFO("{0} has been loaded already", FontType);
			return;
		}

		std::string ResourcePath = "../Resources/Fonts/" + FontType + ".TREfont";
		if (!std::filesystem::exists(ResourcePath))
		{
			RunCompiler("../Assets/Font/" + FontType + ".ttf");
			TRE_CORE_INFO("Compiled Font Asset of type: {0}", FontType);
		}

		std::unique_ptr<Font> NewFont = std::make_unique<Font>(ResourcePath);
		
		ResourceHandle assetHandle = Resource::GenerateGUID();
		NewFont->SetHandle(assetHandle);

		m_LoadedFonts[FontType] = assetHandle;

		ResourceManager::Instance().AddResource(std::move(NewFont));
		m_AvailableFonts.push_back(FontType);
		TRE_CORE_INFO("{0} has been loaded successfully", FontType);
	}

	bool FontManager::IsFontLoaded(std::string FontType)
	{
		if (m_LoadedFonts.contains(FontType))
			return true;

		return false;
	}

	void FontManager::RunCompiler(std::string assetpath)
	{
		const char* FontExe = "..\\Compilers\\FontCompiler.exe";
		if (std::filesystem::exists(FontExe) == false)
		{
			TRE_CORE_CRITICAL("Error: FontCompiler.exe does not exist");
			return;
		}

		char currentDir[FILENAME_MAX];
		if (_getcwd(currentDir, sizeof(currentDir)))
		{
			std::string exePath = FontExe;

			char command[256];
			snprintf(command, sizeof(command), "\"%s %s\"", exePath.c_str(), assetpath.c_str());

			int result = system(command);
			if (result != 0)
			{
				TRE_CORE_ERROR("Error: FontCompiler.exe failed to run. Code: {0}", result);
				return;
			}
		}
		else
		{
			TRE_CORE_ERROR("Error: could not get current directory");
			return;
		}
	}
}