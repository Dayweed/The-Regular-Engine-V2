#pragma once
#include "Pipeline.h"
#include "RenderPass.h"
#include "AnimationStructure.h"

namespace TRE
{	
    struct anim_player
    {
        //-----------------------------------------------------------------------------------------------------------

        void PlayAnim(int i) noexcept
        {
            assert(i < m_AnimPackage.m_Animations.size());
            m_iCurAnim = i;
            m_Time = 0;
        }

        //-----------------------------------------------------------------------------------------------------------

        void Update(float DT) noexcept
        {
            auto& Anim = m_AnimPackage.m_Animations[m_iCurAnim];

            // advance time
            m_Time += DT;
            while (m_Time >= Anim.m_TimeLength) m_Time -= Anim.m_TimeLength;
        }

        //-----------------------------------------------------------------------------------------------------------

        void ComputeMatrices(glm::mat4 FinalL2W, const glm::mat4& L2W) const noexcept
        {

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

			void BindPipeline(VkCommandBuffer CmdBuffer);

		private:
			std::string m_ModelFilePath;
			std::unique_ptr<Pipeline> m_AnimationPipeline;
            std::unique_ptr<AnimationGeom> m_AnimationCharacter;
	};
}