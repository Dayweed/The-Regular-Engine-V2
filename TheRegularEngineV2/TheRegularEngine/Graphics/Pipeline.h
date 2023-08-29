#pragma once
#include "Buffer.h"
#include "Descriptor.h"
#include "RenderPass.h"

namespace TRE
{
	class Pipeline
	{
		public:
			Pipeline(std::shared_ptr<RenderPass> RenderPass);
			~Pipeline();


			//Shader stuff
			VkShaderModule CreateShader(std::vector<char>& code);
			std::vector<char> readFile(const std::string& filename);

		public:
			std::vector<VkDescriptorSet> GetDescriptorSets();
			std::vector<std::shared_ptr<Buffer>> GetUBOBuffers();
			VkPipelineLayout GetPipelineLayout();
			VkPipeline GetPipeline();

		private:
			VkPipeline m_Pipeline;
			VkPipelineLayout m_Layout;

			std::shared_ptr<RenderPass> m_Renderpass;

			std::vector<std::shared_ptr<Buffer>> m_UBOBuffers{};
			std::unique_ptr<DescriptorPool> m_DescriptorPool;
			std::vector<std::unique_ptr<DescriptorSetLayout>> m_DescriptorSetLayouts;
			std::vector<VkDescriptorSet> m_DescriptorSets;
	};
}