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
		colorAttchmentDescriptions.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttchmentDescriptions.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttchmentDescriptions.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttchmentDescriptions.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttchmentDescriptions.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttchmentDescriptions.finalLayout = m_Info.FinalLayout;

		VkAttachmentReference colorAttachmentRef = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };

		VkAttachmentDescription depthAttachment{}; 
		VkAttachmentReference depthAttachmentRef{};
		if (m_Info.DepthEnabled)
		{
			depthAttachment.format = m_Info.DepthImageFormat;
			depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
			depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
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



}