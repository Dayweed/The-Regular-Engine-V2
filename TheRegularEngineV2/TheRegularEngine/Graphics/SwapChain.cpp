#include "pch.h"
#include "SwapChain.h"
#include "Core/Logger.h"
#include "Core/Engine.h"
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

namespace TRE
{
	VkImageView SwapChain::GetCurrentSwapChainImageView(int Index)
	{
		return m_SwapChainImages[Index].ImageView;
	}

	uint32_t SwapChain::GetQueueIndex()
	{
		return m_QueueIndex;
	}

	SwapChain::SwapChainSettings SwapChain::GetSwapChainSettings()
	{
		return m_SwapChainSettings;
	}

	uint32_t SwapChain::GetImageCount()
	{
		return m_ImageCount;
	}

	uint32_t SwapChain::GetWidth()
	{
		return m_Width;
	}

	uint32_t SwapChain::GetHeight()
	{
		return m_Height;
	}

	VkFramebuffer SwapChain::GetCurrentFrameBuffer()
	{
		assert(m_CurrentImageIndex < m_SwapChainFramebuffers.size()); //Cannot go out of bound
		return m_SwapChainFramebuffers[m_CurrentImageIndex];
	}

	VkCommandBuffer SwapChain::GetCurrentCommandBuffer()
	{
		assert(m_CurrentBufferIndex < m_Commandbuffers.size()); //Cannot go out of bound
		return m_Commandbuffers[m_CurrentBufferIndex];
	}

	uint32_t SwapChain::GetCurrentBufferIndex()
	{
		return m_CurrentBufferIndex;
	}

	uint32_t SwapChain::GetCurrentImageIndex()
	{
		return m_CurrentImageIndex;
	}

	VkFormat SwapChain::GetColorFormat()
	{
		return m_SwapChainSettings.m_SurfaceFormat;
	}

	VkFormat SwapChain::GetDepthFormat()
	{
		return m_SwapChainSettings.m_DepthFormat;
	}

	VkSemaphore SwapChain::GetRenderComplete()
	{
		return m_Semaphores[m_CurrentBufferIndex].RenderComplete;
	}

	VkRenderPass SwapChain::GetRenderPass()
	{
		return m_Renderpass->GetHandle();
	}

	VkExtent2D SwapChain::GetSwapChainExtent()
	{
		return m_Extent;
	}

	SwapChain::SwapChain(std::shared_ptr<Device>& LogicalDevice, std::shared_ptr<PhysicalDevice>& PD, GLFWwindow* Handle) : m_LogicalDevice(LogicalDevice), m_PhysicalDevice(PD)
	{
		if (auto Result = glfwCreateWindowSurface(RendererContext::GetVKInstance(), Handle, nullptr, &m_WindowSurface) != VK_SUCCESS)
		{
			assert(false);
		}
	}

	void SwapChain::DestroySwapChain()
	{
		vkDeviceWaitIdle(m_LogicalDevice->GetLogicalDevice());
		for (int x = 0; x < MAX_FRAMES_IN_FLIGHT; x++)
		{
			vkDestroyFence(m_LogicalDevice->GetLogicalDevice(), m_WaitFences[x], nullptr);
			vkDestroySemaphore(m_LogicalDevice->GetLogicalDevice(), m_Semaphores[x].PresentComplete, nullptr);
			vkDestroySemaphore(m_LogicalDevice->GetLogicalDevice(), m_Semaphores[x].RenderComplete, nullptr);
		}
		CleanSwapChain();
		vkDestroySwapchainKHR(m_LogicalDevice->GetLogicalDevice(), m_SwapChain, nullptr);
		vkDestroyCommandPool(m_LogicalDevice->GetLogicalDevice(), m_CommandPool, nullptr);
		vkDestroySurfaceKHR(RendererContext::GetVKInstance(), m_WindowSurface, nullptr);
	}

	void SwapChain::Initialize(uint32_t Width, uint32_t Height)
	{
		m_QueueIndex = m_PhysicalDevice->GetQueueFamilies().Graphics;

		CreateSwapChain(Width, Height);
		CreateImageViews();
		CreateRenderPass();
		CreateCommandPool();
		CreateFrameBuffer();
		CreateCommandbuffer();
		CreateSyncObjects();
	}

	void SwapChain::BeginFrame()
	{
		vkWaitForFences(m_LogicalDevice->GetLogicalDevice(), 1, &m_WaitFences[m_CurrentBufferIndex], VK_TRUE, UINT64_MAX); //Wait for previous frame to finish //uin64_max disable timeout
		VkResult Result = vkAcquireNextImageKHR(m_LogicalDevice->GetLogicalDevice(), m_SwapChain, UINT64_MAX, m_Semaphores[m_CurrentBufferIndex].PresentComplete, VK_NULL_HANDLE, &m_CurrentImageIndex);
		if (Result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			TRE_CORE_INFO("No Acquire");
			RecreateSwapChain();
			return;
		}

		vkResetFences(m_LogicalDevice->GetLogicalDevice(), 1, &m_WaitFences[m_CurrentBufferIndex]);

		vkResetCommandBuffer(m_Commandbuffers[m_CurrentBufferIndex], 0);
	}

	void SwapChain::Present()
	{
		VkSubmitInfo SubmitInfo{};
		SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore WaitSemaphores[] = { m_Semaphores[m_CurrentBufferIndex].PresentComplete };
		VkPipelineStageFlags WaitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		SubmitInfo.waitSemaphoreCount = 1;
		SubmitInfo.pWaitSemaphores = WaitSemaphores;
		SubmitInfo.pWaitDstStageMask = WaitStages;
		SubmitInfo.commandBufferCount = 1;
		SubmitInfo.pCommandBuffers = &m_Commandbuffers[m_CurrentBufferIndex];

		VkSemaphore SingalSemaphores[] = { m_Semaphores[m_CurrentBufferIndex].RenderComplete };
		SubmitInfo.signalSemaphoreCount = 1;
		SubmitInfo.pSignalSemaphores = SingalSemaphores;

		if (vkQueueSubmit(m_LogicalDevice->GetGraphicsQ(), 1, &SubmitInfo, m_WaitFences[m_CurrentBufferIndex]) != VK_SUCCESS)
		{
			assert(false);
		}

		VkPresentInfoKHR PresentInfo{};
		PresentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		PresentInfo.waitSemaphoreCount = 1;
		PresentInfo.pWaitSemaphores = SingalSemaphores;

		VkSwapchainKHR SwapChains[] = { m_SwapChain };
		PresentInfo.swapchainCount = 1;
		PresentInfo.pSwapchains = SwapChains;
		PresentInfo.pImageIndices = &m_CurrentImageIndex;
		PresentInfo.pResults = nullptr; //Only if using more than 1 swapchain

		auto Result = vkQueuePresentKHR(m_LogicalDevice->GetGraphicsQ(), &PresentInfo);

		if (Result == VK_ERROR_OUT_OF_DATE_KHR || Result == VK_SUBOPTIMAL_KHR || Engine::GetInstance().GetWindow()->GetWindowConfig().resize)
		{
			TRE_CORE_INFO("Recreate");
			Engine::GetInstance().GetWindow()->GetWindowConfig().resize = false;
			RecreateSwapChain();
			Engine::GetInstance().GetMainSceneRenderer()->Resize();
			if (Engine::GetInstance().GetEngineInfo().EnableEditor)
			{
				Engine::GetInstance().GetEditorSceneRenderer()->Resize();
				Engine::GetInstance().GetVulkanImgui()->Resize();
			}
		}

		m_CurrentBufferIndex = (m_CurrentBufferIndex + 1) % MAX_FRAMES_IN_FLIGHT; //Go to next frame
	}

	void SwapChain::CreateSwapChain(uint32_t Width, uint32_t Height)
	{
		m_Width = Width;
		m_Height = Height;

		VkSwapchainKHR OldSwapChain = m_SwapChain;

		auto PhysicalDevice = m_PhysicalDevice->GetPhysicalDevice();

		SwapChainDetails Details;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(PhysicalDevice, m_WindowSurface, &Details.Capabilities);

		uint32_t FormatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(PhysicalDevice, m_WindowSurface, &FormatCount, nullptr);
		std::vector<VkSurfaceFormatKHR> SurfaceFormats(FormatCount);

		if (FormatCount == 0)
		{
			TRE_CORE_ERROR("No formats available for surface");
		}
		vkGetPhysicalDeviceSurfaceFormatsKHR(PhysicalDevice, m_WindowSurface, &FormatCount, SurfaceFormats.data());

		uint32_t PresentCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(PhysicalDevice, m_WindowSurface, &PresentCount, nullptr);
		std::vector<VkPresentModeKHR> PresentModes(PresentCount);

		if (PresentCount == 0)
		{
			TRE_CORE_ERROR("No present mode available for surface");
			assert(false);
		}
		vkGetPhysicalDeviceSurfacePresentModesKHR(PhysicalDevice, m_WindowSurface, &PresentCount, PresentModes.data());

		VkSurfaceFormatKHR surfaceformat{};
		for (const auto& format : SurfaceFormats)
		{
			if (format.format == VK_FORMAT_B8G8R8A8_UNORM && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			{
				m_SwapChainSettings.m_ColorSpace= format.colorSpace;
				m_SwapChainSettings.m_SurfaceFormat = format.format;
				break;
			}
		}

		VkPresentModeKHR PresentModeInfo = VK_PRESENT_MODE_FIFO_KHR; //Guaranteed to have
		for (const auto& PresentMode : PresentModes)
		{
			if (PresentMode == VK_PRESENT_MODE_MAILBOX_KHR) //Something like Triple buffering, avoid tearing, render images as newest as possible, but might use more energy
			{
				PresentModeInfo = VK_PRESENT_MODE_MAILBOX_KHR; //We want this if have
			}
		}

		if (PresentModeInfo == VK_PRESENT_MODE_FIFO_KHR)
		{
			for (const auto& PresentMode : PresentModes)
			{
				if (PresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR)
				{
					PresentModeInfo = VK_PRESENT_MODE_IMMEDIATE_KHR; //Back up if no mailbox
				}
			}
			
		}
		
		m_Extent = { Details.Capabilities.currentExtent.width, Details.Capabilities.currentExtent.height };
		m_Width = m_Extent.width;
		m_Height = m_Extent.height;
		m_SwapChainSettings.m_DepthFormat = m_PhysicalDevice->GetDepthFormat();

		uint32_t ImageCount = Details.Capabilities.minImageCount + 1; //Number of images in swapchain
		if (Details.Capabilities.maxImageCount > 0 && ImageCount > Details.Capabilities.maxImageCount) //Don't go over max
		{
			ImageCount = Details.Capabilities.maxImageCount;
		}
		
		VkSurfaceTransformFlagBitsKHR TransformFlag{};
		if (Details.Capabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
		{
			TransformFlag = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
		}
		else
		{
			TransformFlag = Details.Capabilities.currentTransform;
		}

		VkCompositeAlphaFlagBitsKHR compositeAlphaFlag = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		if ((Details.Capabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR) == 0)
		{
			TRE_CORE_INFO("Default Composite Alpha Flag not available");
			std::vector<VkCompositeAlphaFlagBitsKHR> AllFlags
			{
				VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
					VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
					VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR
			};
			for (auto& Flag : AllFlags)
			{
				if (Details.Capabilities.supportedCompositeAlpha & Flag)
				{
					compositeAlphaFlag = Flag;
					break;
				}
			}
		}

		VkSwapchainCreateInfoKHR CreateInfo{};
		CreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		CreateInfo.surface = m_WindowSurface;
		CreateInfo.minImageCount = ImageCount;
		CreateInfo.imageFormat = m_SwapChainSettings.m_SurfaceFormat;
		CreateInfo.imageColorSpace = m_SwapChainSettings.m_ColorSpace;
		CreateInfo.imageExtent = m_Extent;
		CreateInfo.imageArrayLayers = 1;
		CreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		CreateInfo.oldSwapchain = OldSwapChain;
		CreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		CreateInfo.queueFamilyIndexCount = 0;
		CreateInfo.pQueueFamilyIndices = nullptr;
		CreateInfo.preTransform = TransformFlag;
		CreateInfo.compositeAlpha = compositeAlphaFlag;
		CreateInfo.presentMode = PresentModeInfo;
		CreateInfo.clipped = VK_TRUE;

		if (Details.Capabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT)
			CreateInfo.imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

		if (Details.Capabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT)
			CreateInfo.imageUsage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;

		if (auto Result = vkCreateSwapchainKHR(m_LogicalDevice->GetLogicalDevice(), &CreateInfo, nullptr, &m_SwapChain); Result != VK_SUCCESS)
		{
			assert(Result == VK_SUCCESS);
		}

		vkDestroySwapchainKHR(m_LogicalDevice->GetLogicalDevice(), OldSwapChain, nullptr);

		uint32_t ImageCounter;
		vkGetSwapchainImagesKHR(m_LogicalDevice->GetLogicalDevice(), m_SwapChain, &ImageCounter, nullptr);
		m_VulkanImages.resize(ImageCounter);
		vkGetSwapchainImagesKHR(m_LogicalDevice->GetLogicalDevice(), m_SwapChain, &ImageCounter, m_VulkanImages.data());

		m_ImageCount = ImageCounter;
	}

	void SwapChain::CreateImageViews()
	{
		m_SwapChainImages.resize(m_VulkanImages.size());

		for (size_t i = 0; i < m_SwapChainImages.size(); i++)
		{
			m_SwapChainImages[i].Image = m_VulkanImages[i];

			VkImageViewCreateInfo viewInfo{};
			viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			viewInfo.image = m_SwapChainImages[i].Image;
			viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			viewInfo.format = m_SwapChainSettings.m_SurfaceFormat;
			viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			viewInfo.subresourceRange.baseMipLevel = 0;
			viewInfo.subresourceRange.levelCount = 1;
			viewInfo.subresourceRange.baseArrayLayer = 0;
			viewInfo.subresourceRange.layerCount = 1;

			if (vkCreateImageView(m_LogicalDevice->GetLogicalDevice(), &viewInfo, nullptr, &m_SwapChainImages[i].ImageView) != VK_SUCCESS) {
				assert(false);
			}
		}
	}

	void SwapChain::CreateRenderPass()
	{
		RenderPassInfo Info{};
		Info.FinalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		Info.ImageFormat = m_SwapChainSettings.m_SurfaceFormat;
		Info.DepthEnabled = false;
		m_Renderpass = std::make_shared<RenderPass>(m_LogicalDevice, Info);
	}

	void SwapChain::CreateCommandPool()
	{
		VkCommandPoolCreateInfo CommandPoolCreateInfo{};
		CommandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		CommandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		CommandPoolCreateInfo.queueFamilyIndex = m_PhysicalDevice->GetQueueFamilies().Graphics;

		if (vkCreateCommandPool(m_LogicalDevice->GetLogicalDevice(), &CommandPoolCreateInfo, nullptr, &m_CommandPool) != VK_SUCCESS)
		{
			assert(false);
		}
	}

	void SwapChain::CreateFrameBuffer()
	{
		m_SwapChainFramebuffers.resize(m_SwapChainImages.size());
		for (size_t i = 0; i < m_SwapChainImages.size(); i++)
		{
			std::array<VkImageView, 1> Attachments = { m_SwapChainImages[i].ImageView };

			VkFramebufferCreateInfo FramebufferCreateInfo{};
			FramebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			FramebufferCreateInfo.renderPass = m_Renderpass->GetHandle();
			FramebufferCreateInfo.attachmentCount = static_cast<uint32_t>(Attachments.size());
			FramebufferCreateInfo.pAttachments = Attachments.data();
			FramebufferCreateInfo.width = m_Extent.width;
			FramebufferCreateInfo.height = m_Extent.height;
			FramebufferCreateInfo.layers = 1;

			if (vkCreateFramebuffer(m_LogicalDevice->GetLogicalDevice(), &FramebufferCreateInfo, nullptr, &m_SwapChainFramebuffers[i]) != VK_SUCCESS)
			{
				assert(false);
			}
		}
	}

	void SwapChain::CreateCommandbuffer()
	{
		m_Commandbuffers.resize(MAX_FRAMES_IN_FLIGHT);

		VkCommandBufferAllocateInfo CommandBufferInfo{};
		CommandBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		CommandBufferInfo.commandPool = m_CommandPool;
		CommandBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		CommandBufferInfo.commandBufferCount = static_cast<uint32_t>(m_Commandbuffers.size());

		if (vkAllocateCommandBuffers(m_LogicalDevice->GetLogicalDevice(), &CommandBufferInfo, m_Commandbuffers.data()) != VK_SUCCESS)
		{
			assert(false);
		}
	}

	void SwapChain::CreateSyncObjects()
	{
		m_WaitFences.resize(MAX_FRAMES_IN_FLIGHT);
		m_Semaphores.resize(MAX_FRAMES_IN_FLIGHT);

		VkSemaphoreCreateInfo SemaphoreCreateInfo{};
		SemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo FenceCreateInfo{};
		FenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		FenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			if (vkCreateSemaphore(m_LogicalDevice->GetLogicalDevice(), &SemaphoreCreateInfo, nullptr, &m_Semaphores[i].PresentComplete) != VK_SUCCESS)
			{
				assert(false);
			}

			if (vkCreateSemaphore(m_LogicalDevice->GetLogicalDevice(), &SemaphoreCreateInfo, nullptr, &m_Semaphores[i].RenderComplete) != VK_SUCCESS)
			{
				assert(false);
			}

			if (vkCreateFence(m_LogicalDevice->GetLogicalDevice(), &FenceCreateInfo, nullptr, &m_WaitFences[i]) != VK_SUCCESS)
			{
				assert(false);
			}
		}
	}

	void SwapChain::RecreateSwapChain()
	{
		int width = 0, height = 0;
		glfwGetFramebufferSize(Engine::GetInstance().GetWindow()->GetWindowHandle(), &width, &height);
		while (width == 0 || height == 0)
		{
			glfwGetFramebufferSize(Engine::GetInstance().GetWindow()->GetWindowHandle(), &width, &height);
			glfwWaitEvents();
		}

		vkDeviceWaitIdle(m_LogicalDevice->GetLogicalDevice());

		CleanSwapChain();

		CreateSwapChain(width, height);
		CreateImageViews();
		CreateFrameBuffer();
	}

	void SwapChain::CleanSwapChain()
	{
		for (auto fb : m_SwapChainFramebuffers)
		{
			vkDestroyFramebuffer(m_LogicalDevice->GetLogicalDevice(), fb, nullptr);
		}
		for (auto imageview : m_SwapChainImages)
		{
			vkDestroyImageView(m_LogicalDevice->GetLogicalDevice(), imageview.ImageView, nullptr);
		}

		m_SwapChainImages.clear();
		m_SwapChainFramebuffers.clear();
	}
}