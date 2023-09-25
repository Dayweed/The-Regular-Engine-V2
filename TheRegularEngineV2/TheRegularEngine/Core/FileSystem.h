#pragma once
/*!
	@file		FileSystem.h
	@author		Isaiah Lim (Code Contribution 100%)
	@email		lim.i@digipen.edu
	@date		08/09/2023
	@brief		Handles all the default folder names for each core purpose

	Copyright (C) 2023 DigiPen Institute of Technology.
	Reproduction or disclosure of this file or its contents without the
	prior written consent of DigiPen Institute of Technology is prohibited.
************************************************************************/

#define FILESYS_SCENE	"SCENE_FOLDER"
#define FILESYS_DESC	"DESC_FOLDER"
#define FILESYS_GEOM	"GEOM_FOLDER"

namespace TRE
{
	class FileSystem
	{
	public:
		static FileSystem& Instance()
		{
			static FileSystem instance;
			return instance;
		}

		std::string GetFolderName(std::string type);
		std::string GetFileTypeName(std::string type);
		void LoadDefaultFolderFileNames(std::string filePath);
		void GenerateFolderFileNamesFile(std::string filePath);

	private:
		// Delete possible copy ctor and assignment to ensure singleton
		FileSystem() {};
		FileSystem(FileSystem const&) = delete;
		void operator=(FileSystem const&) = delete;
		void* operator new(size_t) = delete;

		// Folder Type, Names
		std::unordered_map<std::string, std::string> m_FolderNames{
			{FILESYS_SCENE, "../Scenes/"}
		};

		// File Type, Names
		std::unordered_map<std::string, std::string> m_FileTypeNames{
			{FILESYS_SCENE, ".json"}
		};
	};

	class FileExplorer
	{
		public:
			static std::string OpenFileExplorer(const char* filter);
			static std::string SaveFileExplorer(const char* filter);
	};
}

#define GETFOLDER	FileSystem::Instance().GetFolderName
#define GETFILE		FileSystem::Instance().GetFileTypeName