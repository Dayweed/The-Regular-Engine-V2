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
		PrimitiveType Primitive;
		std::shared_ptr<Shader> VertexShader;
		std::shared_ptr<Shader> FragmentShader;
	};

	class Pipeline
	{
		public:
			Pipeline(const PipelineConfigurations& PipelineConfig);
			~Pipeline();

			VkPrimitiveTopology GetVulkanTopology(PrimitiveType TopologyType);

		public:
			const VkDescriptorSet& GetDescriptorSets();
			VkPipelineLayout GetPipelineLayout();
			VkPipeline GetPipeline();
			PipelineConfigurations& GetConfig();
			std::shared_ptr<Buffer> GetUBOBuffers() { return m_UBOBuffer; }

		private:
			VkPipeline m_Pipeline;
			VkPipelineLayout m_Layout;

			PipelineConfigurations m_Config;
			
			std::unique_ptr<DescriptorPool> m_DescriptorPool;
			VkDescriptorSet m_DescriptorSet;

			std::shared_ptr<Buffer> m_UBOBuffer{};

			VkDescriptorSetLayout m_DescriptorSetLayout;
	};
}