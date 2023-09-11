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
		// Set up document
		rapidjson::Document fileSystemDoc;
		std::string file_properties_name{ filePath + ".json" };
		fileSystemDoc.SetObject();
		WriteToExternalFile(fileSystemDoc, file_properties_name.c_str());

		// Save scene properties
		ObjectSerializer scene_prop(file_properties_name.c_str());
		Allocator allocator = scene_prop.getDoc().GetAllocator();

		// Scene Properties
		ObjectBuilder scene;
		scene.insertValue(FILESYS_SCENE, "../Scenes/", allocator);
		//scene_prop_obj.insertComponent(scene.getValue(), "FILESYS_SCENE", allocator);
		scene_prop.AddObjectToDoc(scene.getValue(), FILESYS_SCENE);

		// Write to Doc
		scene_prop.writeToDoc(file_properties_name.c_str());
	}
}