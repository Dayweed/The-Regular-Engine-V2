#include "Geom.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <direct.h>

namespace TRE
{
	void Geom::RunCompiler(std::string descPath)
	{
		const char* geomExe = "..\\Compilers\\GeomCompiler.exe";
		if (std::filesystem::exists(geomExe) == false)
		{
			std::cout << "Error: GeomCompiler.exe does not exist" << std::endl;
			return;
		}

		char currentDir[FILENAME_MAX];
		if (_getcwd(currentDir, sizeof(currentDir)))
		{
			//std::string exePath = currentDir;
			//exePath += "\\";
			//exePath += textureExe;
			std::string exePath = geomExe;

			std::replace(descPath.begin(), descPath.end(), '/', '\\');
			//std::string newDescPath = currentDir;
			//newDescPath += "\\";
			//newDescPath += descPath;
			std::string newDescPath = descPath;

			char command[256];
			snprintf(command, sizeof(command), "\"%s %s\"", exePath.c_str(), newDescPath.c_str());

			int result = system(command);
			if (result != 0)
			{
				std::cout << "Error: GeomCompiler.exe failed to run. Code: " << result << std::endl;
				return;
			}
		}
		else
		{
			std::cout << "Error: could not get current directory" << std::endl;
			return;
		}
	}

	void Geom::Serialize(const std::string& filePath, const std::unique_ptr<Geom> geom)
	{
		std::string_view name = filePath;
		name.remove_prefix(name.find_last_of('/') + 1);
		name.remove_suffix(name.size() - name.find_last_of('.'));

		std::ofstream file(filePath, std::ios::binary);

		//file.write(reinterpret_cast<const char*>(&m_Geom->pMesh->Name), sizeof(Geom::Mesh) * m_Geom->nMeshes);
		//file.write(reinterpret_cast<const char*>(&m_Geom->pSubMesh), sizeof(Geom::SubMesh) * m_Geom->nSubMeshes);
		file.write(reinterpret_cast<const char*>(&geom->nPosition), sizeof(std::uint32_t));
		file.write(reinterpret_cast<const char*>(geom->pPosition), sizeof(Geom::Position) * geom->nPosition);
		file.write(reinterpret_cast<const char*>(&geom->nExtras), sizeof(std::uint32_t));
		file.write(reinterpret_cast<const char*>(geom->pExtra), sizeof(Geom::Extra) * geom->nExtras);
		file.write(reinterpret_cast<const char*>(&geom->nIndices), sizeof(std::uint32_t));
		file.write(reinterpret_cast<const char*>(geom->pIndices), sizeof(std::uint32_t) * geom->nIndices);

		file.close();
	}

	std::unique_ptr<Geom> Geom::Deserialize(const std::string& filePath)
	{
		auto geom = std::make_unique<Geom>();

		std::filesystem::path path = filePath;
		if (std::filesystem::exists(filePath))
		{
			std::ifstream file(filePath, std::ios::binary);
			if (file.is_open())
			{
				file.seekg(0, std::ios::end);
				std::size_t size = file.tellg();
				file.seekg(0, std::ios::beg);

				char* buffer = new char[size];
				file.read(buffer, size);
				file.close();

				std::size_t offset = 0;
				//geom->pMesh = reinterpret_cast<Geom::Mesh*>(buffer);
				//geom->pSubMesh = reinterpret_cast<Geom::SubMesh*>(buffer + sizeof(Geom::Mesh) * geom->nMeshes);
				geom->nPosition = *reinterpret_cast<std::uint32_t*>(buffer + offset);
				offset += sizeof(std::uint32_t);
				geom->pPosition = new Geom::Position[geom->nPosition];
				memcpy(geom->pPosition, buffer + offset, sizeof(Geom::Position) * geom->nPosition);
				offset += sizeof(Geom::Position) * geom->nPosition;
				geom->nExtras = *reinterpret_cast<std::uint32_t*>(buffer + offset);
				geom->pExtra = new Geom::Extra[geom->nExtras];
				offset += sizeof(std::uint32_t);
				memcpy(geom->pExtra, buffer + offset, sizeof(Geom::Extra) * geom->nExtras);
				offset += sizeof(Geom::Extra) * geom->nExtras;
				geom->nIndices = *reinterpret_cast<std::uint32_t*>(buffer + offset);
				geom->pIndices = new std::uint32_t[geom->nIndices];
				offset += sizeof(std::uint32_t);
				memcpy(geom->pIndices, buffer + offset, sizeof(std::uint32_t) * geom->nIndices);
				offset += sizeof(std::uint32_t) * geom->nIndices;

				delete[] buffer;
			}
			else
			{
				std::cout << "Error: could not open file: " << filePath << std::endl;
			}
		}
		else
		{
			std::cout << "Error: file does not exist: " << filePath << std::endl;
		}

		return geom;
	}
}