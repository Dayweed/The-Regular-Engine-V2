#pragma once
#include "glm/gtx/transform.hpp"
#include "glm/gtx/matrix_decompose.hpp"

namespace TRE
{
	struct vertex
	{
		glm::vec3         m_Position;
		glm::vec3         m_Normal;
		glm::vec3         m_Tangent;
		glm::vec2         m_UV;
	};

	//struct BoneInfluence
	//{
	//	glm::vec4         m_BoneWeights;
	//	glm::uvec4        m_BoneIndex;
	//};

	//struct submesh
	//{
	//	std::vector<vertex>		   m_Vertices;
	//	std::vector<BoneInfluence> m_BoneInfluence;
	//	std::vector<int>		   m_Indices;
	//	int						   m_iMaterial;
	//};

	//struct mesh
	//{
	//	std::string             m_Name;
	//	std::vector<submesh>    m_Submeshes;
	//};

	//struct geom
	//{
	//	std::string                     m_FileName;
	//	std::vector<mesh>               m_Mesh;
	//	std::vector<std::string>        m_TexturePaths;
	//};
}