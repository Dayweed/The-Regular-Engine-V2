#pragma once
#include <array>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>
#include "glm/glm.hpp"

namespace TRE
{
	struct IColor
	{
		std::uint8_t R;
		std::uint8_t G;
		std::uint8_t B;
		std::uint8_t A;

		IColor(std::uint8_t r = 0, std::uint8_t g = 0, std::uint8_t b = 0, std::uint8_t a = 0) : R(r), G(g), B(b), A(a) {}

		bool operator==(const IColor& _Other) const
		{
			return R == _Other.R && G == _Other.G && B == _Other.B && A == _Other.A;
		}
	};

	struct Geom
	{
		struct Position
		{
			glm::vec3 Position;
		};

		struct Extra
		{
			glm::vec3 Color{};
			glm::vec3 Normal{};
			glm::vec2 UV{};
		};

		struct Mesh
		{
			std::array<char, 64>	Name ;			//Name of mesh
			//std::uint16_t			nSubMeshes;		//Total number of submeshes in mesh
			//std::uint16_t			iSubMesh;		//Index of submesh
			//std::uint16_t			nLODs{ 1 };		//Total number of LODs in mesh
			//std::uint16_t			iLOD{ 1 };		//Index of LOD of mesh
		};

		struct SubMesh
		{
			std::uint32_t			m_nFaces;			//Total number of faces(triangles) in submesh
			std::uint32_t			m_iIndices;			//Index of indices in submesh
			std::uint32_t			m_nIndices;			//Total number of indices for this submesh
			std::uint32_t			m_iVertices;		//Index of vertices in submesh
			std::uint32_t			m_nVertices;		//Total number of vertices for this submesh
			std::uint16_t			m_iMaterial;		//Index of material in submesh
			glm::vec3				m_PosCompressionOffset;
			glm::vec2				m_UVCompressionOffset;
		};

		Mesh*						pMesh;
		SubMesh*					pSubMesh;
		Position*					pPosition;
		Extra*						pExtra;
		std::uint32_t*				pIndices;

		std::uint32_t				nMeshes;
		std::uint32_t				nSubMeshes;
		std::uint32_t				nPosition;
		std::uint32_t				nExtras;
		std::uint32_t				nIndices;
		glm::vec3					PosCompressionScale;
		glm::vec2					UVCompressionScale;

		~Geom()
		{
			delete[] pMesh;
			delete[] pSubMesh;
			delete[] pPosition;
			delete[] pExtra;
			delete[] pIndices;
		}

		static void RunCompiler(std::string descPath);
		static void Serialize(const std::string& filePath, const std::unique_ptr<Geom> geom);
		static std::unique_ptr<Geom> Deserialize(const std::string& filePath);
	};

	struct TempGeom
	{
		struct Submesh
		{
			std::vector<Geom::Position> Position;
			std::vector<Geom::Extra> Extra;
			std::vector<std::uint32_t> Indices;
			std::uint32_t MaterialIndex;

			//Next time then compress
			glm::vec3 PosCompressionOffset;
			glm::vec2 UVCompressionOffset;
		};

		struct Mesh
		{
			std::string Name;
			std::vector<Submesh> Submeshes;
		};

		std::string Name;
		std::vector<Mesh> Meshes;
		glm::vec3 PosCompressionScale;
		glm::vec2 UVCompressionScale;
	};
}