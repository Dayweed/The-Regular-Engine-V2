#include "DescriptorFile.h"
#include <iostream>

namespace TRE
{
	void DescriptorFile::GenerateDescriptorFile()
	{
		std::cout << "Generating descriptor file for " << m_AssetPath << std::endl;
		WriteAssetPath();
		Write();
		m_DescriptorFile.close();
	}

	void DescriptorFile::ReadDescriptorFile(const std::string& descriptorPath)
	{
		SetDescriptorPath(descriptorPath);
		ReadDescriptorPath();
		Read();
		m_DescriptorFile.close();
	}

	void DescriptorFile::WriteAssetPath()
	{
		m_DescriptorFile = std::fstream(m_DescriptorPath, std::fstream::out);
		if (m_DescriptorFile.is_open() == false)
		{
			std::cout << "Error: Could not open descriptor file" << std::endl;
			return;
		}
		m_DescriptorFile << "Asset File Path:\n";
		m_DescriptorFile << m_AssetPath << "\n\n";
	}

	void DescriptorFile::ReadDescriptorPath()
	{
		m_DescriptorFile = std::fstream(m_DescriptorPath, std::fstream::in);
		if (m_DescriptorFile.is_open() == false)
		{
			std::cout << "Error: Could not open descriptor file" << std::endl;
			return;
		}
		std::string line;
		std::getline(m_DescriptorFile, line);
		if (line == "Asset File Path:")
		{
			std::getline(m_DescriptorFile, line);
			m_AssetPath = line;
			std::getline(m_DescriptorFile, line);
		}
		else
		{
			std::cout << "Error: Descriptor file is not valid" << std::endl;
			return;
		}
	}
}