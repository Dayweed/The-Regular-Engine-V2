#pragma once
#include "Pipeline.h"
#include "RenderPass.h"
#include "AnimationStructure.h"
#include "Material.h"
#include "AnimationPlayer.h"

namespace TRE
{	
    struct AnimationUBO;

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