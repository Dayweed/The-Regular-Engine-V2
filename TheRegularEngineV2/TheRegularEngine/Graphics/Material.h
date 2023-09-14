#pragma once
#include "Shader.h"

namespace TRE
{
	class Material
	{
		public:
			Material(const std::shared_ptr<Shader>& VertexShader, const std::shared_ptr<Shader>& FragShader);
			Material(std::shared_ptr<Material> CopyMaterial);
			~Material();

			void Invalidate();
			void UpdateForRendering(std::vector<VkWriteDescriptorSet> DescriptorWrites);

			const VkDescriptorSet& GetDescriptor(uint32_t FrameIndex);

		private:
			std::shared_ptr<Shader> m_VertexShader;
			std::shared_ptr<Shader> m_FragmentShader;

			std::vector<VkDescriptorSet> m_DescriptorSets;
			std::vector<VkWriteDescriptorSet> m_WriteDescriptors;
			VkDescriptorSetLayout m_DescriptorSetLayout;
	};
}