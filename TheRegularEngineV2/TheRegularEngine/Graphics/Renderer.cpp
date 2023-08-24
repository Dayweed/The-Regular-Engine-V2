#include "pch.h"
#include "Core/Engine.h"
#include "Renderer.h"
#include "RendererContext.h"
#include "MeshRenderer.h"
#include "imgui_impl_vulkan.h"
#include "Camera.h"
#include "Descriptor.h"

namespace TRE
{
	std::vector<VkImageView>& Renderer::GetImageView()
	{
		return m_ImageView;
	}

	VkSampler Renderer::GetSampler()
	{
		return m_Sampler;
	}

	std::vector<char> Renderer::readFile(const std::string& filename)
	{
		std::ifstream file(filename, std::ios::ate | std::ios::binary);

		if (!file.is_open()) {
			throw std::runtime_error("failed to open file!");
		}

		size_t fileSize = (size_t)file.tellg();
		std::vector<char> buffer(fileSize);

		file.seekg(0);
		file.read(buffer.data(), fileSize);

		file.close();

		return buffer;
	}

	uint32_t Renderer::FindMemoryType(uint32_t memorytypebits, VkMemoryPropertyFlags MemoryPropertyFlags)
	{
		VkPhysicalDeviceMemoryProperties MemoryProperties = RendererContext::GetPhysicalDevice()->GetPhysicalDeviceMemoryProperties();

		for (uint32_t x = 0; x < MemoryProperties.memoryTypeCount; x++)
		{
			if ((memorytypebits & (1 << x)) && (MemoryProperties.memoryTypes[x].propertyFlags & MemoryPropertyFlags) == MemoryPropertyFlags)
			{
				return x;
			}
		}

		std::cout << "Unable to find memory type" << std::endl;
		assert(false);
		return 0;
	}

	Renderer::Renderer(const std::shared_ptr<Device>& Device) : m_Device(Device)
	{
		Create();
		uint32_t ImageCount = Engine::GetInstance().GetWindow()->GetSwapChain().GetImageCount();
		auto SwapChain = Engine::GetInstance().GetWindow()->GetSwapChain();

		// Create sampler to sample from the attachment in the fragment shader
		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.addressModeV = samplerInfo.addressModeU;
		samplerInfo.addressModeW = samplerInfo.addressModeU;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.maxAnisotropy = 1.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 1.0f;
		samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
		
		if (auto Result = vkCreateSampler(m_Device->GetLogicalDevice(), &samplerInfo, nullptr, &m_Sampler); Result != VK_SUCCESS)
		{
			assert(Result == VK_SUCCESS);
		}

		VkCommandPoolCreateInfo CmdPoolCreateInfo{};
		CmdPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		CmdPoolCreateInfo.queueFamilyIndex = SwapChain.GetQueueIndex();
		CmdPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

		VkCommandBufferAllocateInfo CommandBufferAllocateInfo{};
		CommandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		CommandBufferAllocateInfo.commandBufferCount = 1;
		CommandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

		m_Commandbuffers.resize(ImageCount);
		m_CommandPool.resize(ImageCount);

		for (int x = 0; x < ImageCount; x++)
		{
			if (VkResult Result = vkCreateCommandPool(m_Device->GetLogicalDevice(), &CmdPoolCreateInfo, nullptr, &m_CommandPool[x]); Result != VK_SUCCESS)
			{
				std::cout << "Unable to create a command pool" << std::endl;
				assert(Result == VK_SUCCESS);
			}

			CommandBufferAllocateInfo.commandPool = m_CommandPool[x];
			if (VkResult Result = vkAllocateCommandBuffers(m_Device->GetLogicalDevice(), &CommandBufferAllocateInfo, &m_Commandbuffers[x]); Result != VK_SUCCESS)
			{
				std::cout << "Unable to create a command buffer" << std::endl;
				assert(Result == VK_SUCCESS);
			}
		}

		m_DescriptorPool = DescriptorPool::Builder()
			.SetMaxSets(10)
			.AddPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10)
			.AddPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 10)
			.Build();
	}

	void Renderer::Create()
	{
		uint32_t ImageCount = Engine::GetInstance().GetWindow()->GetSwapChain().GetImageCount();
		m_Images.resize(ImageCount);
		m_ImageView.resize(ImageCount);
		m_Memory.resize(ImageCount);
		m_FrameBuffer.resize(ImageCount);

		auto SwapChain = Engine::GetInstance().GetWindow()->GetSwapChain();

		for (int x = 0; x < m_Images.size(); x++)
		{
			// Color attachment
			VkImageCreateInfo image{};
			image.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			image.imageType = VK_IMAGE_TYPE_2D;
			image.format = VK_FORMAT_R8G8B8A8_UNORM;
			image.extent.width = SwapChain.GetWidth();
			image.extent.height = SwapChain.GetHeight();
			image.extent.depth = 1;
			image.mipLevels = 1;
			image.arrayLayers = 1;
			image.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			image.samples = VK_SAMPLE_COUNT_1_BIT;
			image.tiling = VK_IMAGE_TILING_OPTIMAL;
			image.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT; // We will sample directly from the color attachment
			vkCreateImage(m_Device->GetLogicalDevice(), &image, nullptr, &m_Images[x]);

			VkMemoryRequirements memReqs;
			VkMemoryAllocateInfo memAlloc{};
			memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			vkGetImageMemoryRequirements(m_Device->GetLogicalDevice(), m_Images[x], &memReqs);
			memAlloc.allocationSize = memReqs.size;
			memAlloc.memoryTypeIndex = FindMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
			vkAllocateMemory(m_Device->GetLogicalDevice(), &memAlloc, nullptr, &m_Memory[x]);
			vkBindImageMemory(m_Device->GetLogicalDevice(), m_Images[x], m_Memory[x], 0);

			VkImageViewCreateInfo colorImageView{};
			colorImageView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			colorImageView.viewType = VK_IMAGE_VIEW_TYPE_2D;
			colorImageView.format = VK_FORMAT_R8G8B8A8_UNORM;
			colorImageView.subresourceRange = {};
			colorImageView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			colorImageView.subresourceRange.baseMipLevel = 0;
			colorImageView.subresourceRange.levelCount = 1;
			colorImageView.subresourceRange.baseArrayLayer = 0;
			colorImageView.subresourceRange.layerCount = 1;
			colorImageView.image = m_Images[x];
			vkCreateImageView(m_Device->GetLogicalDevice(), &colorImageView, nullptr, &m_ImageView[x]);
		}

		VkAttachmentDescription attchmentDescriptions{};
		attchmentDescriptions.format = VK_FORMAT_R8G8B8A8_UNORM;
		attchmentDescriptions.samples = VK_SAMPLE_COUNT_1_BIT;
		attchmentDescriptions.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attchmentDescriptions.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attchmentDescriptions.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attchmentDescriptions.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attchmentDescriptions.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attchmentDescriptions.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		VkAttachmentReference colorReference = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };

		VkSubpassDescription subpassDescription{};
		subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpassDescription.colorAttachmentCount = 1;
		subpassDescription.pColorAttachments = &colorReference;
		subpassDescription.pDepthStencilAttachment = nullptr;

		VkSubpassDependency dependencies{};
		dependencies.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependencies.dstSubpass = 0;
		dependencies.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies.srcAccessMask = 0;
		dependencies.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		VkRenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = 1;
		renderPassInfo.pAttachments = &attchmentDescriptions;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpassDescription;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependencies;

		if (auto Result = vkCreateRenderPass(m_Device->GetLogicalDevice(), &renderPassInfo, nullptr, &m_Renderpass); Result != VK_SUCCESS)
		{
			assert(Result == VK_SUCCESS);
		}

		for (int x = 0; x < m_FrameBuffer.size(); x++)
		{
			VkFramebufferCreateInfo fbufCreateInfo{};
			fbufCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			fbufCreateInfo.renderPass = m_Renderpass;
			fbufCreateInfo.attachmentCount = 1;
			fbufCreateInfo.pAttachments = &m_ImageView[x];
			fbufCreateInfo.width = SwapChain.GetWidth();
			fbufCreateInfo.height = SwapChain.GetHeight();
			fbufCreateInfo.layers = 1;

			if (auto Result = vkCreateFramebuffer(m_Device->GetLogicalDevice(), &fbufCreateInfo, nullptr, &m_FrameBuffer[x]); Result != VK_SUCCESS)
			{
				assert(Result == VK_SUCCESS);
			}
		}
	}

	void Renderer::Resize()
	{
		for (int x = 0; x < m_FrameBuffer.size(); x++)
		{
			vkDestroyFramebuffer(m_Device->GetLogicalDevice(), m_FrameBuffer[x], nullptr);
			vkDestroyImage(m_Device->GetLogicalDevice(), m_Images[x], nullptr);
			vkDestroyImageView(m_Device->GetLogicalDevice(), m_ImageView[x], nullptr);
			vkFreeMemory(m_Device->GetLogicalDevice(), m_Memory[x], nullptr);
		}
		vkDestroyRenderPass(m_Device->GetLogicalDevice(), m_Renderpass, nullptr);

		Create();
	}

	Renderer::~Renderer()
	{
		vkDeviceWaitIdle(m_Device->GetLogicalDevice());

		for (int x = 0; x < m_Images.size(); x++)
		{
			vkFreeMemory(m_Device->GetLogicalDevice(), m_Memory[x], nullptr);
			vkDestroyImage(m_Device->GetLogicalDevice(), m_Images[x], nullptr);
			vkDestroyImageView(m_Device->GetLogicalDevice(), m_ImageView[x], nullptr);
			vkDestroyFramebuffer(m_Device->GetLogicalDevice(), m_FrameBuffer[x], nullptr);
			vkDestroyCommandPool(m_Device->GetLogicalDevice(), m_CommandPool[x], nullptr);
		}

		vkDestroyDescriptorSetLayout(m_Device->GetLogicalDevice(), m_DescriptorLayout, nullptr);
		vkDestroySampler(m_Device->GetLogicalDevice(), m_Sampler, nullptr);
		vkDestroyRenderPass(m_Device->GetLogicalDevice(), m_Renderpass, nullptr);
		vkDestroyPipeline(m_Device->GetLogicalDevice(), m_GraphicsPipeline, nullptr);
		vkDestroyPipelineLayout(m_Device->GetLogicalDevice(), m_PipelineLayout, nullptr);
	}

	void Renderer::Initialize()
	{
		auto SwapChain = Engine::GetInstance().GetWindow()->GetSwapChain();

		VkDescriptorSetLayoutBinding uboLayoutBinding{};
		uboLayoutBinding.binding = 0;
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboLayoutBinding.descriptorCount = 1;
		uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		VkDescriptorSetLayoutBinding samplerLayoutBinding{};
		samplerLayoutBinding.binding = 0;
		samplerLayoutBinding.descriptorCount = 1;
		samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		
		VkDescriptorSetLayoutBinding bindings[1] = { samplerLayoutBinding };
		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = 1;
		layoutInfo.pBindings = bindings;

		if (auto Result = vkCreateDescriptorSetLayout(m_Device->GetLogicalDevice(), &layoutInfo, nullptr, &m_DescriptorLayout); Result != VK_SUCCESS)
		{
			assert(Result == VK_SUCCESS);
		}

		auto vertShaderCode = readFile("Resources/Shaders/vert.spv");
		auto fragShaderCode = readFile("Resources/Shaders/frag.spv");

		VkShaderModule vertShaderModule = CreateShader(vertShaderCode);
		VkShaderModule fragShaderModule = CreateShader(fragShaderCode);

		VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = vertShaderModule;
		vertShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = fragShaderModule;
		fragShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

		//Vertex input
		auto attributeDescriptions = RenderObject::Vertex::GetAttributeDescriptions();
		auto bindingDescription = RenderObject::Vertex::GetBindingDescriptions();

		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
		vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescription.size());
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
		vertexInputInfo.pVertexBindingDescriptions = bindingDescription.data();

		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		VkViewport viewport{};
		viewport.width = static_cast<float>(SwapChain.GetWidth());
		viewport.height = static_cast<float>(SwapChain.GetHeight());
		viewport.minDepth = 0.f;
		viewport.maxDepth = 1.f;

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = SwapChain.GetSwapChainExtent();

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
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;

		VkPipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
		colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

		VkPipelineColorBlendStateCreateInfo colorBlending{};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY;
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.blendConstants[0] = 0.0f;
		colorBlending.blendConstants[1] = 0.0f;
		colorBlending.blendConstants[2] = 0.0f;
		colorBlending.blendConstants[3] = 0.0f;
		std::vector<VkDynamicState> dynamicStates = 
		{
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR
		};
		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
		dynamicState.pDynamicStates = dynamicStates.data();

		//Create descriptor set layout
		const uint32_t imageCount = Engine::GetInstance().GetWindow()->GetSwapChain().GetImageCount();
		std::vector<std::unique_ptr<Buffer>> UBOBuffers(imageCount);
		for (int i = 0; i < UBOBuffers.size(); i++)
		{
			UBOBuffers[i] = std::make_unique<Buffer>(sizeof(UBO), 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
			UBOBuffers[i]->Map();
		}

		m_DescriptorSetLayouts.push_back(DescriptorSetLayout::Builder()
			.AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
			.Build());

		std::vector<VkDescriptorSet> globalDescriptorsSets(imageCount);
		for (int i = 0; i < globalDescriptorsSets.size(); ++i)
		{
			auto bufferInfo = UBOBuffers[i]->DescriptorInfo(sizeof(UBO), 0);

			DescriptorWriter(*(m_DescriptorSetLayouts[0]), *m_DescriptorPool)
				.WriteBuffer(0, &bufferInfo)
				.Build(globalDescriptorsSets[i]);
		}

		std::vector<VkDescriptorSetLayout> layouts(imageCount, m_DescriptorSetLayouts[0]->GetDescriptorSetLayout());

		//Create pipeline layout
		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT; //Push constant can be accessed from both vertex and fragment shaders
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(PushConstant);

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = &m_DescriptorLayout; //change
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

		if (vkCreatePipelineLayout(m_Device->GetLogicalDevice(), &pipelineLayoutInfo, nullptr, &m_PipelineLayout) != VK_SUCCESS) 
		{
			throw std::runtime_error("failed to create pipeline layout!");
		}

		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderStages;
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDynamicState = &dynamicState;
		pipelineInfo.layout = m_PipelineLayout;
		pipelineInfo.renderPass = m_Renderpass;
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

		if (vkCreateGraphicsPipelines(m_Device->GetLogicalDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_GraphicsPipeline) != VK_SUCCESS) 
		{
			throw std::runtime_error("failed to create graphics pipeline!");
		}

		vkDestroyShaderModule(m_Device->GetLogicalDevice(), fragShaderModule, nullptr);
		vkDestroyShaderModule(m_Device->GetLogicalDevice(), vertShaderModule, nullptr);
	}

	void Renderer::Shutdown()
	{

	}

	void Renderer::BeginFrame()
	{
		uint32_t Index = Engine::GetInstance().GetWindow()->GetSwapChain().GetCurrentBufferIndex();
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if (auto Result = vkBeginCommandBuffer(m_Commandbuffers[Index], &beginInfo); Result != VK_SUCCESS)
		{
			assert(Result == VK_SUCCESS);
		}

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = m_Renderpass;
		renderPassInfo.framebuffer = m_FrameBuffer[Engine::GetInstance().GetWindow()->GetSwapChain().GetCurrentImageIndex()];
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = Engine::GetInstance().GetWindow()->GetSwapChain().GetSwapChainExtent();

		VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f}} };
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearColor;

		vkCmdBeginRenderPass(m_Commandbuffers[Index], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(Engine::GetInstance().GetWindow()->GetSwapChain().GetWidth());
		viewport.height = static_cast<float>(Engine::GetInstance().GetWindow()->GetSwapChain().GetHeight());
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(m_Commandbuffers[Index], 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = Engine::GetInstance().GetWindow()->GetSwapChain().GetSwapChainExtent();
		vkCmdSetScissor(m_Commandbuffers[Index], 0, 1, &scissor);

		//vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipelineLayout, 0, 1, &m_DescriptorSets[Engine::GetInstance().GetWindow()->GetSwapChain().GetCurrentImageIndex()], 0, NULL);
		vkCmdBindPipeline(m_Commandbuffers[Index], VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsPipeline);
		
		//VERY INEFFICIENT
		for (const auto& go_mr : _ecs_manager->GetGO<MeshRenderer>())
		{
			PushConstant pc{};
			pc.m_Model = go_mr->GetComponent<Transform>().GetModelMatrix();
			Camera& mainCamera = _system_manager->GetSystem<CameraSystem>()->GetMainCamera()->GetComponent<Camera>();
			pc.m_ProjView = mainCamera.m_ProjectionMatrix * mainCamera.m_ViewMatrix;

			vkCmdPushConstants(m_Commandbuffers[Index], m_PipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstant), &pc);

			MeshRenderer& mr = (go_mr.get())->GetComponent<MeshRenderer>();
			mr.m_RenderObject->Bind(m_Commandbuffers[Index]);
			mr.m_RenderObject->Draw(m_Commandbuffers[Index]);
		}

		vkCmdEndRenderPass(m_Commandbuffers[Index]);

		if (auto Result = vkEndCommandBuffer(m_Commandbuffers[Index]); Result != VK_SUCCESS)
		{
			assert(Result == VK_SUCCESS);
		}

		VkPipelineStageFlags PipelineStageFlags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		VkSubmitInfo SubmitInfo{};
		SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		SubmitInfo.pWaitDstStageMask = &PipelineStageFlags;
		SubmitInfo.commandBufferCount = 1;
		SubmitInfo.pCommandBuffers = &m_Commandbuffers[Index];

		if (auto Result = vkQueueSubmit(m_Device->GetGraphicsQ(), 1, &SubmitInfo, 0); Result != VK_SUCCESS)
		{
			std::cout << "Unable to queue submit" << std::endl;
			assert(Result == VK_SUCCESS);
		}
	}

	VkShaderModule Renderer::CreateShader(std::vector<char>& code)
	{
		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = code.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

		VkShaderModule shaderModule;
		if (auto Result = vkCreateShaderModule(RendererContext::GetDevice()->GetLogicalDevice(), &createInfo, nullptr, &shaderModule); Result != VK_SUCCESS)
		{
			assert(Result == VK_SUCCESS);
		}

		return shaderModule;
	}
}