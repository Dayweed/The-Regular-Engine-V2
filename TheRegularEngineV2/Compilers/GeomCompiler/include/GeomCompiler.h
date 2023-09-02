#pragma once
#include "Geom.h"
#include "DescriptorFile.h"
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

namespace TRE
{
	class GeomDescriptorFile : public DescriptorFile
	{
	public:
		void SetGeomPath(const std::string& path) { m_GeomPath = path; }
		void SetPosition(const glm::vec3& position) { m_Position = position; }
		void SetRotation(const glm::vec3& rotation) { m_Rotation = rotation; }
		void SetScale(const glm::vec3& scale) { m_Scale = scale; }
		//void SetMeshRenameBool(const bool meshRename) { m_MeshRename = meshRename; }
		//void SetMeshName(const std::string& meshName) { m_MeshName = meshName; }

		const std::string& GetGeomPath() const { return m_GeomPath; }
		const glm::vec3& GetPosition() const { return m_Position; }
		const glm::vec3& GetRotation() const { return m_Rotation; }
		const glm::vec3& GetScale() const { return m_Scale; }
		//const bool GetMeshRenameBool() const { return m_MeshRename; }
		//const std::string& GetMeshName() const { return m_MeshName; }
	protected:
		void Generate() override;
		void Read() override;
	private:
		std::string m_GeomPath;
		glm::vec3 m_Position{ 0,0,0 };
		glm::vec3 m_Rotation{ 0,0,0 };
		glm::vec3 m_Scale{ 1,1,1 };
		//bool m_MeshRename{ false };
		//std::string m_MeshName;
	};

	class GeomCompiler
	{
	public:
		static GeomCompiler& Instance()
		{
			static GeomCompiler instance;
			return instance;
		}

		void Compile(const std::string& filename);

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
		const aiScene*			m_Scene;
		std::vector<Refs>		m_References;
		std::unique_ptr<Geom>	m_Geom;
		std::string				m_filePath;
	private:
		GeomCompiler() {};
		GeomCompiler(GeomCompiler const&) = delete;
		void operator=(GeomCompiler const&) = delete;
		void* operator new(size_t) = delete;
	};
	static GeomCompiler* _geom_compiler{ &GeomCompiler::Instance() };

}