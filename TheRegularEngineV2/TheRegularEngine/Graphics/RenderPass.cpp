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

		VkAttachmentDescription attchmentDescriptions{};
		attchmentDescriptions.format = m_Info.ImageFormat;
		attchmentDescriptions.samples = VK_SAMPLE_COUNT_1_BIT;
		attchmentDescriptions.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attchmentDescriptions.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attchmentDescriptions.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attchmentDescriptions.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attchmentDescriptions.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attchmentDescriptions.finalLayout = m_Info.FinalLayout;

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
		renderPassInfo.renderArea.extent = Engine::GetInstance().GetWindow()->GetSwapChain().GetSwapChainExtent();

		VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f}} };
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearColor;

		vkCmdBeginRenderPass(cmdBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
	}

	void RenderPass::EndRenderPass(VkCommandBuffer cmdBuffer)
	{
		vkCmdEndRenderPass(cmdBuffer);
	}



}