#include "pch.h"
//#include "AnimationPlayer.h"
//#include "glm/gtx/transform.hpp"
//#include "glm/gtx/matrix_decompose.hpp"
//#include "VulkanUtilities.h"
//
//namespace TRE
//{
//    void AnimationPlayer::Update(float DT)
//    {
//        auto& Anim = m_Animation[m_iCurAnim];
//
//        // advance time
//        m_Time += DT;
//        while (m_Time >= Anim.m_TimeLength) m_Time -= Anim.m_TimeLength;
//    }
//
//    void AnimationPlayer::ComputeMatrices(std::span<glm::mat4> FinalL2W, const glm::mat4& L2W) const
//    {
//        auto& Anim = m_Animation[m_iCurAnim];
//        const float FrameTime = m_Time * Anim.m_FPS;
//        const int   iFrameT0 = static_cast<int>(FrameTime);
//        const int   iFrameT1 = static_cast<int>((iFrameT0 + 1) % Anim.m_BoneKeyFrames[0].m_Scale.size());
//
//        // compute hierarchy matrices
//        for (int i = 0; i < Anim.m_BoneKeyFrames.size(); ++i)
//        {
//            auto& KeyFrame = Anim.m_BoneKeyFrames[i];
//            glm::vec3 BlendedScale = KeyFrame.m_Scale[iFrameT0] + ((FrameTime - iFrameT0) * (KeyFrame.m_Scale[iFrameT1] - KeyFrame.m_Scale[iFrameT0]));
//            glm::quat BlendedRotate = vkUtils::Blend(KeyFrame.m_Rotate[iFrameT0], (FrameTime - iFrameT0), KeyFrame.m_Rotate[iFrameT1]);
//            glm::vec3 BlendedTranslate = KeyFrame.m_Translate[iFrameT0] + ((FrameTime - iFrameT0) * (KeyFrame.m_Translate[iFrameT1] - KeyFrame.m_Translate[iFrameT0]));
//
//            glm::mat4 Scale(1.f); glm::mat4 Translate(1.f);
//            Scale = glm::scale(Scale, BlendedScale);
//            glm::mat4 Rotate = glm::mat4_cast(BlendedRotate);
//            Translate = glm::translate(Translate, BlendedTranslate);
//
//            FinalL2W[i] = Translate * Rotate * Scale;
//
//            if (-1 != m_Skeleton.m_Bones[i].m_iParent)
//            {
//                assert(m_Skeleton.m_Bones[i].m_iParent <= i);
//                FinalL2W[i] = FinalL2W[m_Skeleton.m_Bones[i].m_iParent] * FinalL2W[i];
//            }
//            else
//            {
//                assert(i == 0);
//                FinalL2W[i] = L2W * FinalL2W[i];
//            }
//        }
//
//        // add the bind matrices into the final hierarchy
//        for (int i = 0; i < Anim.m_BoneKeyFrames.size(); ++i)
//        {
//            FinalL2W[i] *= m_Skeleton.m_Bones[i].m_InvBind;
//        }
//    }
//}