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
		std::shared_ptr<Shader> VertexShader;
		std::shared_ptr<Shader> FragmentShader;
		PrimitiveType Primitive;
		float LineWidth = 1.f;
		size_t VertexStride = 0;
	};

	class Pipeline
	{
		public:
			Pipeline(const PipelineConfigurations& PipelineConfig, const std::shared_ptr<RenderPass>& TargetRenderPass);
			~Pipeline();

		public:
			VkPrimitiveTopology GetVulkanTopology(PrimitiveType TopologyType);
			const std::vector<VkDescriptorSet>& GetDescriptorSets();
			VkPipelineLayout GetPipelineLayout();
			VkPipeline GetPipeline();
			PipelineConfigurations& GetConfig();

		private:
			VkPipeline m_Pipeline;
			VkPipelineLayout m_Layout;
			PipelineConfigurations m_Config;

			std::vector<VkDescriptorSet> m_DescriptorSets;
			VkDescriptorSetLayout m_DescriptorSetLayout;
	};
}