#pragma once
#include <array>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>
#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"
#include <span>

namespace TRE
{
	struct BoneInfluence
	{
		glm::vec4         m_BoneWeights;
		glm::uvec4        m_BoneIndex;
	};

	struct bone_keyframes
	{
		std::vector<glm::vec3> m_Scale;
		std::vector<glm::quat> m_Rotate;
		std::vector<glm::vec3> m_Translate;
	};

	struct Animation
	{
		std::string                     m_Name;             // Name for the animation (for debug)
		int                             m_FPS;              // What is the frame per second (ideally we should have 60fps or grader)
		float                           m_TimeLength;       // How long in seconds is this animation
		std::vector<bone_keyframes>     m_BoneKeyFrames;    // Keyframe used for the animation, note that the order of the keyframes in the vector should match the skeleton order
	};

	struct bone
	{
		std::string		m_Name;               // Bone name used for debugging
		glm::mat4		m_InvBind;            // Skin/Bind Pose to Local Space of the bone
		glm::mat4		m_NeutalPose;         // The neutral pose given by the Nodes
		int				m_iParent;            // Parent used to to concadenate the matrices
	};

	struct Skeleton
	{
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

	//struct AnimationPlayer
	//{
	//	void Update(float DT);
	//	void ComputeMatrices(std::span<glm::mat4> FinalL2W, const glm::mat4& L2W) const;

	//	const Skeleton& m_Skeleton;
	//	const std::vector<Animation>& m_Animation;
	//	int           m_iCurAnim{};
	//	float         m_Time{};
	//};

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
			glm::vec3 Tangent{};
			glm::vec3 Bitangent{};
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
			std::uint32_t			m_iBones;			//Total number of bones for this submesh
			std::uint32_t			m_nBones;			//Index of bone in submesh
			glm::vec3				m_PosCompressionOffset;
			glm::vec2				m_UVCompressionOffset;
		};

		Mesh*						pMesh;
		SubMesh*					pSubMesh;
		Position*					pPosition;
		Extra*						pExtra;
		BoneInfluence*				pBone;
		std::uint32_t*				pIndices;

		Skeleton					m_Skeleton;
		std::vector<Animation>		m_Animation;

		std::uint32_t				nMeshes;
		std::uint32_t				nSubMeshes;
		std::uint32_t				nPosition;
		std::uint32_t				nExtras;
		std::uint32_t				nIndices;
		std::uint32_t				nBones;
		glm::vec3					PosCompressionScale;
		glm::vec2					UVCompressionScale;

		bool m_IsAnimated = false;

		~Geom()
		{
			delete[] pMesh;
			delete[] pSubMesh;
			delete[] pPosition;
			delete[] pExtra;
			delete[] pIndices;
		}

		static void RunCompiler(std::string descPath);
		static void Serialize(const std::string& filePath, const std::unique_ptr<Geom> geom, const Skeleton& Skel, const std::vector<Animation>& Animations);
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

			std::vector<BoneInfluence> Bone;

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