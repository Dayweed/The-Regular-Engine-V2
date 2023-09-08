#include "pch.h"
#include "FileSystem.h"

namespace TRE
{
	std::string FileSystem::GetFolderName(std::string type)
	{
		if (m_FolderNames.find(type) != m_FolderNames.end())
		{
			return m_FolderNames[type];
		}
		return "";
	}

	std::string FileSystem::GetFileTypeName(std::string type)
	{
		if (m_FileTypeNames.find(type) != m_FileTypeNames.end())
		{
			return m_FileTypeNames[type];
		}
		return "";
	}

	void FileSystem::LoadDefaultFolderFileNames(std::string filePath)
	{

	}
}