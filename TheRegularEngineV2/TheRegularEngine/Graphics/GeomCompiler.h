#pragma once
#include "pch.h"
#include "Geom.h"
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

namespace TRE
{
	class GeomCompiler
	{
	public:
		static GeomCompiler& Instance()
		{
			static GeomCompiler instance;
			return instance;
		}

		void Compile(const std::string& filename);
		void Serialize(const std::string& returnPath = "");
		void Deserialize(const std::string& geomPath);

		std::unique_ptr<Geom> GetGeom() { return std::move(m_Geom); }
	private:
		struct Refs
		{
			std::vector<const aiNode*> Nodes;
		};

		struct FullVertex
		{
			glm::vec3 Position;
			glm::vec2 UV;
			glm::vec3 fNormal;
			glm::vec3 fTangent;
			glm::vec3 fBitangent;
			glm::vec4 fColor;
			IColor Normal;
			IColor Tangent;
			IColor Bitangent;
			IColor Color;
		};

		struct InputMeshPart
		{
			std::string MeshName;
			std::string Name;

			std::vector<FullVertex> Vertices;
			std::vector<std::uint32_t> Indices;
			std::uint32_t MaterialIndex;
		};

		struct CompressedMeshPart
		{
			std::string MeshName;
			std::string Name;

			std::vector<Geom::Position> Position;
			std::vector<Geom::Extra> Extra;
			std::vector<std::uint32_t> Indices;
			std::uint32_t MaterialIndex;

			glm::vec3 PosCompressionOffset;
			glm::vec2 UVCompressionOffset;
		};
	private:
		bool SanityCheck();
		void ImportData();
		void ImportStaticMesh(std::vector<InputMeshPart>& inputMesh);
		bool ImportGeometryValidateMesh(const aiMesh& AssimpMesh, int& iTexture, int& iColor);
		void MergeData(std::vector<InputMeshPart>& inputMesh);
		void Optimize(std::vector<InputMeshPart>& inputMesh);
		std::vector<CompressedMeshPart> Quantize(const std::vector<InputMeshPart>& inputMesh);
		std::unique_ptr<TempGeom> CreateSkinGeom(const std::vector<CompressedMeshPart>&& compressedMesh);
		void CastToGeom(std::unique_ptr<TempGeom> tempGeom);

	private:
		const aiScene*		m_Scene;
		std::vector<Refs>	m_References;
		std::unique_ptr<Geom> m_Geom;
		std::string m_filePath;
	private:
		GeomCompiler() {};
		GeomCompiler(GeomCompiler const&) = delete;
		void operator=(GeomCompiler const&) = delete;
		void* operator new(size_t) = delete;
	};
	static GeomCompiler* _geom_compiler{ &GeomCompiler::Instance() };

}