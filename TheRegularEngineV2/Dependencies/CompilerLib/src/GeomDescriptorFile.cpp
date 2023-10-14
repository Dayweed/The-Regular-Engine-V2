#include "GeomDescriptorFile.h"
#include <iostream>

namespace
{
	std::vector<std::string> SplitString(const std::string& str, const std::string& delimiter)
	{
		std::vector<std::string> strings;
		std::string::size_type pos = 0;
		std::string::size_type prev = 0;
		while ((pos = str.find(delimiter, prev)) != std::string::npos)
		{
			strings.push_back(str.substr(prev, pos - prev));
			prev = pos + 1;
		}
		strings.push_back(str.substr(prev));
		return strings;
	}
}

namespace TRE
{
	void GeomDescriptorFile::Write()
	{
		m_DescriptorFile << "Geom File Path:\n";
		m_DescriptorFile << m_ResourcePath << "\n\n";
		m_DescriptorFile << "Scale:\n";
		m_DescriptorFile << m_Scale.x << ", " << m_Scale.y << ", " << m_Scale.z << "\n\n";
		m_DescriptorFile << "Rotation:\n";
		m_DescriptorFile << m_Rotation.x << ", " << m_Rotation.y << ", " << m_Rotation.z << "\n\n";
		m_DescriptorFile << "Translation:\n";
		m_DescriptorFile << m_Position.x << ", " << m_Position.y << ", " << m_Position.z << "\n\n";
		m_DescriptorFile << "Optimize:\n";
		m_DescriptorFile << m_Optimize << "\n\n";
		//m_DescriptorFile << "Mesh rename:\n";
		//m_DescriptorFile << (m_MeshRename ? "True" : "False") << std::endl << std::endl;
		//m_DescriptorFile << "Mesh name:\n";
		//m_DescriptorFile << m_MeshName << std::endl << std::endl;
	}

	void GeomDescriptorFile::Read()
	{
		std::string line;
		std::getline(m_DescriptorFile, line);
		if (line == "Geom File Path:")
		{
			std::getline(m_DescriptorFile, line);
			m_ResourcePath = line;
			std::getline(m_DescriptorFile, line);
		}
		else
		{
			std::cout << "Error: Geom file is not valid" << std::endl;
			return;
		}
		std::getline(m_DescriptorFile, line);
		if (line == "Scale:")
		{
			std::getline(m_DescriptorFile, line);
			std::vector<std::string> scaleContainer = SplitString(line, ", ");

			m_Scale.x = std::stof(scaleContainer[0]);
			m_Scale.y = std::stof(scaleContainer[1]);
			m_Scale.z = std::stof(scaleContainer[2]);
			std::getline(m_DescriptorFile, line);
		}
		else
		{
			std::cout << "Error: Scale missing" << std::endl;
			return;
		}
		std::getline(m_DescriptorFile, line);
		if (line == "Rotation:")
		{
			std::getline(m_DescriptorFile, line);
			std::vector<std::string> rotateContainer = SplitString(line, ", ");

			m_Rotation.x = std::stof(rotateContainer[0]);
			m_Rotation.y = std::stof(rotateContainer[1]);
			m_Rotation.z = std::stof(rotateContainer[2]);

			std::getline(m_DescriptorFile, line);
		}
		else
		{
			std::cout << "Error: Rotation missing" << std::endl;
			return;
		}
		std::getline(m_DescriptorFile, line);
		if (line == "Translation:")
		{
			std::getline(m_DescriptorFile, line);
			std::vector<std::string> translationContainer = SplitString(line, ", ");

			m_Position.x = std::stof(translationContainer[0]);
			m_Position.y = std::stof(translationContainer[1]);
			m_Position.z = std::stof(translationContainer[2]);

			std::getline(m_DescriptorFile, line);
		}
		else
		{
			std::cout << "Error: Translation missing" << std::endl;
			return;
		}
		std::getline(m_DescriptorFile, line);
		if (line == "Optimize:")
		{
			std::getline(m_DescriptorFile, line);

			m_Optimize = (bool)std::stoi(line);

			std::getline(m_DescriptorFile, line);
		}
		else
		{
			std::cout << "Error: Optimize flag missing" << std::endl;
			return;
		}
	}
}