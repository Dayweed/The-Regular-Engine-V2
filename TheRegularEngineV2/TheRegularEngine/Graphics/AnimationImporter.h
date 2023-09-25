#pragma once
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"
#include "AnimationStructure.h"

namespace TRE
{
	class AnimationImporter
	{
		public:
			struct myMeshPart
			{
				std::string                     m_MeshName;
				std::string                     m_Name;
				std::vector<vertex>				m_Vertices;
				std::vector<int>                m_Indices;
				int                             m_iMaterialInstance;
			};

			struct refs
			{
				std::vector<const aiNode*> m_Nodes;
			};

			AnimationImporter();
			~AnimationImporter();

			bool Import(std::string FileName, geom* pGeom, Skeleton* pSkeleton, anim_package* pAnimPackage);
			bool SanityCheck();
			void ImportSkeleton();
			void ImportAnimations();
			void ImportGeometry();
			void ImportGeometrySkin(std::vector<myMeshPart>& MyNodes);
			void ImportMaterials();

		private:
			std::vector<refs> m_MeshReferences;
			anim_package* m_pAnimPackage = nullptr;
			geom* m_pGeom = nullptr;
			Skeleton* m_pSkeleton = nullptr;
			const aiScene* m_pScene = nullptr;
	};
}