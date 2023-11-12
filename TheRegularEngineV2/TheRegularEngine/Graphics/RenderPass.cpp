#include "pch.h"
#include "RenderPass.h"
#include "TREIncludes.h"

namespace TRE
{
	RenderPassInfo RenderPass::GetInfo()
	{
		return m_Info;
	}

	VkRenderPass RenderPass::GetHandle()
	{
		return m_Renderpass;
	}

	void RenderPass::Recreate()
	{
		if (m_Renderpass != VK_NULL_HANDLE) 
			vkDestroyRenderPass(m_Device->GetLogicalDevice(), m_Renderpass, nullptr);

		VkAttachmentDescription colorAttchmentDescriptions{};
		colorAttchmentDescriptions.format = m_Info.ImageFormat;
		colorAttchmentDescriptions.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttchmentDescriptions.loadOp = m_Info.ClearColor ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
		colorAttchmentDescriptions.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttchmentDescriptions.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttchmentDescriptions.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttchmentDescriptions.initialLayout = colorAttchmentDescriptions.loadOp == VK_ATTACHMENT_LOAD_OP_CLEAR ? VK_IMAGE_LAYOUT_UNDEFINED : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		colorAttchmentDescriptions.finalLayout = m_Info.FinalLayout;

		VkAttachmentReference colorAttachmentRef = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };

		VkAttachmentDescription depthAttachment{}; 
		VkAttachmentReference depthAttachmentRef{};
		if (m_Info.DepthEnabled)
		{
			depthAttachment.format = m_Info.DepthImageFormat;
			depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
			depthAttachment.loadOp = m_Info.ClearColor ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
			depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			depthAttachment.initialLayout = depthAttachment.loadOp == VK_ATTACHMENT_LOAD_OP_CLEAR ? VK_IMAGE_LAYOUT_UNDEFINED : VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			depthAttachment.finalLayout = m_Info.DepthFinalLayout;

			depthAttachmentRef.attachment = 1;
			depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		}

		VkSubpassDescription subpassDescription{};
		subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpassDescription.colorAttachmentCount = 1;
		subpassDescription.pColorAttachments = &colorAttachmentRef;

		if (m_Info.DepthEnabled)
			subpassDescription.pDepthStencilAttachment = &depthAttachmentRef;

		VkSubpassDependency dependencies{};
		dependencies.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependencies.dstSubpass = 0;
		dependencies.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependencies.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependencies.srcAccessMask = 0;
		dependencies.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		std::vector<VkAttachmentDescription> attachments;
		attachments.push_back(colorAttchmentDescriptions);
		if (m_Info.DepthEnabled)
			attachments.push_back(depthAttachment);
		//std::array<VkAttachmentDescription, 2> attachments = { colorAttchmentDescriptions, depthAttachment };
		VkRenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		renderPassInfo.pAttachments = attachments.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpassDescription;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependencies;

		if (auto Result = vkCreateRenderPass(m_Device->GetLogicalDevice(), &renderPassInfo, nullptr, &m_Renderpass); Result != VK_SUCCESS)
		{
			assert(Result == VK_SUCCESS);
		}
	}

	RenderPass::RenderPass(std::shared_ptr<Device> Device, RenderPassInfo Info) : m_Device(Device), m_Info(Info)
	{
		Recreate();
	}

	RenderPass::~RenderPass()
	{
		vkDestroyRenderPass(m_Device->GetLogicalDevice(), m_Renderpass, nullptr);
	}

	void RenderPass::BeginRenderPass(VkCommandBuffer cmdBuffer, VkFramebuffer FrameBuffer)
	{
		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = m_Renderpass;
		renderPassInfo.framebuffer = FrameBuffer;
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = Engine::GetInstance().GetWindow()->GetSwapChain()->GetSwapChainExtent();

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = { 0.01f, 0.01f, 0.01f, 1.0f };
		clearValues[1].depthStencil = { 1.0f, 0 };
		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(cmdBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
	}

	void RenderPass::EndRenderPass(VkCommandBuffer cmdBuffer)
	{
		vkCmdEndRenderPass(cmdBuffer);
	}

	RenderPass::RenderPass(std::shared_ptr<Device> Device, bool isshadow) : m_Device(Device)
	{
		(void)isshadow;
		//Renderpass
		VkAttachmentDescription attachmentDescription{};
		attachmentDescription.format = VK_FORMAT_D16_UNORM;
		attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
		attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;							// Clear depth at beginning of the render pass
		attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;						// We will read from depth, so it's important to store the depth attachment results
		attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;					// We don't care about initial layout of the attachment
		attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;// Attachment will be transitioned to shader read at render pass end

		VkAttachmentReference depthReference = {};
		depthReference.attachment = 0;
		depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;			// Attachment will be used as depth/stencil during render pass

		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 0;													// No color attachments
		subpass.pDepthStencilAttachment = &depthReference;									// Reference to our depth attachment

		// Use subpass dependencies for layout transitions
		std::array<VkSubpassDependency, 2> dependencies;

		dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[0].dstSubpass = 0;
		dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		dependencies[0].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
		dependencies[0].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		dependencies[1].srcSubpass = 0;
		dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[1].srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		dependencies[1].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		VkRenderPassCreateInfo renderPassCreateInfo{};
		renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassCreateInfo.attachmentCount = 1;
		renderPassCreateInfo.pAttachments = &attachmentDescription;
		renderPassCreateInfo.subpassCount = 1;
		renderPassCreateInfo.pSubpasses = &subpass;
		renderPassCreateInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
		renderPassCreateInfo.pDependencies = dependencies.data();

		if (auto Result = vkCreateRenderPass(Device->GetLogicalDevice(), &renderPassCreateInfo, nullptr, &m_Renderpass); Result != VK_SUCCESS)
		{
			assert(Result == VK_SUCCESS && "Unable to create image sampler for shadow");
		}
	}

}