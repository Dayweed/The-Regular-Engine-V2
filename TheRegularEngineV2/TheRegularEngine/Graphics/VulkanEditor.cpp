#include "pch.h"
#include "TREIncludes.h"
#include "Imgui/imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#include "Editor/ImGuizmo.h"
#include "implot.h"

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

		if (auto Result = vkCreateSampler(m_LogicalDevice->GetLogicalDevice(), &samplerInfo, nullptr, &m_Sampler); Result != VK_SUCCESS)
		{
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
		
		m_DescriptorSets.resize(RendererContext::GetFramesInFlight());
		for (int x = 0; x < m_DescriptorSets.size(); x++)
		{
			m_DescriptorSets[x] = ImGui_ImplVulkan_AddTexture(m_Sampler, Engine::GetInstance().GetMainSceneRenderer()->GetColorImages()[x]->GetImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		}
	}

	void VulkanEditor::SetUpImgui()
	{
		ImGui::CreateContext();
		ImPlot::CreateContext();
		ImGui::StyleColorsDark();

		ImGuiIO& IO = ImGui::GetIO();
		IO.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		IO.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
		IO.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

		ImGui::StyleColorsDark();
		SetImguiTheme();
	}

	void VulkanEditor::BeginFrame()
	{
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		ImGuizmo::BeginFrame();
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
		m_DescriptorSets.resize(RendererContext::GetFramesInFlight());
		for (int x = 0; x < m_DescriptorSets.size(); x++)
		{
			m_DescriptorSets[x] = ImGui_ImplVulkan_AddTexture(m_Sampler, Engine::GetInstance().GetMainSceneRenderer()->GetColorImages()[x]->GetImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		}
	}

	VulkanEditor::~VulkanEditor()
	{
		vkDeviceWaitIdle(m_LogicalDevice->GetLogicalDevice());
		vkDestroyDescriptorPool(m_LogicalDevice->GetLogicalDevice(), m_DescriptorPool, nullptr);
		vkDestroySampler(m_LogicalDevice->GetLogicalDevice(), m_Sampler, nullptr);
		ImPlot::DestroyContext();
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	void VulkanEditor::SetImguiTheme()
	{
		auto& style = ImGui::GetStyle();
		auto& colors = ImGui::GetStyle().Colors;

		// Headers
		colors[ImGuiCol_Header] = ImGui::ColorConvertU32ToFloat4(IM_COL32(47, 47, 47, 255));
		colors[ImGuiCol_HeaderHovered] = ImGui::ColorConvertU32ToFloat4(IM_COL32(47, 47, 47, 255));
		colors[ImGuiCol_HeaderActive] = ImGui::ColorConvertU32ToFloat4(IM_COL32(47, 47, 47, 255));

		// Buttons
		colors[ImGuiCol_Button] = ImColor(56, 56, 56, 200);
		colors[ImGuiCol_ButtonHovered] = ImColor(70, 70, 70, 255);
		colors[ImGuiCol_ButtonActive] = ImColor(56, 56, 56, 150);

		// Frame BG
		colors[ImGuiCol_FrameBg] = ImGui::ColorConvertU32ToFloat4(IM_COL32(15, 15, 15, 255));
		colors[ImGuiCol_FrameBgHovered] = ImGui::ColorConvertU32ToFloat4(IM_COL32(15, 15, 15, 255));
		colors[ImGuiCol_FrameBgActive] = ImGui::ColorConvertU32ToFloat4(IM_COL32(15, 15, 15, 255));

		// Tabs
		colors[ImGuiCol_Tab] = ImGui::ColorConvertU32ToFloat4(IM_COL32(21, 21, 21, 255));
		colors[ImGuiCol_TabHovered] = ImColor(255, 225, 135, 30);
		colors[ImGuiCol_TabActive] = ImColor(255, 225, 135, 60);
		colors[ImGuiCol_TabUnfocused] = ImGui::ColorConvertU32ToFloat4(IM_COL32(21, 21, 21, 255));
		colors[ImGuiCol_TabUnfocusedActive] = colors[ImGuiCol_TabHovered];

		// Title
		colors[ImGuiCol_TitleBg] = ImGui::ColorConvertU32ToFloat4(IM_COL32(21, 21, 21, 255));
		colors[ImGuiCol_TitleBgActive] = ImGui::ColorConvertU32ToFloat4(IM_COL32(21, 21, 21, 255));
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

		// Resize Grip
		colors[ImGuiCol_ResizeGrip] = ImVec4(0.91f, 0.91f, 0.91f, 0.25f);
		colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.81f, 0.81f, 0.81f, 0.67f);
		colors[ImGuiCol_ResizeGripActive] = ImVec4(0.46f, 0.46f, 0.46f, 0.95f);

		// Scrollbar
		colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
		colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.0f);
		colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.0f);
		colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.0f);

		// Check Mark
		colors[ImGuiCol_CheckMark] = ImColor(200, 200, 200, 255);

		// Slider
		colors[ImGuiCol_SliderGrab] = ImVec4(0.51f, 0.51f, 0.51f, 0.7f);
		colors[ImGuiCol_SliderGrabActive] = ImVec4(0.66f, 0.66f, 0.66f, 1.0f);

		// Text
		colors[ImGuiCol_Text] = ImGui::ColorConvertU32ToFloat4(IM_COL32(192, 192, 192, 255));

		// Checkbox
		colors[ImGuiCol_CheckMark] = ImGui::ColorConvertU32ToFloat4(IM_COL32(192, 192, 192, 255));

		// Separator
		colors[ImGuiCol_Separator] = ImGui::ColorConvertU32ToFloat4(IM_COL32(26, 26, 26, 255));
		colors[ImGuiCol_SeparatorActive] = ImGui::ColorConvertU32ToFloat4(IM_COL32(39, 185, 242, 255));
		colors[ImGuiCol_SeparatorHovered] = ImColor(39, 185, 242, 150);

		// Window Background
		colors[ImGuiCol_WindowBg] = ImGui::ColorConvertU32ToFloat4(IM_COL32(21, 21, 21, 255));
		colors[ImGuiCol_ChildBg] = ImGui::ColorConvertU32ToFloat4(IM_COL32(36, 36, 36, 255));
		colors[ImGuiCol_PopupBg] = ImGui::ColorConvertU32ToFloat4(IM_COL32(50, 50, 50, 255));
		colors[ImGuiCol_Border] = ImGui::ColorConvertU32ToFloat4(IM_COL32(26, 26, 26, 255));

		// Tables
		colors[ImGuiCol_TableHeaderBg] = ImGui::ColorConvertU32ToFloat4(IM_COL32(47, 47, 47, 255));
		colors[ImGuiCol_TableBorderLight] = ImGui::ColorConvertU32ToFloat4(IM_COL32(26, 26, 26, 255));

		// Menubar
		colors[ImGuiCol_MenuBarBg] = ImVec4{ 0.0f, 0.0f, 0.0f, 0.0f };

		//========================================================
		/// Style
		style.FrameRounding = 2.5f;
		style.FrameBorderSize = 1.0f;
		style.IndentSpacing = 11.0f;
	}
}