#include "pch.h"
#include "TREIncludes.h"
#include "Imgui/imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

namespace TRE
{
	VkDescriptorSet VulkanEditor::GetDset()
	{
		return m_DescriptorSets[Engine::GetInstance().GetWindow()->GetSwapChain()->GetCurrentImageIndex()];
	}

	VulkanEditor::VulkanEditor(const std::shared_ptr<Device>& LogicalDevice)
	{
		m_LogicalDevice = LogicalDevice;

		//For now it may be very oversized, copied from imgui demo. Can be optimized in future.
		VkDescriptorPoolSize pool_sizes[] =
		{
			{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
		};

		VkDescriptorPoolCreateInfo DescriptorPoolCreateInfo{};
		DescriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		DescriptorPoolCreateInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		DescriptorPoolCreateInfo.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);
		DescriptorPoolCreateInfo.poolSizeCount = static_cast<uint32_t>(std::size(pool_sizes));
		DescriptorPoolCreateInfo.pPoolSizes = pool_sizes;
		
		if (auto Result = vkCreateDescriptorPool(RendererContext::GetDevice()->GetLogicalDevice(), &DescriptorPoolCreateInfo, nullptr, &m_DescriptorPool); Result != VK_SUCCESS)
		{
			TRE_CORE_WARN("Unable to create descriptor pool for imgui");
			assert(Result == VK_SUCCESS);
		}

		SetUpImgui();
		
		ImGui_ImplGlfw_InitForVulkan(Engine::GetInstance().GetWindow()->GetWindowHandle(), true);

		auto ImageCount = Engine::GetInstance().GetWindow()->GetSwapChain()->GetImageCount();

		ImGui_ImplVulkan_InitInfo ImguiVulkanInitInfo{};
		ImguiVulkanInitInfo.Instance = RendererContext::GetVKInstance();
		ImguiVulkanInitInfo.PhysicalDevice = LogicalDevice->GetPhysicalDevice()->GetPhysicalDevice();
		ImguiVulkanInitInfo.Device = LogicalDevice->GetLogicalDevice();
		ImguiVulkanInitInfo.QueueFamily = m_LogicalDevice->GetPhysicalDevice()->GetQueueFamilies().Graphics;
		ImguiVulkanInitInfo.Queue = m_LogicalDevice->GetGraphicsQ();
		ImguiVulkanInitInfo.DescriptorPool = m_DescriptorPool;
		ImguiVulkanInitInfo.MinImageCount = 2;
		ImguiVulkanInitInfo.ImageCount = ImageCount;
		ImguiVulkanInitInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

		ImGui_ImplVulkan_Init(&ImguiVulkanInitInfo, Engine::GetInstance().GetWindow()->GetSwapChain()->GetRenderPass());

		auto cmdbuffer = LogicalDevice->AllocateCommandBuffer(true);
		ImGui_ImplVulkan_CreateFontsTexture(cmdbuffer);
		LogicalDevice->SubmitCommands(cmdbuffer);
		vkDeviceWaitIdle(LogicalDevice->GetLogicalDevice());
		ImGui_ImplVulkan_DestroyFontUploadObjects();

		m_ImGuiCommandBuffers.resize(ImageCount);
		for (uint32_t x = 0; x < ImageCount; x++)
		{
			m_ImGuiCommandBuffers[x] = LogicalDevice->AllocateSecondaryCommandBuffer();
		}
		
		auto Renderer = Engine::GetInstance().GetRenderer();
		m_DescriptorSets.resize(Engine::GetInstance().GetRenderer()->GetColorImages().size());
		for (int x = 0; x < m_DescriptorSets.size(); x++)
		{
			m_DescriptorSets[x] = ImGui_ImplVulkan_AddTexture(Renderer->GetSampler(), Renderer->GetColorImages()[x]->GetImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		}
	}

	void VulkanEditor::SetUpImgui()
	{
		ImGui::CreateContext();
		ImGui::StyleColorsDark();

		ImGuiIO& IO = ImGui::GetIO();
		IO.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		IO.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
		IO.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	}

	void VulkanEditor::BeginFrame()
	{
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}

	void VulkanEditor::EndFrame()
	{
		ImGui::Render();

		auto swapChain = Engine::GetInstance().GetWindow()->GetSwapChain();

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = { 0.01f, 0.01f, 0.01f, 1.0f };
		clearValues[1].depthStencil = { 1.0f, 0 };

		uint32_t width = swapChain->GetWidth();
		uint32_t height = swapChain->GetHeight();

		uint32_t commandBufferIndex = swapChain->GetCurrentBufferIndex();

		VkCommandBufferBeginInfo drawCmdBufInfo = {};
		drawCmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		drawCmdBufInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		drawCmdBufInfo.pNext = nullptr;

		VkCommandBuffer drawCommandBuffer = swapChain->GetCurrentCommandBuffer();
		if (auto Result = vkBeginCommandBuffer(drawCommandBuffer, &drawCmdBufInfo); Result != VK_SUCCESS)
		{
			assert(Result == VK_SUCCESS);
		}

		VkRenderPassBeginInfo renderPassBeginInfo = {};
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.pNext = nullptr;
		renderPassBeginInfo.renderPass = swapChain->GetRenderPass();
		renderPassBeginInfo.renderArea.offset.x = 0;
		renderPassBeginInfo.renderArea.offset.y = 0;
		renderPassBeginInfo.renderArea.extent.width = width;
		renderPassBeginInfo.renderArea.extent.height = height;
		renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassBeginInfo.pClearValues = clearValues.data();
		renderPassBeginInfo.framebuffer = swapChain->GetCurrentFrameBuffer();

		vkCmdBeginRenderPass(drawCommandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

		VkCommandBufferInheritanceInfo inheritanceInfo = {};
		inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
		inheritanceInfo.renderPass = swapChain->GetRenderPass();
		inheritanceInfo.framebuffer = swapChain->GetCurrentFrameBuffer();

		VkCommandBufferBeginInfo cmdBufInfo = {};
		cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		cmdBufInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
		cmdBufInfo.pInheritanceInfo = &inheritanceInfo;

		if (auto Result = vkBeginCommandBuffer(m_ImGuiCommandBuffers[commandBufferIndex], &cmdBufInfo); Result != VK_SUCCESS)
		{
			assert(Result == VK_SUCCESS);
		}

		VkViewport viewport = {};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.height = (float)height;
		viewport.width = (float)width;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(m_ImGuiCommandBuffers[commandBufferIndex], 0, 1, &viewport);

		VkRect2D scissor = {};
		scissor.extent.width = width;
		scissor.extent.height = height;
		scissor.offset.x = 0;
		scissor.offset.y = 0;
		vkCmdSetScissor(m_ImGuiCommandBuffers[commandBufferIndex], 0, 1, &scissor);

		ImDrawData* main_draw_data = ImGui::GetDrawData();
		ImGui_ImplVulkan_RenderDrawData(main_draw_data, m_ImGuiCommandBuffers[commandBufferIndex]);

		vkEndCommandBuffer(m_ImGuiCommandBuffers[commandBufferIndex]);

		std::vector<VkCommandBuffer> commandBuffers;
		commandBuffers.push_back(m_ImGuiCommandBuffers[commandBufferIndex]);

		vkCmdExecuteCommands(drawCommandBuffer, uint32_t(commandBuffers.size()), commandBuffers.data());

		vkCmdEndRenderPass(drawCommandBuffer);
		vkEndCommandBuffer(drawCommandBuffer);

		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}
	}

	void VulkanEditor::Resize()
	{
		vkFreeDescriptorSets(m_LogicalDevice->GetLogicalDevice(), m_DescriptorPool, static_cast<uint32_t>(m_DescriptorSets.size()), m_DescriptorSets.data());
		auto Renderer = Engine::GetInstance().GetRenderer();
		m_DescriptorSets.resize(Engine::GetInstance().GetRenderer()->GetColorImages().size());
		for (int x = 0; x < m_DescriptorSets.size(); x++)
		{
			m_DescriptorSets[x] = ImGui_ImplVulkan_AddTexture(Renderer->GetSampler(), Renderer->GetColorImages()[x]->GetImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		}
	}

	VulkanEditor::~VulkanEditor()
	{
		vkDeviceWaitIdle(m_LogicalDevice->GetLogicalDevice());
		vkDestroyDescriptorPool(m_LogicalDevice->GetLogicalDevice(), m_DescriptorPool, nullptr);
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}
}