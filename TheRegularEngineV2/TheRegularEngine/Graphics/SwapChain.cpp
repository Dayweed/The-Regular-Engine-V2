#include "pch.h"
#include "TREIncludes.h"
#include "GLFW/glfw3.h"

namespace TRE
{
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
		assert(m_CurrentImageIndex < m_FrameBuffers.size()); //Cannot go out of bound
		return m_FrameBuffers[m_CurrentImageIndex];
	}

	VkCommandBuffer SwapChain::GetCurrentCommandBuffer()
	{
		assert(m_CurrentBufferIndex < m_CommandBuffers.size()); //Cannot go out of bound
		return m_CommandBuffers[m_CurrentBufferIndex].CommandBuffer;
	}

	std::vector<SwapChain::SwapChainImage> SwapChain::GetCurrentSwapChainImage()
	{
		return m_SwapChainImages;
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
		return m_Semaphores.RenderComplete;
	}

	VkRenderPass SwapChain::GetRenderPass()
	{
		return m_Renderpass->GetHandle();
	}

	VkExtent2D SwapChain::GetSwapChainExtent()
	{
		return m_Extent;
	}

	void SwapChain::Initialize(VkInstance Instance, const std::shared_ptr<Device>& LogicalDevice, GLFWwindow* Handle)
	{
		m_Instance = Instance;
		m_LogicalDevice = LogicalDevice;
		auto PhysicalDevice = m_LogicalDevice->GetPhysicalDevice()->GetPhysicalDevice();
		glfwCreateWindowSurface(m_Instance, Handle, nullptr, &m_WindowSurface);

		uint32_t NumberofQueues; 
		vkGetPhysicalDeviceQueueFamilyProperties(PhysicalDevice, &NumberofQueues, nullptr);
		assert(NumberofQueues > 0);

		std::vector<VkQueueFamilyProperties> QueueProperties(NumberofQueues);
		vkGetPhysicalDeviceQueueFamilyProperties(PhysicalDevice, &NumberofQueues, QueueProperties.data());

		std::vector<VkBool32> SupportsPresent(NumberofQueues);
		for (uint32_t x = 0; x < NumberofQueues; x++)
		{
			vkGetPhysicalDeviceSurfaceSupportKHR(PhysicalDevice, x, m_WindowSurface, &SupportsPresent[x]);
		}

		uint32_t PresentQueueIndex = UINT32_MAX;
		uint32_t GraphicsQueueIndex = UINT32_MAX;

		for (uint32_t x = 0; x < NumberofQueues; x++)
		{
			if ((QueueProperties[x].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0)
			{
				if (GraphicsQueueIndex == UINT32_MAX)
					GraphicsQueueIndex = x;

				if (SupportsPresent[x] == VK_TRUE)
				{
					GraphicsQueueIndex = x;
					PresentQueueIndex = x;
					break;
				}
			}
		}

		//In case present is not found
		if (PresentQueueIndex == UINT32_MAX)
		{
			for (uint32_t x = 0; x < NumberofQueues; x++)
			{
				if (SupportsPresent[x] == VK_TRUE)
				{
					PresentQueueIndex = x;
					break;
				}
			}
		}

		assert(PresentQueueIndex != UINT32_MAX);
		assert(GraphicsQueueIndex != UINT32_MAX);

		m_QueueIndex = GraphicsQueueIndex;

		FindImageFormatAndColorSpace();

		m_SwapChainSettings.m_DepthFormat = m_LogicalDevice->GetPhysicalDevice()->GetDepthFormat();
	}

	void SwapChain::CreateSwapChain(uint32_t* width, uint32_t* height, bool Vsync)
	{
		VkSwapchainKHR CurrentSwapChain = m_SwapChain;
		auto PhysicalDevice = m_LogicalDevice->GetPhysicalDevice()->GetPhysicalDevice();

		VkSurfaceCapabilitiesKHR SurfaceCapabilities;
		if (VkResult Result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(PhysicalDevice, m_WindowSurface, &SurfaceCapabilities); Result != VK_SUCCESS)
			std::cout << "Unable to get surface capabilities" << std::endl;

		uint32_t NumberofPresentMode;
		vkGetPhysicalDeviceSurfacePresentModesKHR(PhysicalDevice, m_WindowSurface, &NumberofPresentMode, nullptr);
		assert(NumberofPresentMode > 0); //Cannot be 0 else cannot draw image

		std::vector<VkPresentModeKHR> PresentModes(NumberofPresentMode);
		vkGetPhysicalDeviceSurfacePresentModesKHR(PhysicalDevice, m_WindowSurface, &NumberofPresentMode, PresentModes.data());

		VkExtent2D SwapChainExtent{};
		if (SurfaceCapabilities.currentExtent.width == (uint32_t)-1) //Special case
		{
			SwapChainExtent.width = *width;
			SwapChainExtent.height = *height;
		}
		else
		{
			SwapChainExtent = SurfaceCapabilities.currentExtent;
			*width = SurfaceCapabilities.currentExtent.width;
			*height = SurfaceCapabilities.currentExtent.height;
		}

		m_Width = *width;
		m_Height = *height;
		m_Extent = SwapChainExtent;

		VkPresentModeKHR SwapChainPresentMode = VK_PRESENT_MODE_FIFO_KHR; //Guaranteed to have and used if vsync is on
		if (!Vsync)
		{
			for (const auto& Mode : PresentModes)
			{
				if (Mode == VK_PRESENT_MODE_MAILBOX_KHR)
				{
					SwapChainPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
					break;
				}
				if ((SwapChainPresentMode != VK_PRESENT_MODE_MAILBOX_KHR) && (Mode == VK_PRESENT_MODE_IMMEDIATE_KHR))
					SwapChainPresentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
			}
		}

		uint32_t ImageCount = SurfaceCapabilities.minImageCount + 1; //Recommended to be min + 1
		if ((SurfaceCapabilities.maxImageCount > 0) && (ImageCount > SurfaceCapabilities.maxImageCount)) //Make sure it does not go above cap. If 0 means no cap (special vulkan val)
			ImageCount = SurfaceCapabilities.maxImageCount;

		//Ensure no pre transform by vulkan whenever possible
		VkSurfaceTransformFlagsKHR TransformFlag;
		if (SurfaceCapabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
		{
			TransformFlag = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
		}
		else
		{
			TransformFlag = SurfaceCapabilities.currentTransform;
		}

		//Almost always want to ignore hence default to this, if not supported then just use whichever is first
		VkCompositeAlphaFlagBitsKHR compositeAlphaFlag = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		if ((SurfaceCapabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR) == 0)
		{
			std::cout << "Default Composite Alpha Flag not available" << std::endl;
			std::vector<VkCompositeAlphaFlagBitsKHR> AllFlags
			{
				VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
					VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
					VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR
			};
			for (auto& Flag : AllFlags)
			{
				if (SurfaceCapabilities.supportedCompositeAlpha & Flag)
				{
					compositeAlphaFlag = Flag;
					break;
				}
			}
		}

		VkSwapchainCreateInfoKHR SwapChainCreateInfo{};
		SwapChainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		SwapChainCreateInfo.surface = m_WindowSurface;
		SwapChainCreateInfo.minImageCount = ImageCount;
		SwapChainCreateInfo.imageFormat = m_SwapChainSettings.m_SurfaceFormat;
		SwapChainCreateInfo.imageColorSpace = m_SwapChainSettings.m_ColorSpace;
		SwapChainCreateInfo.imageExtent = SwapChainExtent;
		SwapChainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		SwapChainCreateInfo.preTransform = static_cast<VkSurfaceTransformFlagBitsKHR>(TransformFlag);
		SwapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE; //Best performance, image owned by 1 queue at a time only, needs to be explictly transferred
		SwapChainCreateInfo.queueFamilyIndexCount = 0; //No need to specify which queues are sharing cause we are using exclusive ownership
		SwapChainCreateInfo.pQueueFamilyIndices = nullptr; //^
		SwapChainCreateInfo.imageArrayLayers = 1;
		SwapChainCreateInfo.presentMode = SwapChainPresentMode;
		SwapChainCreateInfo.oldSwapchain = CurrentSwapChain;
		SwapChainCreateInfo.clipped = VK_TRUE;
		SwapChainCreateInfo.compositeAlpha = compositeAlphaFlag;

		//2 usage below are used to render off screen and transfer images in case of post-processing etc
		if (SurfaceCapabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT)
		{
			SwapChainCreateInfo.imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		}
		if (SurfaceCapabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT)
		{
			SwapChainCreateInfo.imageUsage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		}

		if (VkResult Result = vkCreateSwapchainKHR(m_LogicalDevice->GetLogicalDevice(), &SwapChainCreateInfo, nullptr, &m_SwapChain); Result != VK_SUCCESS)
		{
			std::cout << "Unable to create swap chain" << std::endl;
			assert(Result == VK_SUCCESS);
		}

		if (CurrentSwapChain)
			vkDestroySwapchainKHR(m_LogicalDevice->GetLogicalDevice(), CurrentSwapChain, nullptr);

		for (auto& image : m_SwapChainImages)
		{
			vkDestroyImageView(m_LogicalDevice->GetLogicalDevice(), image.ImageView, nullptr);
			vkDestroyImageView(m_LogicalDevice->GetLogicalDevice(), image.DepthImageView, nullptr);
		}
		m_SwapChainImages.clear();

		if (VkResult Result = vkGetSwapchainImagesKHR(m_LogicalDevice->GetLogicalDevice(), m_SwapChain, &m_ImageCount, nullptr); Result != VK_SUCCESS)
		{
			std::cout << "Unable to get number of swapchain images" << std::endl;
			assert(Result == VK_SUCCESS);
		}

		m_SwapChainImages.resize(m_ImageCount);
		m_VulkanImages.resize(m_ImageCount);
		m_DepthImages.resize(m_ImageCount);
		m_DepthMemory.resize(m_ImageCount);
		
		if (VkResult Result = vkGetSwapchainImagesKHR(m_LogicalDevice->GetLogicalDevice(), m_SwapChain, &m_ImageCount, m_VulkanImages.data()); Result != VK_SUCCESS)
		{
			std::cout << "Unable to get swapchain images" << std::endl;
			assert(Result == VK_SUCCESS);
		}

		m_SwapChainImages.resize(m_ImageCount);

		for (uint32_t x = 0; x < m_ImageCount; x++)
		{
			VkImageViewCreateInfo ImageViewCreateInfo{};
			ImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			ImageViewCreateInfo.image = m_VulkanImages[x];
			ImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			ImageViewCreateInfo.format = m_SwapChainSettings.m_SurfaceFormat;
			ImageViewCreateInfo.components =
			{
				VK_COMPONENT_SWIZZLE_R,
				VK_COMPONENT_SWIZZLE_G,
				VK_COMPONENT_SWIZZLE_B,
				VK_COMPONENT_SWIZZLE_A
			};
			ImageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			ImageViewCreateInfo.subresourceRange.baseMipLevel = 0;
			ImageViewCreateInfo.subresourceRange.levelCount = 1;
			ImageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
			ImageViewCreateInfo.subresourceRange.layerCount = 1;

			m_SwapChainImages[x].Image = m_VulkanImages[x];

			if (VkResult Result = vkCreateImageView(m_LogicalDevice->GetLogicalDevice(), &ImageViewCreateInfo, nullptr, &m_SwapChainImages[x].ImageView); Result != VK_SUCCESS)
			{
				std::cout << "Unable to create image view for swap chain" << std::endl;
				assert(Result == VK_SUCCESS);
			}
		}

		//Depth Image
		for (uint32_t x = 0; x < ImageCount; ++x)
		{
			VkImageCreateInfo image{};
			image.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			image.imageType = VK_IMAGE_TYPE_2D;
			image.format = m_SwapChainSettings.m_DepthFormat;
			image.extent.width = m_Extent.width;
			image.extent.height = m_Extent.height;
			image.extent.depth = 1;
			image.mipLevels = 1;
			image.arrayLayers = 1;
			image.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			image.samples = VK_SAMPLE_COUNT_1_BIT;
			image.tiling = VK_IMAGE_TILING_OPTIMAL;
			image.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
			if (VkResult Result = vkCreateImage(m_LogicalDevice->GetLogicalDevice(), &image, nullptr, &m_DepthImages[x]); Result != VK_SUCCESS)
			{
				std::cout << "Unable to create depth image for swap chain" << std::endl;
				assert(Result == VK_SUCCESS);
			}
			m_SwapChainImages[x].DepthImage = m_DepthImages[x];

			VkMemoryRequirements memReqs;
			VkMemoryAllocateInfo memAlloc{};
			memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			vkGetImageMemoryRequirements(m_LogicalDevice->GetLogicalDevice(), m_DepthImages[x], &memReqs);
			memAlloc.allocationSize = memReqs.size;
			memAlloc.memoryTypeIndex = m_LogicalDevice->FindMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
			vkAllocateMemory(m_LogicalDevice->GetLogicalDevice(), &memAlloc, nullptr, &m_DepthMemory[x]);
			vkBindImageMemory(m_LogicalDevice->GetLogicalDevice(), m_DepthImages[x], m_DepthMemory[x], 0);

			VkImageViewCreateInfo depthStencilView{};
			depthStencilView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			depthStencilView.viewType = VK_IMAGE_VIEW_TYPE_2D;
			depthStencilView.format = m_SwapChainSettings.m_DepthFormat;
			depthStencilView.flags = 0;
			depthStencilView.subresourceRange = {};
			depthStencilView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
			depthStencilView.subresourceRange.baseMipLevel = 0;
			depthStencilView.subresourceRange.levelCount = 1;
			depthStencilView.subresourceRange.baseArrayLayer = 0;
			depthStencilView.subresourceRange.layerCount = 1;
			depthStencilView.image = m_DepthImages[x];
			vkCreateImageView(m_LogicalDevice->GetLogicalDevice(), &depthStencilView, nullptr, &m_SwapChainImages[x].DepthImageView);
		}

		//Command Buffers
		for (auto& CommandBuffer : m_CommandBuffers)
			vkDestroyCommandPool(m_LogicalDevice->GetLogicalDevice(), CommandBuffer.CommandPool, nullptr);

		VkCommandPoolCreateInfo CommandPoolCreateInfo{};
		CommandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		CommandPoolCreateInfo.queueFamilyIndex = m_QueueIndex;
		CommandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;

		VkCommandBufferAllocateInfo CommandBufferAllocateInfo{};
		CommandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		CommandBufferAllocateInfo.commandBufferCount = 1;
		CommandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

		m_CommandBuffers.resize(m_ImageCount);
		for (auto& Command : m_CommandBuffers)
		{
			if (VkResult Result = vkCreateCommandPool(m_LogicalDevice->GetLogicalDevice(), &CommandPoolCreateInfo, nullptr, &Command.CommandPool); Result != VK_SUCCESS)
			{
				std::cout << "Unable to create a command pool" << std::endl;
				assert(Result == VK_SUCCESS);
			}

			CommandBufferAllocateInfo.commandPool = Command.CommandPool;
			if (VkResult Result = vkAllocateCommandBuffers(m_LogicalDevice->GetLogicalDevice(), &CommandBufferAllocateInfo, &Command.CommandBuffer); Result != VK_SUCCESS)
			{
				std::cout << "Unable to create a command buffer" << std::endl;
				assert(Result == VK_SUCCESS);
			}
		}
		
		//Synchronization
		if (!m_Semaphores.RenderComplete || !m_Semaphores.PresentComplete)
		{
			VkSemaphoreCreateInfo SemaphoreCreateInfo{};
			SemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
			if (auto Result = vkCreateSemaphore(m_LogicalDevice->GetLogicalDevice(), &SemaphoreCreateInfo, nullptr, &m_Semaphores.RenderComplete); Result != VK_SUCCESS)
			{
				std::cout << "Unable to create semaphore" << std::endl;
				assert(Result == VK_SUCCESS);
			}
			if (auto Result = vkCreateSemaphore(m_LogicalDevice->GetLogicalDevice(), &SemaphoreCreateInfo, nullptr, &m_Semaphores.PresentComplete); Result != VK_SUCCESS)
			{
				std::cout << "Unable to create semaphore" << std::endl;
				assert(Result == VK_SUCCESS);
			}
		}

		if (m_WaitFences.size() != m_ImageCount)
		{
			VkFenceCreateInfo FenceCreateInfo{};
			FenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			FenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
			m_WaitFences.resize(m_ImageCount);

			for (auto& Fence : m_WaitFences)
			{
				if (auto Result = vkCreateFence(m_LogicalDevice->GetLogicalDevice(), &FenceCreateInfo, nullptr, &Fence); Result != VK_SUCCESS)
				{
					std::cout << "Unable to create fence" << std::endl;
					assert(Result == VK_SUCCESS);
				}
			}
		}

		//RenderPass
		RenderPassInfo RenderPassCreateInfo{};
		RenderPassCreateInfo.ImageFormat = m_SwapChainSettings.m_SurfaceFormat;
		RenderPassCreateInfo.FinalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		RenderPassCreateInfo.DepthImageFormat = m_SwapChainSettings.m_DepthFormat;
		RenderPassCreateInfo.DepthFinalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		m_Renderpass = std::make_shared<RenderPass>(m_LogicalDevice, RenderPassCreateInfo);

		//FrameBuffers
		for (auto& framebuffer : m_FrameBuffers)
		{
			vkDestroyFramebuffer(m_LogicalDevice->GetLogicalDevice(), framebuffer, nullptr);
		}

		VkFramebufferCreateInfo FrameBufferCreateInfo{};
		FrameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		FrameBufferCreateInfo.renderPass = m_Renderpass->GetHandle();
		FrameBufferCreateInfo.attachmentCount = 2;
		FrameBufferCreateInfo.width = m_Width;
		FrameBufferCreateInfo.height = m_Height;
		FrameBufferCreateInfo.layers = 1;
		
		m_FrameBuffers.resize(m_ImageCount);

		for (int x = 0; x < m_FrameBuffers.size(); x++)
		{
			std::array<VkImageView, 2> Attachments = { m_SwapChainImages[x].ImageView, m_SwapChainImages[x].DepthImageView };
			FrameBufferCreateInfo.pAttachments = Attachments.data();
			if (auto Result = vkCreateFramebuffer(m_LogicalDevice->GetLogicalDevice(), &FrameBufferCreateInfo, nullptr, &m_FrameBuffers[x]); Result != VK_SUCCESS)
			{
				std::cout << "Unable to create framebuffer" << std::endl;
				assert(Result == VK_SUCCESS);
			}
		}
	}

	void SwapChain::DestroySwapChain()
	{
		vkDeviceWaitIdle(m_LogicalDevice->GetLogicalDevice());

		if (m_SwapChain)
			vkDestroySwapchainKHR(m_LogicalDevice->GetLogicalDevice(), m_SwapChain, nullptr);

		if (m_WindowSurface)
			vkDestroySurfaceKHR(m_Instance, m_WindowSurface, nullptr);

		for (auto& image : m_SwapChainImages)
		{
			vkDestroyImageView(m_LogicalDevice->GetLogicalDevice(), image.ImageView, nullptr);
			vkDestroyImageView(m_LogicalDevice->GetLogicalDevice(), image.DepthImageView, nullptr);
		}

		for (auto& CommandBuffer : m_CommandBuffers)
			vkDestroyCommandPool(m_LogicalDevice->GetLogicalDevice(), CommandBuffer.CommandPool, nullptr);

		for (auto& framebuffer : m_FrameBuffers)
		{
			vkDestroyFramebuffer(m_LogicalDevice->GetLogicalDevice(), framebuffer, nullptr);
		}

		if (m_Semaphores.RenderComplete)
			vkDestroySemaphore(m_LogicalDevice->GetLogicalDevice(), m_Semaphores.RenderComplete, nullptr);

		if (m_Semaphores.PresentComplete)
			vkDestroySemaphore(m_LogicalDevice->GetLogicalDevice(), m_Semaphores.PresentComplete, nullptr);

		for (auto& fence : m_WaitFences)
			vkDestroyFence(m_LogicalDevice->GetLogicalDevice(), fence, nullptr);

		vkDeviceWaitIdle(m_LogicalDevice->GetLogicalDevice());
	}

	void SwapChain::BeginFrame()
	{
		vkWaitForFences(m_LogicalDevice->GetLogicalDevice(), 1, &m_WaitFences[m_CurrentBufferIndex], VK_TRUE, UINT64_MAX);
		vkResetFences(m_LogicalDevice->GetLogicalDevice(), 1, &m_WaitFences[m_CurrentBufferIndex]);

		m_CurrentImageIndex = AccuireNextImage();
		if (auto result = vkResetCommandPool(m_LogicalDevice->GetLogicalDevice(), m_CommandBuffers[m_CurrentBufferIndex].CommandPool, 0); result != VK_SUCCESS)
		{
			std::cout << "Unable to reset pool" << std::endl;
			assert(result == VK_SUCCESS);
		}
	}

	void SwapChain::Present()
	{
		VkPipelineStageFlags PipelineStageFlags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		VkSubmitInfo SubmitInfo{};
		SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		SubmitInfo.pWaitDstStageMask = &PipelineStageFlags;
		SubmitInfo.waitSemaphoreCount = 1;
		SubmitInfo.pWaitSemaphores = &m_Semaphores.PresentComplete;
		SubmitInfo.signalSemaphoreCount = 1;
		SubmitInfo.pSignalSemaphores = &m_Semaphores.RenderComplete;
		SubmitInfo.commandBufferCount = 1;
		SubmitInfo.pCommandBuffers = &m_CommandBuffers[m_CurrentBufferIndex].CommandBuffer;

		if (auto Result = vkResetFences(m_LogicalDevice->GetLogicalDevice(), 1, &m_WaitFences[m_CurrentBufferIndex]); Result != VK_SUCCESS)
		{
			std::cout << "Unable to reset fences" << std::endl;
			assert(Result == VK_SUCCESS);
		}

		if (auto Result = vkQueueSubmit(m_LogicalDevice->GetGraphicsQ(), 1, &SubmitInfo, m_WaitFences[m_CurrentBufferIndex]); Result != VK_SUCCESS)
		{
			std::cout << "Unable to queue submit" << std::endl;
			assert(Result == VK_SUCCESS);
		}

		VkPresentInfoKHR PresentInfo{};
		PresentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		PresentInfo.swapchainCount = 1;
		PresentInfo.pSwapchains = &m_SwapChain;
		PresentInfo.pImageIndices = &m_CurrentImageIndex;
		PresentInfo.pWaitSemaphores = &m_Semaphores.RenderComplete;
		PresentInfo.waitSemaphoreCount = 1;

		if (auto Result = vkQueuePresentKHR(m_LogicalDevice->GetGraphicsQ(), &PresentInfo); Result != VK_SUCCESS)
		{
			std::cout << "Resized" << std::endl;
			if (Result == VK_ERROR_OUT_OF_DATE_KHR || Result == VK_SUBOPTIMAL_KHR)
			{
				Resize(m_Width, m_Height);
				Engine::GetInstance().GetRenderer()->Resize();
				Engine::GetInstance().GetVulkanImgui()->Resize();
			}
			else
			{
				assert(Result == VK_SUCCESS);
			}
		}

		m_CurrentBufferIndex = (m_CurrentBufferIndex + 1) % MAX_FRAMES_IN_FLIGHT;
		m_CurrentImageIndex = (m_CurrentImageIndex + 1) % MAX_FRAMES_IN_FLIGHT;

		if (auto Result = vkWaitForFences(m_LogicalDevice->GetLogicalDevice(), 1, &m_WaitFences[m_CurrentBufferIndex], VK_TRUE, UINT64_MAX); Result != VK_SUCCESS)
		{
			std::cout << "Failed to wait for fence" << std::endl;
			assert(Result == VK_SUCCESS);
		}
	}

	uint32_t SwapChain::AccuireNextImage()
	{
		uint32_t Index;

		if (auto Result = vkAcquireNextImageKHR(m_LogicalDevice->GetLogicalDevice(), m_SwapChain, UINT64_MAX, m_Semaphores.PresentComplete, nullptr, &Index); Result != VK_SUCCESS)
		{
			std::cout << "Unable to get next image" << std::endl;
			assert(Result == VK_SUCCESS);
		}
		return Index;
	}

	void SwapChain::Resize(uint32_t width, uint32_t height)
	{
		vkDeviceWaitIdle(m_LogicalDevice->GetLogicalDevice());
		CreateSwapChain(&width, &height, true); //Change later
		vkDeviceWaitIdle(m_LogicalDevice->GetLogicalDevice());
	}

	void SwapChain::FindImageFormatAndColorSpace()
	{
		auto PhysicalDevice = m_LogicalDevice->GetPhysicalDevice()->GetPhysicalDevice();
		uint32_t FormatCount;
		if (VkResult Result = vkGetPhysicalDeviceSurfaceFormatsKHR(PhysicalDevice, m_WindowSurface, &FormatCount, nullptr); Result != VK_SUCCESS)
		{
			std::cout << "Surface Format bad result at " << __FILE__ << " and line " << __LINE__ << std::endl;
			assert(Result == VK_SUCCESS);
		}
		
		assert(FormatCount > 0);

		std::vector<VkSurfaceFormatKHR> SurfaceFormats(FormatCount);
		if (VkResult Result = vkGetPhysicalDeviceSurfaceFormatsKHR(PhysicalDevice, m_WindowSurface, &FormatCount, SurfaceFormats.data()); Result != VK_SUCCESS)
		{
			std::cout << "Unable to get Surface Format at " << __FILE__ << " and line " << __LINE__ << std::endl;
			assert(Result == VK_SUCCESS);
		}

		if ((FormatCount == 1) && (SurfaceFormats[0].format == VK_FORMAT_UNDEFINED))
		{
			m_SwapChainSettings.m_ColorSpace = SurfaceFormats[0].colorSpace;
			m_SwapChainSettings.m_SurfaceFormat = VK_FORMAT_B8G8R8A8_UNORM;
		}
		else
		{
			bool FormatFound = false;
			for (const auto& Format : SurfaceFormats)
			{
				if (Format.format == VK_FORMAT_B8G8R8A8_UNORM)
				{
					m_SwapChainSettings.m_ColorSpace = Format.colorSpace;
					m_SwapChainSettings.m_SurfaceFormat = Format.format;
					FormatFound = true;
					break;
				}
			}
			
			if (!FormatFound)
			{
				m_SwapChainSettings.m_ColorSpace = SurfaceFormats[0].colorSpace;
				m_SwapChainSettings.m_SurfaceFormat = SurfaceFormats[0].format;
			}
		}
	}
}