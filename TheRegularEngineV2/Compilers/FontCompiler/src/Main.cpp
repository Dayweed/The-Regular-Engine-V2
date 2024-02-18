#include <iostream>
#include <memory>
#include <fstream>
#include "FontCompiler.h"

int main(int argc, char** argv)
{
	std::cout << "===Font Compiler: Run===" << std::endl;
	if (argc != 2)
	{
		std::cout << "Usage: ./FontCompiler.exe FontPath" << std::endl;
		return 0;
	}

	std::string path = argv[1];
	std::unique_ptr<FontCompiler> Compiler = std::make_unique<FontCompiler>(path);

	//Serializing font into binary
	std::string OutputFilePath = "../Resources/Fonts/" + Compiler->m_FontType + ".TREfont";
	std::ofstream file(OutputFilePath, std::ios::binary | std::ios::trunc);

	uint32_t TotalDataSize = Compiler->m_Width * Compiler->m_Height * 4;

	std::cout << "Wdith: " << Compiler->m_Width << std::endl;
	std::cout << "Height: " << Compiler->m_Height << std::endl;

	if (TotalDataSize <= 0 || Compiler->m_TextureData == nullptr)
	{
		assert(false && "Failure to get font data");
	}

	file.write(reinterpret_cast<const char*>(&Compiler->m_Width), sizeof(uint32_t));
	file.write(reinterpret_cast<const char*>(&Compiler->m_Height), sizeof(uint32_t));
	file.write(reinterpret_cast<const char*>(&TotalDataSize), sizeof(uint32_t));
	file.write(reinterpret_cast<const char*>(&Compiler->m_TextureData[0]), TotalDataSize);

	for (unsigned char c = 0; c < 128; c++)
	{
		file.write(reinterpret_cast<const char*>(&Compiler->m_Characters[c].Size), sizeof(glm::vec2));
		file.write(reinterpret_cast<const char*>(&Compiler->m_Characters[c].Bearing), sizeof(glm::vec2));
		file.write(reinterpret_cast<const char*>(&Compiler->m_Characters[c].UV), sizeof(glm::vec2) * 4);
		file.write(reinterpret_cast<const char*>(&Compiler->m_Characters[c].Advance), sizeof(unsigned int));
		file.write(reinterpret_cast<const char*>(&Compiler->m_Characters[c].HeightAdvance), sizeof(unsigned int));
	}
	
	file.close();

	std::cout << "Font Compiled Success" << std::endl;
}