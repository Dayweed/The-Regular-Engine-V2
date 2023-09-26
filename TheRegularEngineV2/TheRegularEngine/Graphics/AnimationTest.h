#pragma once
#include "Pipeline.h"
#include "RenderPass.h"
#include "AnimationStructure.h"
#include "Material.h"

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