#pragma once
#include "AnimationStructure.h"

namespace TRE
{
    struct AnimationPlayer
    {
            void Update(float DT);
            void ComputeMatrices(std::span<glm::mat4> FinalL2W, const glm::mat4& L2W) const;
            
            const Skeleton&     m_Skeleton;
            const anim_package& m_AnimPackage;
            int           m_iCurAnim{};
            float         m_Time{};
    };

    struct AnimationGeom
    {
        geom            m_SkinGeom;
        Skeleton        m_Skeleton;
        anim_package    m_AnimPackage;
        AnimationPlayer m_AnimPlayer{ m_Skeleton, m_AnimPackage };
    };
}