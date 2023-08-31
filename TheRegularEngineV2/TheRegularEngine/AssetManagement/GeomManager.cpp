#include "pch.h"
#include "GeomManager.h"

namespace TRE
{
	void GeomManager::Serialize(const std::unique_ptr<Geom> geom, const std::string& filePath)
	{
		std::string_view path = filePath;
		std::string_view name = path;
		name.remove_prefix(name.find_last_of('/') + 1);
		name.remove_suffix(name.size() - name.find_last_of('.'));

		std::cout << "serializing mesh... " << name << std::endl;

		std::ofstream file(path, std::ios::binary);

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

	std::unique_ptr<Geom> GeomManager::Deserialize(const std::string& filePath)
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
				//TRE_CORE_ERROR("Failed to open file: {0}", geomPath);
			}
		}
		else
		{
			//TRE_CORE_ERROR("File does not exist: {0}", geomPath);
		}

		return geom;
	}
}