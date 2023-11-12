#include "pch.h"
#include "Pipeline.h"
#include "RendererContext.h"
#include "RenderObject.h"
#include "Core/Engine.h"
#include "Descriptor.h"
#include "Core/Logger.h"

namespace TRE
{
	uint32_t VertexBufferInput::VertexDataTypeSize(VertexInputDataType type)
	{
		switch (type)
		{
			case VertexInputDataType::Float:  return 4;
			case VertexInputDataType::Vec2:   return 4 * 2;
			case VertexInputDataType::Vec3:   return 4 * 3;
			case VertexInputDataType::Vec4:   return 4 * 4;
			case VertexInputDataType::Mat3:   return 4 * 3 * 3;
			case VertexInputDataType::Mat4:   return 4 * 4 * 4;
			case VertexInputDataType::Int:    return 4;
			case VertexInputDataType::IVec2:  return 4 * 2;
			case VertexInputDataType::IVec3:  return 4 * 3;
			case VertexInputDataType::IVec4:  return 4 * 4;
			case VertexInputDataType::Bool:   return 1;
		}

		assert(false, "Data Type not registered");
		return 0;
	}

	VkFormat Pipeline::VertexDataTypeToVulkanFormat(VertexInputDataType type)
	{
		switch (type)
		{
			case VertexInputDataType::Float:   return VK_FORMAT_R32_SFLOAT;
			case VertexInputDataType::Vec2:    return VK_FORMAT_R32G32_SFLOAT;
			case VertexInputDataType::Vec3:    return VK_FORMAT_R32G32B32_SFLOAT;
			case VertexInputDataType::Vec4:    return VK_FORMAT_R32G32B32A32_SFLOAT;
			case VertexInputDataType::Int:     return VK_FORMAT_R32_SINT;
			case VertexInputDataType::IVec2:   return VK_FORMAT_R32G32_SINT;
			case VertexInputDataType::IVec3:   return VK_FORMAT_R32G32B32_SINT;
			case VertexInputDataType::IVec4:   return VK_FORMAT_R32G32B32A32_SINT;
		}

		assert(false);
		return VK_FORMAT_UNDEFINED;
	}

	PipelineConfigurations& Pipeline::GetConfig()
	{
		return m_Config;
	}

	VkPipelineLayout Pipeline::GetPipelineLayout()
	{
		return m_Layout;
	}
	
	VkPipeline Pipeline::GetPipeline()
	{
		return m_Pipeline;
	}

	Pipeline::Pipeline(const PipelineConfigurations& PipelineConfig, const std::shared_ptr<RenderPass>& TargetRenderPass) : m_Config(PipelineConfig)
	{
		auto SwapChain = Engine::GetInstance().GetWindow()->GetSwapChain();
		auto Device = RendererContext::GetDevice();

		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		std::vector<VkVertexInputBindingDescription> VertexInputBindingDescriptions;
		std::vector<VkVertexInputAttributeDescription> VertexInputAttributesDescriptions;

		if (m_Config.UseAutoShaderVertexInput)
		{
			const auto& VertexInputAttributesDescriptions = m_Config.Shader->GetVertexAttributes();
			VkVertexInputBindingDescription VertexInputBindingDescriptions{};
			VertexInputBindingDescriptions.binding = 0;
			VertexInputBindingDescriptions.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
			VertexInputBindingDescriptions.stride = (m_Config.VertexStride == 0) ? m_Config.Shader->GetVertexStrides() : (uint32_t)m_Config.VertexStride;

			vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(VertexInputAttributesDescriptions.size());
			vertexInputInfo.pVertexAttributeDescriptions = VertexInputAttributesDescriptions.data();
			vertexInputInfo.vertexBindingDescriptionCount = 1;
			vertexInputInfo.pVertexBindingDescriptions = &VertexInputBindingDescriptions;
		}
		else
		{
			int previouslocation = 0;
			for (const auto& VertexLayout : m_Config.CustomVertexBufferInputLayout)
			{
				VkVertexInputBindingDescription Binding{};
				Binding.binding = VertexLayout.BindPoint;
				Binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
				Binding.stride = VertexLayout.VertexStride;
				VertexInputBindingDescriptions.push_back(Binding);

				for (int x = 0; x < VertexLayout.m_Inputs.size(); x++)
				{
					VkVertexInputAttributeDescription InputAttribute{};
					InputAttribute.location = previouslocation + 1;
					InputAttribute.binding = Binding.binding;
					InputAttribute.offset = VertexLayout.m_Inputs[x].Offset;
					InputAttribute.format = VertexDataTypeToVulkanFormat(VertexLayout.m_Inputs[x].Type);
					previouslocation = InputAttribute.location;
					VertexInputAttributesDescriptions.push_back(InputAttribute);
				}
			}

			vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(VertexInputAttributesDescriptions.size());
			vertexInputInfo.pVertexAttributeDescriptions = VertexInputAttributesDescriptions.data();
			vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(VertexInputBindingDescriptions.size());
			vertexInputInfo.pVertexBindingDescriptions = VertexInputBindingDescriptions.data();
		}

		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = GetVulkanTopology(m_Config.Primitive);
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		VkViewport viewport{};
		viewport.width = static_cast<float>(SwapChain->GetWidth());
		viewport.height = static_cast<float>(SwapChain->GetHeight());
		viewport.minDepth = 0.f;
		viewport.maxDepth = 1.f;

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = SwapChain->GetSwapChainExtent();

		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.scissorCount = 1;
		viewportState.pViewports = &viewport;
		viewportState.pScissors = &scissor;

		VkPipelineRasterizationStateCreateInfo rasterizer{};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = m_Config.LineWidth;
		rasterizer.cullMode = m_Config.CullMode;
		rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;
		rasterizer.depthBiasConstantFactor = 0.f;
		rasterizer.depthBiasClamp = 0.f;

		VkPipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisampling.minSampleShading = 0.f;
		multisampling.pSampleMask = nullptr;
		multisampling.alphaToCoverageEnable = VK_FALSE;
		multisampling.alphaToOneEnable = VK_FALSE;

		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = m_Config.EnableBlending ? VK_TRUE : VK_FALSE;
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
		colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

		VkPipelineColorBlendStateCreateInfo colorBlending{};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY;
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.blendConstants[0] = 1.0f;
		colorBlending.blendConstants[1] = 1.0f;
		colorBlending.blendConstants[2] = 1.0f;
		colorBlending.blendConstants[3] = 1.0f;
		std::vector<VkDynamicState> dynamicStates =
		{
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR
		};
		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
		dynamicState.pDynamicStates = dynamicStates.data();
		
		VkPipelineDepthStencilStateCreateInfo depthStencil{};
		depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencil.depthTestEnable = m_Config.EnableDepthTest ? VK_TRUE : VK_FALSE;
		depthStencil.depthWriteEnable = VK_TRUE;
		depthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
		depthStencil.depthBoundsTestEnable = VK_FALSE;
		depthStencil.stencilTestEnable = VK_FALSE;
		depthStencil.minDepthBounds = 0.f;
		depthStencil.maxDepthBounds = 1.f;
		depthStencil.front = {};
		depthStencil.back = {};

		auto DescriptSetLayouts = m_Config.Shader->GetAllDescriptorLayout();
		const auto& PushConstantRanges = m_Config.Shader->GetPushConstants();

		std::vector<VkPushConstantRange> VulkanPushConstantRanges(PushConstantRanges.size());
		for (int x = 0; x < PushConstantRanges.size(); x++)
		{
			const auto& PushConstant = PushConstantRanges[x];
			auto& VulkanPushConstant = VulkanPushConstantRanges[x];

			VulkanPushConstant.stageFlags = PushConstant.ShaderStageFlag;
			VulkanPushConstant.offset = PushConstant.Offset;
			VulkanPushConstant.size = PushConstant.Size;
		}

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(DescriptSetLayouts.size());
		pipelineLayoutInfo.pSetLayouts = DescriptSetLayouts.data();
		pipelineLayoutInfo.pushConstantRangeCount = static_cast<uint32_t>(VulkanPushConstantRanges.size());
		pipelineLayoutInfo.pPushConstantRanges = VulkanPushConstantRanges.data();

		if (auto Result = vkCreatePipelineLayout(Device->GetLogicalDevice(), &pipelineLayoutInfo, nullptr, &m_Layout); Result != VK_SUCCESS)
		{
			TRE_CORE_CRITICAL("Unable to create pipeline");
			assert(Result == VK_SUCCESS);
		}

		const auto& ShaderStages = m_Config.Shader->GetPipelineShaderInfo();

		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = static_cast<uint32_t>(ShaderStages.size());
		pipelineInfo.pStages = ShaderStages.data();
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDynamicState = &dynamicState;
		pipelineInfo.pDepthStencilState = &depthStencil;
		pipelineInfo.layout = m_Layout;
		pipelineInfo.renderPass = TargetRenderPass->GetHandle();
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

		if (auto Result = vkCreateGraphicsPipelines(Device->GetLogicalDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_Pipeline); Result != VK_SUCCESS)
		{
			TRE_CORE_CRITICAL("Pipeline failed to create");
			assert(Result == VK_SUCCESS);
		}
	}

	Pipeline::~Pipeline()
	{
		auto Device = RendererContext::GetDevice();
		vkDestroyPipeline(Device->GetLogicalDevice(), m_Pipeline, nullptr);
		vkDestroyPipelineLayout(Device->GetLogicalDevice(), m_Layout, nullptr);
	}

	VkPrimitiveTopology Pipeline::GetVulkanTopology(PrimitiveType TopologyType)
	{
		switch (TopologyType)
		{
			case PrimitiveType::Point:			return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
			case PrimitiveType::Lines:			return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
			case PrimitiveType::LinesStrip:		return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
			case PrimitiveType::Triangles:		return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
			case PrimitiveType::TrianglesStrip:	return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
			case PrimitiveType::TranglesFan:	return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
		}

		TRE_CORE_ERROR("Unknown toplogy");
		assert(false);
		return VK_PRIMITIVE_TOPOLOGY_MAX_ENUM;
	}
}