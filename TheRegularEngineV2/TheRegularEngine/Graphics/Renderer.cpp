#include "pch.h"
#include "Core/Engine.h"
#include "Core/Transform.h"
#include "Renderer.h"
#include "RendererContext.h"
#include "MeshRenderer.h"
#include "imgui_impl_vulkan.h"
#include "Camera.h"
#include "Core/Logger.h"
#include "ShaderCompiler.h"
#include "VulkanTexture.h"

namespace TRE
{
	std::shared_ptr<DescriptorPool>& Renderer::GetDescriptorPool()
	{
		return m_DescriptorPool;
	}

	std::vector<std::unique_ptr<Image>>& Renderer::GetColorImages()
	{
		return m_ColorImages;
	}

	VkSampler Renderer::GetSampler()
	{
		return m_Sampler;
	}

	Renderer::Renderer(const std::shared_ptr<Device>& Device) : m_Device(Device)
	{
		auto SwapChain = Engine::GetInstance().GetWindow()->GetSwapChain();
		uint32_t ImageCount = Engine::GetInstance().GetWindow()->GetSwapChain()->GetImageCount();

		Create();

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
		CmdPoolCreateInfo.queueFamilyIndex = SwapChain->GetQueueIndex();
		CmdPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

		VkCommandBufferAllocateInfo CommandBufferAllocateInfo{};
		CommandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		CommandBufferAllocateInfo.commandBufferCount = 1;
		CommandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

		m_Commandbuffers.resize(ImageCount);
		m_CommandPool.resize(ImageCount);

		for (uint32_t x = 0; x < ImageCount; x++)
		{
			if (VkResult Result = vkCreateCommandPool(m_Device->GetLogicalDevice(), &CmdPoolCreateInfo, nullptr, &m_CommandPool[x]); Result != VK_SUCCESS)
			{
				TRE_CORE_ERROR("Unable to create a command pool");
				assert(Result == VK_SUCCESS);
			}

			CommandBufferAllocateInfo.commandPool = m_CommandPool[x];
			if (VkResult Result = vkAllocateCommandBuffers(m_Device->GetLogicalDevice(), &CommandBufferAllocateInfo, &m_Commandbuffers[x]); Result != VK_SUCCESS)
			{
				TRE_CORE_ERROR("Unable to create a command buffer");
				assert(Result == VK_SUCCESS);
			}
		}

		m_DescriptorPool = DescriptorPool::Builder()
			.SetMaxSets(10)
			.AddPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 100)
			.AddPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 100)
			.Build();

		m_UBOBuffer = std::make_shared<UniformBuffer>(sizeof(UBO), 0);
	}

	void Renderer::Initialize()
	{
		auto SwapChain = Engine::GetInstance().GetWindow()->GetSwapChain();

		RenderPassInfo RenderPassCreateInfo{};
		RenderPassCreateInfo.FinalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		RenderPassCreateInfo.ImageFormat = SwapChain->GetColorFormat();
		RenderPassCreateInfo.DepthImageFormat = SwapChain->GetDepthFormat();
		RenderPassCreateInfo.DepthFinalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		std::shared_ptr<RenderPass> renderpass = std::make_shared<RenderPass>(m_Device, RenderPassCreateInfo);

		CreateFrameBuffer(renderpass);

		std::shared_ptr<Shader> VertShader = std::make_shared<Shader>();
		VertShader = ShaderCompiler::CompileShader("Resources/Shaders/Template.vert", VK_SHADER_STAGE_VERTEX_BIT);

		std::shared_ptr<Shader> FragShader = std::make_shared<Shader>();
		FragShader = ShaderCompiler::CompileShader("Resources/Shaders/Template.frag", VK_SHADER_STAGE_FRAGMENT_BIT);

		PipelineConfigurations PipelineConfig;
		PipelineConfig.Primitive = PrimitiveType::Triangles;
		PipelineConfig.RenderPass = renderpass;
		PipelineConfig.VertexShader = VertShader;
		PipelineConfig.FragmentShader = FragShader;
		m_Pipeline = std::make_unique<Pipeline>(PipelineConfig);

		//TO DELETE
		Texture::RunCompiler("../Assets/Test.desc");
		auto texture = Texture::Deserialize("../Assets/Test.DDS");
		TextureManager::Instance().LoadTexture(std::move(texture));



		
	}

	void Renderer::CreateFrameBuffer(std::shared_ptr<RenderPass>& renderpass)
	{
		uint32_t ImageCount = Engine::GetInstance().GetWindow()->GetSwapChain()->GetImageCount();
		auto SwapChain = Engine::GetInstance().GetWindow()->GetSwapChain();
		m_FrameBuffer.resize(ImageCount);
		for (int x = 0; x < m_FrameBuffer.size(); x++)
		{
			VkFramebufferCreateInfo fbufCreateInfo{};
			fbufCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			fbufCreateInfo.renderPass = renderpass->GetHandle();
			std::array<VkImageView, 2> attachments = { m_ColorImages[x]->GetImageView(), m_DepthImages[x]->GetImageView() };
			fbufCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
			fbufCreateInfo.pAttachments = attachments.data();
			fbufCreateInfo.width = SwapChain->GetWidth();
			fbufCreateInfo.height = SwapChain->GetHeight();
			fbufCreateInfo.layers = 1;

			if (auto Result = vkCreateFramebuffer(m_Device->GetLogicalDevice(), &fbufCreateInfo, nullptr, &m_FrameBuffer[x]); Result != VK_SUCCESS)
			{
				assert(Result == VK_SUCCESS);
			}
		}
	}

	void Renderer::Create()
	{
		uint32_t ImageCount = Engine::GetInstance().GetWindow()->GetSwapChain()->GetImageCount();
		m_ColorImages.resize(ImageCount);
		m_DepthImages.resize(ImageCount);

		auto SwapChain = Engine::GetInstance().GetWindow()->GetSwapChain();

		// Color attachment
		for (int x = 0; x < m_ColorImages.size(); x++)
		{
			m_ColorImages[x] = std::make_unique<Image>(SwapChain->GetWidth(), SwapChain->GetHeight(), SwapChain->GetColorFormat(),
				VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
		}

		// Depth attachment
		for (int x = 0; x < m_DepthImages.size(); x++)
		{
			m_DepthImages[x] = std::make_unique<Image>(SwapChain->GetWidth(), SwapChain->GetHeight(), SwapChain->GetDepthFormat(),
								VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_ASPECT_DEPTH_BIT);
		}
	}

	void Renderer::Resize()
	{
		for (int x = 0; x < m_FrameBuffer.size(); x++)
		{
			vkDestroyFramebuffer(m_Device->GetLogicalDevice(), m_FrameBuffer[x], nullptr);
		}

		m_ColorImages.clear();
		m_DepthImages.clear();

		Create();
		CreateFrameBuffer(m_Pipeline->GetConfig().RenderPass);
	}

	Renderer::~Renderer()
	{
		vkDeviceWaitIdle(m_Device->GetLogicalDevice());
		TextureManager::Instance().Shutdown();
		for (int x = 0; x < m_ColorImages.size(); x++)
		{
			vkDestroyFramebuffer(m_Device->GetLogicalDevice(), m_FrameBuffer[x], nullptr);
			vkDestroyCommandPool(m_Device->GetLogicalDevice(), m_CommandPool[x], nullptr);
		}

		m_ColorImages.clear();
		m_DepthImages.clear();

		vkDestroySampler(m_Device->GetLogicalDevice(), m_Sampler, nullptr);
	}

	void Renderer::Shutdown()
	{

	}

	void Renderer::BeginFrame()
	{
		uint32_t Index = Engine::GetInstance().GetWindow()->GetSwapChain()->GetCurrentBufferIndex();
		uint32_t ImageIndex = Engine::GetInstance().GetWindow()->GetSwapChain()->GetCurrentImageIndex();


		std::vector<VkWriteDescriptorSet> Writes;
		for (auto x : m_Pipeline->GetConfig().VertexShader->GetWriteDescriptorSets())
		{
			//x.second.pBufferInfo = &BufferInfo;
			x.second.pBufferInfo = &m_UBOBuffer->GetDescriptorBufferInfo();
			x.second.dstSet = m_Pipeline->GetDescriptorSets()[Index];
			Writes.push_back(x.second);
		}
		VkDescriptorImageInfo imageInfo = TextureManager::Instance().GetTexture("Test")->GetDescriptorImageInfo();
		for (auto x : m_Pipeline->GetConfig().FragmentShader->GetWriteDescriptorSets())
		{
			x.second.dstBinding = 1;
			x.second.pImageInfo = &imageInfo;
			x.second.dstSet = m_Pipeline->GetDescriptorSets()[Index];
			Writes.push_back(x.second);
		}
		vkUpdateDescriptorSets(RendererContext::GetDevice()->GetLogicalDevice(), static_cast<uint32_t>(Writes.size()), Writes.data(), 0, nullptr);

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if (auto Result = vkBeginCommandBuffer(m_Commandbuffers[Index], &beginInfo); Result != VK_SUCCESS)
		{
			assert(Result == VK_SUCCESS);
		}

		//UBO
		UBO ubo{};
		const Camera& mainCamera = ECSSystemManager::Instance().GetSystem<CameraSystem>()->GetMainCamera()->GetComponent<Camera>();
		ubo.m_ProjView = mainCamera.m_ProjectionMatrix * mainCamera.m_ViewMatrix;
		m_UBOBuffer->SetData(&ubo, sizeof(UBO));

		m_Pipeline->GetConfig().RenderPass->BeginRenderPass(m_Commandbuffers[Index], m_FrameBuffer[ImageIndex]);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.f;
		viewport.width = static_cast<float>(Engine::GetInstance().GetWindow()->GetSwapChain()->GetWidth());
		viewport.height = static_cast<float>(Engine::GetInstance().GetWindow()->GetSwapChain()->GetHeight());
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(m_Commandbuffers[Index], 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = Engine::GetInstance().GetWindow()->GetSwapChain()->GetSwapChainExtent();
		vkCmdSetScissor(m_Commandbuffers[Index], 0, 1, &scissor);

		vkCmdBindPipeline(m_Commandbuffers[Index], VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline->GetPipeline());
		vkCmdBindDescriptorSets(m_Commandbuffers[Index], VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline->GetPipelineLayout(), 0, 1, &m_Pipeline->GetDescriptorSets()[Index], 0, NULL);

		//VERY INEFFICIENT
		for (const auto& go_mr : ECSManager::Instance().GetEntities<MeshRenderer>())
		{
			PushConstant pc{};
			pc.m_Model = go_mr->GetComponent<Transform>().GetModelMatrix();
			vkCmdPushConstants(m_Commandbuffers[Index], m_Pipeline->GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstant), &pc);

			MeshRenderer& mr = (go_mr.get())->GetComponent<MeshRenderer>();
			if(mr.m_RenderObject == nullptr)
				continue;
			mr.m_RenderObject->Bind(m_Commandbuffers[Index]);
			mr.m_RenderObject->Draw(m_Commandbuffers[Index]);
		}

		m_Pipeline->GetConfig().RenderPass->EndRenderPass(m_Commandbuffers[Index]);

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
			TRE_CORE_ERROR("Unable to queue submit");
			assert(Result == VK_SUCCESS);
		}
	}
}