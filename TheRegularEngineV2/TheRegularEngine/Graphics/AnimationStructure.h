#pragma once

namespace TRE
{
	struct material_instance
	{
		enum class shading_model : std::uint8_t
		{
			UNKOWN, GOURAUD, PBR, TOON, UNLIGHT
		};

		enum class address_mode : std::uint8_t
		{
			TILE, MIRROR, CLAMP
		};

		struct sampler
		{
			int                 m_iTexture = -1;
			address_mode        m_UMode;
			address_mode        m_VMode;
		};

		std::string             m_Name;
		shading_model           m_ShadingModel;
		glm::ivec4				m_DiffuseColor;
		glm::ivec4				m_SpecularColor;
		glm::ivec4				m_AmbientColor;
		glm::ivec4				m_EmmisiveColor;
		float                   m_ShininessFactor;
		float                   m_ShininessStreanthFactor;
		float                   m_OpacityFactor;
		sampler                 m_DiffuseSampler;
		sampler                 m_SpecularSampler;
		sampler                 m_OpacitySampler;
		sampler                 m_AmbientSampler;
		sampler                 m_EmissiveSampler;
		sampler                 m_ShininessSampler;
		sampler                 m_LightmapSampler;
		sampler                 m_NormalSampler;
		sampler                 m_HeightSampler;
	};

	struct bone_keyframes
	{
		//std::vector<glm::mat4>  m_Transfoms;        // Vector for all the frames of the animation
		std::vector<glm::vec3> m_Scale;
		std::vector<glm::vec4> m_Rotate;
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
		std::vector<animation>          m_Animations;       // A list of animation
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

	struct vertex
	{
		glm::vec3         m_Position;
		glm::vec2          m_UV;
		glm::vec3          m_Normal;
		glm::vec3          m_Tangent;
		glm::vec3          m_Bitangent;
		glm::vec4           m_BoneWeights;
		glm::ivec4           m_BoneIndex;
	};

	struct submesh
	{
		std::vector<vertex>     m_Vertices;
		std::vector<int>        m_Indices;
		int                     m_iMaterial;
	};

	struct mesh
	{
		std::string             m_Name;
		std::vector<submesh>    m_Submeshes;
	};

	struct geom
	{
		std::string                     m_FileName;
		std::vector<mesh>               m_Mesh;
		std::vector<material_instance>  m_MaterialInstance;
		std::vector<std::string>        m_TexturePaths;
	};
}