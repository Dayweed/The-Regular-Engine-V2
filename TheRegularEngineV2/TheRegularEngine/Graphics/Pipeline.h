#pragma once
#include "Buffer.h"
#include "Descriptor.h"
#include "RenderPass.h"
#include "UniformBuffer.h"
#include "Shader.h"

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
		std::shared_ptr<Shader> VertexShader;
		std::shared_ptr<Shader> FragmentShader;
		PrimitiveType Primitive;
	};

	class Pipeline
	{
		public:
			Pipeline(const PipelineConfigurations& PipelineConfig, std::shared_ptr<Buffer>& UniformBuffer);
			~Pipeline();

		public:
			VkPrimitiveTopology GetVulkanTopology(PrimitiveType TopologyType);
			const VkDescriptorSet& GetDescriptorSets();
			VkPipelineLayout GetPipelineLayout();
			VkPipeline GetPipeline();
			PipelineConfigurations& GetConfig();

		private:
			VkPipeline m_Pipeline;
			VkPipelineLayout m_Layout;
			PipelineConfigurations m_Config;

			VkDescriptorSet m_DescriptorSet;
			VkDescriptorSetLayout m_DescriptorSetLayout;
	};
}