#include "pch.h"
#include "FileSystem.h"
//#include <commdlg.h>
#include <windows.h>
#include "Engine.h"
#include "Serialization.h"
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#define GLFW_EXPOSE_NATIVE_WIN32
#include "GLFW/glfw3native.h"

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
		folder.insertValue(FILESYS_SCENE, std::string("..\\Scenes\\"), allocator);
		folder.insertValue(FILESYS_DESC, std::string("..\\Assets\\"), allocator);
		folder.insertValue(FILESYS_GEOM, std::string("..\\Assets\\"), allocator);
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

namespace TRE
{
	std::string FileExplorer::OpenFileExplorer(const char* filter)
	{
		// Temporary buffers
		CHAR szFile[256] = { 0 };
		CHAR currentDir[256] = { 0 };

		// win32 struct, alot of information
		OPENFILENAMEA ofn;
		// Clear out the struct to make sure its empty
		ZeroMemory(&ofn, sizeof(OPENFILENAME));
		ofn.lStructSize = sizeof(OPENFILENAME);
		// Set "owner" to current glfw window
		ofn.hwndOwner = glfwGetWin32Window(Engine::GetInstance().GetWindow()->GetWindowHandle());
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		// Get current working directory (By default its where the project file is at)
		if (GetCurrentDirectoryA(256, currentDir))
			ofn.lpstrInitialDir = currentDir;
		// Filter (Types of file (.txt? .json?))
		if (filter != nullptr)
		{
			ofn.lpstrFilter = filter;
			// Which index of the filter to find, should be set to .txt or .json?
			ofn.nFilterIndex = 1;
		}
		// Make sure working directory dont get changed and check everything exists
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
		if (GetOpenFileNameA(&ofn) == TRUE)
			return ofn.lpstrFile;
		// Return empty string if file explorer is closed
		return std::string{};
	}

	std::string FileExplorer::SaveFileExplorer(const char* filter)
	{
		// Temporary buffers
		CHAR szFile[256] = { 0 };
		CHAR currentDir[256] = { 0 };

		// win32 struct, alot of information
		OPENFILENAMEA ofn;
		// Clear out the struct to make sure its empty
		ZeroMemory(&ofn, sizeof(OPENFILENAME));
		ofn.lStructSize = sizeof(OPENFILENAME);
		// Set "owner" to current glfw window
		ofn.hwndOwner = glfwGetWin32Window(Engine::GetInstance().GetWindow()->GetWindowHandle());
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		// Get current working directory (By default its where the project file is at)
		if (GetCurrentDirectoryA(256, currentDir))
			ofn.lpstrInitialDir = currentDir;
		// Filter (Types of file (.txt? .json?))
		ofn.lpstrFilter = filter;
		// Which index of the filter to find, should be set to .txt or .json?
		ofn.nFilterIndex = 1;
		// Make sure working directory dont get changed and check everything exists
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

		// Sets the extension type to the one given
		ofn.lpstrDefExt = strchr(filter, '\0') + 1;
		if (GetSaveFileNameA(&ofn) == TRUE)
			return ofn.lpstrFile;
		// Return empty string if file explorer is closed
		return std::string{};
	}
}