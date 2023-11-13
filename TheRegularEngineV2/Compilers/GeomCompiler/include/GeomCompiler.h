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
			static GeomCompiler& Instance()
			{
				static GeomCompiler instance;
				return instance;
			}

			void Compile(const GeomDescriptorFile& geomDesc);

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

				//glm::ivec4 BoneWeights;
				//glm::ivec4 BoneIndex;
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

			struct Skeleton
			{
				struct bone
				{
					std::string		m_Name;               // Bone name used for debugging
					glm::mat4		m_InvBind;            // Skin/Bind Pose to Local Space of the bone
					glm::mat4		m_NeutalPose;         // The neutral pose given by the Nodes
					int				m_iParent;            // Parent used to to concadenate the matrices
				};

				int findBone(std::string BoneName) const
				{
					int i = 0;
					for (auto& B : m_Bones)
						if (BoneName == B.m_Name)
							return i;
						else ++i;

					return -1;
				}

				std::vector<bone>   m_Bones;              // Bones are shorted (Parents go first)
			};

			struct bone_keyframes
			{
				std::vector<glm::vec3> m_Scale;
				std::vector<glm::quat> m_Rotate;
				std::vector<glm::vec3> m_Translate;
			};

			struct animation
			{
				std::string                     m_Name;             // Name for the animation (for debug)
				int                             m_FPS;              // What is the frame per second (ideally we should have 60fps or grader)
				float                           m_TimeLength;       // How long in seconds is this animation
				std::vector<bone_keyframes>     m_BoneKeyFrames;    // Keyframe used for the animation, note that the order of the keyframes in the vector should match the skeleton order
			};

			struct anim_package
			{
				std::vector<animation>          m_Animations;
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