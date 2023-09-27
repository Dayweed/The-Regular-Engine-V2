#include "TextureDescriptorFile.h"
#include <iostream>

namespace TRE
{
	void TextureDescriptorFile::Write()
	{
		m_DescriptorFile << "Texture File Path:\n";
		m_TexturePath = m_DescriptorPath.substr(m_DescriptorPath.find_last_of("/") + 1);
		m_TexturePath = "../Resources/" + m_TexturePath;
		m_TexturePath = m_TexturePath.substr(0, m_TexturePath.find_last_of("."));
		m_TexturePath += ".DDS";
		m_DescriptorFile << m_TexturePath << "\n\n";
		m_TextureName = m_AssetPath.substr(m_AssetPath.find_last_of("/") + 1);
		m_TextureName = m_TextureName.substr(0, m_TextureName.find_last_of("."));
		m_DescriptorFile << "Texture Name:\n";
		m_DescriptorFile << m_TextureName << "\n\n";
		m_DescriptorFile << "Compress:\n";
		m_DescriptorFile << m_Compress << "\n\n";
		m_DescriptorFile << "GL_Linear:\n";
		m_DescriptorFile << m_Linear << "\n\n";
		m_DescriptorFile << "BCn:\n";
		m_DescriptorFile << m_BCn << "\n\n";
		m_DescriptorFile << "sRGB:\n";
		m_DescriptorFile << m_sRGB << "\n\n";
		m_DescriptorFile << "Transparent:\n";
		m_DescriptorFile << m_Transparent << "\n\n";
	}

	void TextureDescriptorFile::Read()
	{
		std::string line;
		std::getline(m_DescriptorFile, line);
		if (line == "Texture File Path:")
		{
			std::getline(m_DescriptorFile, line);
			m_TexturePath = line;
			std::getline(m_DescriptorFile, line);
		}
		else
		{
			std::cout << "Error: Texture is not valid" << std::endl;
			return;
		}
		std::getline(m_DescriptorFile, line);
		if (line == "Texture Name:")
		{
			std::getline(m_DescriptorFile, line);
			m_TextureName = line;
			std::getline(m_DescriptorFile, line);
		}
		else
		{
			std::cout << "Error: Texture Name is not valid" << std::endl;
			return;
		}
		std::getline(m_DescriptorFile, line);
		if (line == "Compress:")
		{
			std::getline(m_DescriptorFile, line);

			m_Compress = (bool)std::stoi(line);

			std::getline(m_DescriptorFile, line);
		}
		else
		{
			std::cout << "Error: Texture Compression flag missing" << std::endl;
			return;
		}
		std::getline(m_DescriptorFile, line);
		if (line == "GL_Linear:")
		{
			std::getline(m_DescriptorFile, line);

			m_Linear = (bool)std::stoi(line);

			std::getline(m_DescriptorFile, line);
		}
		else
		{
			std::cout << "Error: Texture Linear Filter flag missing" << std::endl;
			return;
		}
		std::getline(m_DescriptorFile, line);
		if (line == "BCn:")
		{
			std::getline(m_DescriptorFile, line);

			m_BCn = std::stoi(line);

			std::getline(m_DescriptorFile, line);
		}
		else
		{
			std::cout << "Error: Texture BCn flag missing" << std::endl;
			return;
		}
		std::getline(m_DescriptorFile, line);
		if (line == "sRGB:")
		{
			std::getline(m_DescriptorFile, line);

			m_sRGB = (bool)std::stoi(line);

			std::getline(m_DescriptorFile, line);
		}
		else
		{
			std::cout << "Error: Texture sRGB flag missing" << std::endl;
			return;
		}
		std::getline(m_DescriptorFile, line);
		if (line == "Transparent:")
		{
			std::getline(m_DescriptorFile, line);

			m_Transparent = (bool)std::stoi(line);

			std::getline(m_DescriptorFile, line);
		}
		else
		{
			std::cout << "Error: Texture transparency flag missing" << std::endl;
			return;
		}
	}
}