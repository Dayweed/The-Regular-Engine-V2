#pragma once
#include "Buffer.h"
#include "Descriptor.h"
#include "RenderPass.h"

namespace TRE
{
	enum class PrimitiveType //Although vk has more, the other 6 would not be used.
	{
		Point,
		Lines,
		LinesStrip,
		Triangles,
		TrianglesStrip,
		TranglesFan
	};

	struct PipelineConfigurations
	{
		std::shared_ptr<RenderPass> RenderPass;
		PrimitiveType Primitive;
	};

	class Pipeline
	{
		public:
			Pipeline(const PipelineConfigurations& PipelineConfig);
			~Pipeline();

			VkPrimitiveTopology GetVulkanTopology(PrimitiveType TopologyType);

			//Shader stuff
			VkShaderModule CreateShader(std::vector<char>& code);
			std::vector<char> readFile(const std::string& filename);

		public:
			std::vector<VkDescriptorSet> GetDescriptorSets();
			std::vector<std::shared_ptr<Buffer>> GetUBOBuffers();
			VkPipelineLayout GetPipelineLayout();
			VkPipeline GetPipeline();
			PipelineConfigurations& GetConfig();

		private:
			VkPipeline m_Pipeline;
			VkPipelineLayout m_Layout;

			PipelineConfigurations m_Config;
			std::vector<std::shared_ptr<Buffer>> m_UBOBuffers{};
			std::unique_ptr<DescriptorPool> m_DescriptorPool;
			std::vector<std::unique_ptr<DescriptorSetLayout>> m_DescriptorSetLayouts;
			std::vector<VkDescriptorSet> m_DescriptorSets;
	};
}