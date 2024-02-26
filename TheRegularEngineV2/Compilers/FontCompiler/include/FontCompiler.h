#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include "glm/glm.hpp"

struct Character
{
	glm::vec2	 Size;
	glm::vec2	 Bearing;
	glm::vec2	 UV[4];
	unsigned int Advance;
	unsigned int HeightAdvance; //To be remove, just making it easy now
};

class FontCompiler
{
	public:
		FontCompiler(std::string Path);

		void CreateNewFontFace(std::string Filepath, std::string FontType);
		
		std::string m_FilePath;
		std::string m_FontType;
		std::unordered_map<char, Character> m_Characters;
		
		
		uint8_t* m_TextureData = nullptr;
		uint32_t m_Width = 0;
		uint32_t m_Height = 0;
};