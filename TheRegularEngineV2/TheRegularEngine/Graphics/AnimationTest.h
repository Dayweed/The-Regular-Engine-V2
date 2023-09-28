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
        void Update(float DT) noexcept
        {
            auto& Anim = m_AnimPackage.m_Animations[m_iCurAnim];

            // advance time
            m_Time += DT;
            while (m_Time >= Anim.m_TimeLength) m_Time -= Anim.m_TimeLength;
        }
    
        glm::quat Blend(const glm::quat& Start, float T, const glm::quat& End) const
        {
            bool bFlip = false;

            // Determine if quats are further than 90 degrees
            float Cs = Start.x * End.x + Start.y * End.y + Start.z * End.z + Start.w * End.w;
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
                const float Theta = acosf(Cs);
                const float S = 1.0f / sinf(Theta);

                inv_T = sinf((1.0f - T) * Theta) * S;
                T = sinf((T * Theta)) * S;
            }

            if (bFlip)
            {
                T = -T;
            }

            return End * T + (Start) *inv_T;
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
                glm::vec3 BlendedScale = KeyFrame.m_Scale[iFrameT0] + ((FrameTime - iFrameT0) * (KeyFrame.m_Scale[iFrameT1] - KeyFrame.m_Scale[iFrameT0]));
                glm::quat BlendedRotate = Blend(KeyFrame.m_Rotate[iFrameT0], (FrameTime - iFrameT0), KeyFrame.m_Rotate[iFrameT1]);
                glm::vec3 BlendedTranslate = KeyFrame.m_Translate[iFrameT0] + ((FrameTime - iFrameT0) * (KeyFrame.m_Translate[iFrameT1] - KeyFrame.m_Translate[iFrameT0]));

                glm::mat4 Scale(1.f); glm::mat4 Translate(1.f);
                Scale = glm::scale(Scale, BlendedScale);
                glm::mat4 Rotate = glm::mat4_cast(BlendedRotate);
                Translate = glm::translate(Translate, BlendedTranslate);

                FinalL2W[i] = Translate * Rotate * Scale;

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

        const Skeleton& m_Skeleton;
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