#pragma once
#include "Pipeline.h"
#include "RenderPass.h"
#include "AnimationStructure.h"
#include "Material.h"
#include "glm/gtx/transform.hpp"
#include "glm/gtx/matrix_decompose.hpp"

namespace TRE
{	
    struct AnimationUBO;
    struct anim_player
    {
        //-----------------------------------------------------------------------------------------------------------

        void PlayAnim(int i) noexcept
        {
            assert(i < m_AnimPackage.m_Animations.size());
            m_iCurAnim = i;
            m_Time = 0;
        }

        void Update(float DT) noexcept
        {
            auto& Anim = m_AnimPackage.m_Animations[m_iCurAnim];

            // advance time
            m_Time += DT;
            while (m_Time >= Anim.m_TimeLength) m_Time -= Anim.m_TimeLength;
        }

        //-----------------------------------------------------------------------------------------------------------

        static float Dot(glm::quat Start, glm::quat Q)
        {
            return  Start.x * Q.x +
                Start.y * Q.y +
                Start.z * Q.z +
                Start.w * Q.w;
        }
        
        glm::quat BlendAccurate(const glm::quat& Start, float T, const glm::quat& End) const
        {
            bool bFlip = false;

            // Determine if quats are further than 90 degrees
            float Cs = Dot(Start, End);
            if (Cs < 0.0f)
            {
                Cs = -Cs;
                bFlip = !bFlip;
            }

            float inv_T;
            if ((1.0f - Cs) < 0.000001f)
            {
                inv_T = 1.0f - T;
            }
            else
            {
                const float Theta = acos(Cs);
                const float S = 1.0f / sin(Theta);

                inv_T = sin(( 1.0f - T ) * Theta) * S;
                T = sin(T * Theta) * S;
            }

            if (bFlip)
            {
                T = -T;
            }

            return End * T + (Start) * inv_T;
        }

        static glm::mat4 setRotation(const glm::quat& R)
        {
            float tx = 2.0f * R.x;   // 2x
            float ty = 2.0f * R.y;   // 2y
            float tz = 2.0f * R.z;   // 2z
            float txw = tx *  R.w;   // 2x * w
            float tyw = ty *  R.w;   // 2y * w
            float tzw = tz *  R.w;   // 2z * w
            float txx = tx *  R.x;   // 2x * x
            float tyx = ty *  R.x;   // 2y * x
            float tzx = tz *  R.x;   // 2z * x
            float tyy = ty *  R.y;   // 2y * y
            float tzy = tz *  R.y;   // 2z * y
            float tzz = tz *  R.z;   // 2z * z

            glm::mat4 Return = glm::identity<glm::mat4>();
            // Fill out 3x3 rotations.
            Return[0][0] = 1.0f - (tyy + tzz); Return[0][1] = tyx + tzw;      Return[0][2] = tzx - tyw;
            Return[1][0] = tyx - tzw;      Return[1][1] = 1.0f - (txx + tzz); Return[1][2] = tzy + txw;
            Return[2][0] = tzx + tyw;      Return[2][1] = tzy - txw;            Return[2][2] = 1.0f - (txx + tyy);
            
            return Return;
        }

        void ComputeMatrices(std::span<glm::mat4> FinalL2W, const glm::mat4& L2W) const
        {
            auto& Anim = m_AnimPackage.m_Animations[m_iCurAnim];
            const float FrameTime = m_Time * Anim.m_FPS;
            const int   iFrameT0 = static_cast<int>(FrameTime);
            const int   iFrameT1 = static_cast<int>((iFrameT0 + 1) % Anim.m_BoneKeyFrames[0].m_Scale.size());

            // compute hierarchy matrices
            for (int i = 0; i < Anim.m_BoneKeyFrames.size(); ++i)
            {
                auto& KeyFrame = Anim.m_BoneKeyFrames[i];

                // Setup the L2W matrix for this bone, (two versions) one with inner-frame blending, the other version without it
                //FinalL2W[i].setup(xcore::math::transform3::Blend(KeyFrame.m_Transfoms[iFrameT0], FrameTime - iFrameT0, KeyFrame.m_Transfoms[iFrameT1]));

                glm::vec3 BlendedScale = KeyFrame.m_Scale[iFrameT0] + (FrameTime - iFrameT0) * (KeyFrame.m_Scale[iFrameT1] - KeyFrame.m_Scale[iFrameT0]);
                glm::quat BlendedRotate = BlendAccurate(KeyFrame.m_Rotate[iFrameT0], (FrameTime - iFrameT0), KeyFrame.m_Rotate[iFrameT1]);
                glm::vec3 BlendedTranslate = KeyFrame.m_Translate[iFrameT0] + (FrameTime - iFrameT0) * (KeyFrame.m_Translate[iFrameT1] - KeyFrame.m_Translate[iFrameT0]);

                glm::mat4 Scale = glm::scale(BlendedScale);
                glm::mat4 Rotate = setRotation(BlendedRotate);
                glm::mat4 Translate = glm::translate(BlendedTranslate);

                FinalL2W[i] = Translate * Rotate * Scale;
                //FinalL2W[i] = Scale * Rotate * Translate;

                if (-1 != m_Skeleton.m_Bones[i].m_iParent)
                {
                    assert(m_Skeleton.m_Bones[i].m_iParent <= i);
                    FinalL2W[i] = FinalL2W[m_Skeleton.m_Bones[i].m_iParent] * FinalL2W[i];
                }
                else
                {
                    assert(i == 0);
                    FinalL2W[i] = L2W * FinalL2W[i];
                }
            }

            // add the bind matrices into the final hierarchy
            for (int i = 0; i < Anim.m_BoneKeyFrames.size(); ++i)
            {
                FinalL2W[i] *= m_Skeleton.m_Bones[i].m_InvBind;
            }
        }

        const Skeleton&     m_Skeleton;
        const anim_package& m_AnimPackage;
        int                 m_iCurAnim{};
        float               m_Time{};
    };

	struct AnimationGeom
	{
		geom            m_SkinGeom;
		Skeleton        m_Skeleton;
		anim_package    m_AnimPackage;
		anim_player     m_AnimPlayer{ m_Skeleton, m_AnimPackage };
	};

	class AnimationTest
	{
		public:
			AnimationTest(const std::shared_ptr<RenderPass>& TargetPass);
			~AnimationTest();

            void UpdateAnimations(AnimationUBO& UBO, glm::mat4 L2W);
            void UpdateMaterial(const std::shared_ptr<UniformBuffer>& UBO, uint32_t Index);
			void BindPipeline(VkCommandBuffer CmdBuffer);
            void BindBuffers(VkCommandBuffer CmdBuffer);
            void Draw(VkCommandBuffer CmdBuffer);

            VkPipelineLayout GetPipelineLayout();
            const VkDescriptorSet& GetDescriptorSet(uint32_t Index);

        private:
            void CreateIndexBuffer(const std::vector<int>& indices);
            void CreateVertexBuffer(const std::vector<vertex>& vertices);

		private:
			std::string m_ModelFilePath;
			std::unique_ptr<Pipeline> m_AnimationPipeline;
            std::unique_ptr<AnimationGeom> m_AnimationCharacter;

            std::unique_ptr<Buffer> m_VertexBuffer;
            std::unique_ptr<Buffer> m_IndexBuffer;
            uint32_t m_VertexCount = 0;
            uint32_t m_IndexCount = 0;

            std::unique_ptr<Material> m_MaterialInstace;
	};
}