#pragma once
#include "Pipeline.h"
#include "RenderPass.h"
#include "AnimationStructure.h"
#include "Material.h"
#include "AnimationPlayer.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"

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
            void BindBuffers(VkCommandBuffer CmdBuffer);
            void Draw(VkCommandBuffer CmdBuffer);

            const VkDescriptorSet& GetDescriptorSet(uint32_t Index);

		private:
			std::string m_ModelFilePath;

            std::shared_ptr<AnimationGeom> m_AnimationCharacter;
            std::unique_ptr<VertexBuffer> m_VertexBuffer;
            std::unique_ptr<IndexBuffer> m_IndexBuffer;
            std::unique_ptr<Material> m_MaterialInstace;
	};
}