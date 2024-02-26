#pragma once
#include "Geom.h"
#include "GeomDescriptorFile.h"
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtc/matrix_transform.hpp"

namespace TRE
{
	class GeomCompiler
	{
		public:
			struct anim_package
			{
				std::vector<Animation>          m_Animations;
			};

			static GeomCompiler& Instance()
			{
				static GeomCompiler instance;
				return instance;
			}

			void Compile(const GeomDescriptorFile& geomDesc);

			std::unique_ptr<Geom> GetGeom() { return std::move(m_Geom); }
			const Skeleton& GetSkeleton() { return m_Skeleton; }
			const anim_package& GetAnimation() { return m_AnimPackage; }

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

				std::vector<BoneInfluence> m_BoneInfluence;
			};

			struct CompressedMeshPart
			{
				std::string MeshName;
				std::string Name;

				std::vector<Geom::Position> Position;
				std::vector<Geom::Extra> Extra;
				std::vector<std::uint32_t> Indices;
				std::uint32_t MaterialIndex;

				std::vector<BoneInfluence> m_BoneInfluence;

				glm::vec3 PosCompressionOffset;
				glm::vec2 UVCompressionOffset;
			};

		private:
			bool SanityCheck();
			void ImportGeometry(const GeomDescriptorFile& geomDesc);
			void ImportStaticMesh(std::vector<InputMeshPart>& inputMesh, const GeomDescriptorFile& geomDesc);
			bool ImportGeometryValidateMesh(const aiMesh& AssimpMesh, int& iTexture, int& iColor);
			void MergeData(std::vector<InputMeshPart>& inputMesh);
			void Optimize(std::vector<InputMeshPart>& inputMesh);
			std::vector<CompressedMeshPart> Quantize(const std::vector<InputMeshPart>& inputMesh);
			std::unique_ptr<TempGeom> CreateSkinGeom(const std::vector<CompressedMeshPart>&& compressedMesh);
			void CastToGeom(std::unique_ptr<TempGeom> tempGeom);

			void ImportSkeleton();
			void ImportAnimations();
			void ImportGeometrySkin(std::vector<InputMeshPart>& MyNodes);

		private:
			const aiScene*			m_Scene;
			std::vector<Refs>		m_References;
			std::unique_ptr<Geom>	m_Geom;
			std::string				m_filePath;
			
			bool m_IsAnimatable = false;
			Skeleton m_Skeleton;
			anim_package m_AnimPackage;

		private:
			GeomCompiler() = default;
			GeomCompiler(GeomCompiler const&) = delete;
			void operator=(GeomCompiler const&) = delete;
			void* operator new(size_t) = delete;
	};
}