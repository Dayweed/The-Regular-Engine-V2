#pragma once
#include "Shader.h"
#include "VulkanTexture.h"
#include "UniformBuffer.h"

namespace TRE
{
	class Material
	{
		public:
			Material(const std::shared_ptr<Shader>& VertexShader, const std::shared_ptr<Shader>& FragShader);
			Material(std::shared_ptr<Material> CopyMaterial);
			~Material();

			void Invalidate();
			void UpdateForRendering(const std::shared_ptr<UniformBuffer>& UBO, uint32_t Index, VkDescriptorImageInfo test);

			void SetTextures(std::shared_ptr<VulkanTexture> Textures); //This should be removed eventually
			
			const VkDescriptorSet& GetDescriptor(uint32_t FrameIndex);
			std::shared_ptr<VulkanTexture> GetTextures() { return m_Textures; }

		private:
			std::shared_ptr<Shader> m_VertexShader;
			std::shared_ptr<Shader> m_FragmentShader;

			std::vector<VkDescriptorSet> m_DescriptorSets;
			std::vector<VkWriteDescriptorSet> m_WriteDescriptors;
			VkDescriptorSetLayout m_DescriptorSetLayout;

			std::shared_ptr<VulkanTexture> m_Textures;
	};
}