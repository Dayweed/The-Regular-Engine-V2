#include "pch.h"
#include "FileSystem.h"

#include "Serialization.h"

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

	void FileSystem::GenerateFolderFileNamesFile(std::string filePath)
	{
		// Set up document
		rapidjson::Document fileSystemDoc;
		std::string filePathString{ filePath + ".json" };
		fileSystemDoc.SetObject();
		WriteToExternalFile(fileSystemDoc, filePathString.c_str());

		ObjectSerializer fileSerial(filePathString.c_str());
		Allocator allocator = fileSerial.getDoc().GetAllocator();

		// Folder Name
		ObjectBuilder folder;
		folder.insertValue(FILESYS_SCENE, std::string("../Scenes/"), allocator);
		folder.insertValue(FILESYS_DESC, std::string("../Assets/"), allocator);
		folder.insertValue(FILESYS_GEOM, std::string("../Assets/"), allocator);
		fileSerial.AddObjectToDoc(folder.getValue(), "FolderNames");

		// File Name
		ObjectBuilder file;
		file.insertValue(FILESYS_SCENE, std::string(".json"), allocator);
		file.insertValue(FILESYS_DESC, std::string(".desc"), allocator);
		file.insertValue(FILESYS_GEOM, std::string(".geom"), allocator);
		fileSerial.AddObjectToDoc(file.getValue(), "FileNames");

		// Write to Doc
		fileSerial.writeToDoc(filePathString.c_str());
	}
}