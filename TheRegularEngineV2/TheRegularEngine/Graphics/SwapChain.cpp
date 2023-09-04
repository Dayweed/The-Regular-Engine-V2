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
		//assert(m_CurrentImageIndex < m_FrameBuffers.size()); //Cannot go out of bound
		//return m_FrameBuffers[m_CurrentImageIndex];
		return nullptr;
	}

	VkCommandBuffer SwapChain::GetCurrentCommandBuffer()
	{
		//assert(m_CurrentBufferIndex < m_Commandbufferss.size()); //Cannot go out of bound
		//return m_Commandbufferss[m_CurrentBufferIndex];
		return nullptr;
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
		//return m_Renderpass->GetHandle();
		return m_RenderPass;
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
		vkDestroyRenderPass(m_LogicalDevice->GetLogicalDevice(), m_RenderPass, nullptr);
		CleanSwapChain();
		vkDestroySwapchainKHR(m_LogicalDevice->GetLogicalDevice(), m_SwapChain, nullptr);
		vkDestroyCommandPool(m_LogicalDevice->GetLogicalDevice(), m_CommandPool, nullptr);
		vkDestroySurfaceKHR(RendererContext::GetVKInstance(), m_WindowSurface, nullptr);
	}

	void SwapChain::Initialize(uint32_t Width, uint32_t Height)
	{
		//auto PhysicalDevice = m_LogicalDevice->GetLogicalDevice()->GetPhysicalDevice()->GetPhysicalDevice();

		//uint32_t NumberofQueues; 
		//vkGetPhysicalDeviceQueueFamilyProperties(PhysicalDevice, &NumberofQueues, nullptr);
		//assert(NumberofQueues > 0);

		//std::vector<VkQueueFamilyProperties> QueueProperties(NumberofQueues);
		//vkGetPhysicalDeviceQueueFamilyProperties(PhysicalDevice, &NumberofQueues, QueueProperties.data());

		//std::vector<VkBool32> SupportsPresent(NumberofQueues);
		//for (uint32_t x = 0; x < NumberofQueues; x++)
		//{
		//	vkGetPhysicalDeviceSurfaceSupportKHR(PhysicalDevice, x, m_WindowSurface, &SupportsPresent[x]);
		//}

		//uint32_t PresentQueueIndex = UINT32_MAX;
		//uint32_t GraphicsQueueIndex = UINT32_MAX;

		//for (uint32_t x = 0; x < NumberofQueues; x++)
		//{
		//	if ((QueueProperties[x].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0)
		//	{
		//		if (GraphicsQueueIndex == UINT32_MAX)
		//			GraphicsQueueIndex = x;

		//		if (SupportsPresent[x] == VK_TRUE)
		//		{
		//			GraphicsQueueIndex = x;
		//			PresentQueueIndex = x;
		//			break;
		//		}
		//	}
		//}

		////In case present is not found
		//if (PresentQueueIndex == UINT32_MAX)
		//{
		//	for (uint32_t x = 0; x < NumberofQueues; x++)
		//	{
		//		if (SupportsPresent[x] == VK_TRUE)
		//		{
		//			PresentQueueIndex = x;
		//			break;
		//		}
		//	}
		//}

		//assert(PresentQueueIndex != UINT32_MAX);
		//assert(GraphicsQueueIndex != UINT32_MAX);

		//m_QueueIndex = GraphicsQueueIndex;

		CreateSwapChain(Width, Height);
		CreateImageViews();
		CreateRenderPass();
		CreateCommandPool();
		CreateFrameBuffer();
		CreateCommandbuffer();
		CreateSyncObjects();
	}

	void SwapChain::CreateSwapChain(uint32_t* width, uint32_t* height, bool Vsync)
	{
		FindImageFormatAndColorSpace();
		//m_SwapChainSettings.m_DepthFormat = m_LogicalDevice->GetLogicalDevice()->GetPhysicalDevice()->GetDepthFormat();

		//auto PhysicalDevice = m_LogicalDevice->GetLogicalDevice()->GetPhysicalDevice()->GetPhysicalDevice();

		//VkSurfaceCapabilitiesKHR SurfaceCapabilities;
		//if (VkResult Result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(PhysicalDevice, m_WindowSurface, &SurfaceCapabilities); Result != VK_SUCCESS)
		//	TRE_CORE_CRITICAL("Unable to get surface capabilities");
		//
		//uint32_t NumberofPresentMode;
		//vkGetPhysicalDeviceSurfacePresentModesKHR(PhysicalDevice, m_WindowSurface, &NumberofPresentMode, nullptr);
		//assert(NumberofPresentMode > 0); //Cannot be 0 else cannot draw image
		//
		//std::vector<VkPresentModeKHR> PresentModes(NumberofPresentMode);
		//vkGetPhysicalDeviceSurfacePresentModesKHR(PhysicalDevice, m_WindowSurface, &NumberofPresentMode, PresentModes.data());
		//
		//VkExtent2D SwapChainExtent{};
		//if (SurfaceCapabilities.currentExtent.width == (uint32_t)-1) //Special case
		//{
		//	SwapChainExtent.width = *width;
		//	SwapChainExtent.height = *height;
		//}
		//else
		//{
		//	SwapChainExtent = SurfaceCapabilities.currentExtent;
		//	*width = SurfaceCapabilities.currentExtent.width;
		//	*height = SurfaceCapabilities.currentExtent.height;
		//}
		//
		//m_Width = *width;
		//m_Height = *height;
		//m_Extent = SwapChainExtent;
		//
		//VkPresentModeKHR SwapChainPresentMode = VK_PRESENT_MODE_FIFO_KHR; //Guaranteed to have and used if vsync is on
		//if (!Vsync)
		//{
		//	for (const auto& Mode : PresentModes)
		//	{
		//		if (Mode == VK_PRESENT_MODE_MAILBOX_KHR)
		//		{
		//			SwapChainPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
		//			break;
		//		}
		//		if ((SwapChainPresentMode != VK_PRESENT_MODE_MAILBOX_KHR) && (Mode == VK_PRESENT_MODE_IMMEDIATE_KHR))
		//			SwapChainPresentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
		//	}
		//}
		//
		//uint32_t ImageCount = SurfaceCapabilities.minImageCount + 1; //Recommended to be min + 1
		//if ((SurfaceCapabilities.maxImageCount > 0) && (ImageCount > SurfaceCapabilities.maxImageCount)) //Make sure it does not go above cap. If 0 means no cap (special vulkan val)
		//	ImageCount = SurfaceCapabilities.maxImageCount;
		//
		////Ensure no pre transform by vulkan whenever possible
		//VkSurfaceTransformFlagsKHR TransformFlag;
		//if (SurfaceCapabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
		//{
		//	TransformFlag = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
		//}
		//else
		//{
		//	TransformFlag = SurfaceCapabilities.currentTransform;
		//}
		//
		////Almost always want to ignore hence default to this, if not supported then just use whichever is first
		//VkCompositeAlphaFlagBitsKHR compositeAlphaFlag = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		//if ((SurfaceCapabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR) == 0)
		//{
		//	TRE_CORE_INFO("Default Composite Alpha Flag not available");
		//	std::vector<VkCompositeAlphaFlagBitsKHR> AllFlags
		//	{
		//		VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
		//			VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
		//			VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR
		//	};
		//	for (auto& Flag : AllFlags)
		//	{
		//		if (SurfaceCapabilities.supportedCompositeAlpha & Flag)
		//		{
		//			compositeAlphaFlag = Flag;
		//			break;
		//		}
		//	}
		//}
		//
		//VkSwapchainCreateInfoKHR SwapChainCreateInfo{};
		//SwapChainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		//SwapChainCreateInfo.surface = m_WindowSurface;
		//SwapChainCreateInfo.minImageCount = ImageCount;
		//SwapChainCreateInfo.imageFormat = m_SwapChainSettings.m_SurfaceFormat;
		//SwapChainCreateInfo.imageColorSpace = m_SwapChainSettings.m_ColorSpace;
		//SwapChainCreateInfo.imageExtent = SwapChainExtent;
		//SwapChainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		//SwapChainCreateInfo.preTransform = static_cast<VkSurfaceTransformFlagBitsKHR>(TransformFlag);
		//SwapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE; //Best performance, image owned by 1 queue at a time only, needs to be explictly transferred
		//SwapChainCreateInfo.queueFamilyIndexCount = 0; //No need to specify which queues are sharing cause we are using exclusive ownership
		//SwapChainCreateInfo.pQueueFamilyIndices = nullptr; //^
		//SwapChainCreateInfo.imageArrayLayers = 1;
		//SwapChainCreateInfo.presentMode = SwapChainPresentMode;
		//SwapChainCreateInfo.oldSwapchain = VK_NULL_HANDLE;
		//SwapChainCreateInfo.clipped = VK_TRUE;
		//SwapChainCreateInfo.compositeAlpha = compositeAlphaFlag;
		//
		////2 usage below are used to render off screen and transfer images in case of post-processing etc
		//if (SurfaceCapabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT)
		//{
		//	SwapChainCreateInfo.imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		//}
		//if (SurfaceCapabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT)
		//{
		//	SwapChainCreateInfo.imageUsage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		//}
		//
		//if (VkResult Result = vkCreateSwapchainKHR(m_LogicalDevice->GetLogicalDevice()->GetLogicalDevice(), &SwapChainCreateInfo, nullptr, &m_SwapChain); Result != VK_SUCCESS)
		//{
		//	TRE_CORE_CRITICAL("Unable to create swap chain");
		//	assert(Result == VK_SUCCESS);
		//}
		//
		//for (auto& image : m_SwapChainImages)
		//{
		//	//vkDestroyImageView(m_LogicalDevice->GetLogicalDevice()->GetLogicalDevice(), image.ImageView, nullptr);
		//	//vkDestroyImageView(m_LogicalDevice->GetLogicalDevice()->GetLogicalDevice(), image.DepthImageView, nullptr);
		//}
		//m_SwapChainImages.clear();
		//m_VulkanImages.clear();
		////m_DepthImages.clear();
		////m_DepthMemory.clear();
		//
		//if (VkResult Result = vkGetSwapchainImagesKHR(m_LogicalDevice->GetLogicalDevice()->GetLogicalDevice(), m_SwapChain, &ImageCount, nullptr); Result != VK_SUCCESS)
		//{
		//	TRE_CORE_WARN("Unable to get number of swapchain images");
		//	assert(Result == VK_SUCCESS);
		//}
		//
		//m_SwapChainImages.resize(ImageCount);
		//m_VulkanImages.resize(ImageCount);
		////m_DepthImages.resize(ImageCount);
		////m_DepthMemory.resize(ImageCount);
		//
		//if (VkResult Result = vkGetSwapchainImagesKHR(m_LogicalDevice->GetLogicalDevice()->GetLogicalDevice(), m_SwapChain, &ImageCount, m_VulkanImages.data()); Result != VK_SUCCESS)
		//{
		//	TRE_CORE_WARN("Unable to get swapchain images");
		//	assert(Result == VK_SUCCESS);
		//}
		//
		//for (uint32_t x = 0; x < ImageCount; x++)
		//{
		//	VkImageViewCreateInfo ImageViewCreateInfo{};
		//	ImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		//	ImageViewCreateInfo.image = m_VulkanImages[x];
		//	ImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		//	ImageViewCreateInfo.format = m_SwapChainSettings.m_SurfaceFormat;
		//	ImageViewCreateInfo.components =
		//	{
		//		VK_COMPONENT_SWIZZLE_R,
		//		VK_COMPONENT_SWIZZLE_G,
		//		VK_COMPONENT_SWIZZLE_B,
		//		VK_COMPONENT_SWIZZLE_A
		//	};
		//	ImageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		//	ImageViewCreateInfo.subresourceRange.baseMipLevel = 0;
		//	ImageViewCreateInfo.subresourceRange.levelCount = 1;
		//	ImageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
		//	ImageViewCreateInfo.subresourceRange.layerCount = 1;
		//
		//	//m_SwapChainImages[x].Image = m_VulkanImages[x];
		//
		//	//if (VkResult Result = vkCreateImageView(m_LogicalDevice->GetLogicalDevice()->GetLogicalDevice(), &ImageViewCreateInfo, nullptr, &m_SwapChainImages[x].ImageView); Result != VK_SUCCESS)
		//	//{
		//	//	TRE_CORE_WARN("Unable to create image view for swap chain");
		//	//	assert(Result == VK_SUCCESS);
		//	//}
		//}
		//
		////Depth Image
		////for (uint32_t x = 0; x < ImageCount; ++x)
		////{
		////	VkImageCreateInfo image{};
		////	image.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		////	image.imageType = VK_IMAGE_TYPE_2D;
		////	image.format = m_SwapChainSettings.m_DepthFormat;
		////	image.extent.width = m_Extent.width;
		////	image.extent.height = m_Extent.height;
		////	image.extent.depth = 1;
		////	image.mipLevels = 1;
		////	image.arrayLayers = 1;
		////	image.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		////	image.samples = VK_SAMPLE_COUNT_1_BIT;
		////	image.tiling = VK_IMAGE_TILING_OPTIMAL;
		////	image.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		////	if (VkResult Result = vkCreateImage(m_LogicalDevice->GetLogicalDevice()->GetLogicalDevice(), &image, nullptr, &m_DepthImages[x]); Result != VK_SUCCESS)
		////	{
		////		TRE_CORE_WARN("Unable to create depth image for swap chain");
		////		assert(Result == VK_SUCCESS);
		////	}
		////	//m_SwapChainImages[x].DepthImage = m_DepthImages[x];
		//
		////	VkMemoryRequirements memReqs;
		////	VkMemoryAllocateInfo memAlloc{};
		////	memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		////	vkGetImageMemoryRequirements(m_LogicalDevice->GetLogicalDevice()->GetLogicalDevice(), m_DepthImages[x], &memReqs);
		////	memAlloc.allocationSize = memReqs.size;
		////	memAlloc.memoryTypeIndex = m_LogicalDevice->GetLogicalDevice()->FindMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		////	vkAllocateMemory(m_LogicalDevice->GetLogicalDevice()->GetLogicalDevice(), &memAlloc, nullptr, &m_DepthMemory[x]);
		////	vkBindImageMemory(m_LogicalDevice->GetLogicalDevice()->GetLogicalDevice(), m_DepthImages[x], m_DepthMemory[x], 0);
		//
		////	VkImageViewCreateInfo depthStencilView{};
		////	depthStencilView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		////	depthStencilView.viewType = VK_IMAGE_VIEW_TYPE_2D;
		////	depthStencilView.format = m_SwapChainSettings.m_DepthFormat;
		////	depthStencilView.flags = 0;
		////	depthStencilView.subresourceRange = {};
		////	depthStencilView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
		////	depthStencilView.subresourceRange.baseMipLevel = 0;
		////	depthStencilView.subresourceRange.levelCount = 1;
		////	depthStencilView.subresourceRange.baseArrayLayer = 0;
		////	depthStencilView.subresourceRange.layerCount = 1;
		////	depthStencilView.image = m_DepthImages[x];
		////	vkCreateImageView(m_LogicalDevice->GetLogicalDevice()->GetLogicalDevice(), &depthStencilView, nullptr, &m_SwapChainImages[x].DepthImageView);
		////}
		//
		////Synchronization
		////if (!m_Semaphores.RenderComplete || !m_Semaphores.PresentComplete)
		//{
		//	for (int x = 0; x < m_Semaphores.size(); x++)
		//	{
		//		vkDestroySemaphore(m_LogicalDevice->GetLogicalDevice()->GetLogicalDevice(), m_Semaphores[x].RenderComplete, nullptr);
		//		vkDestroySemaphore(m_LogicalDevice->GetLogicalDevice()->GetLogicalDevice(), m_Semaphores[x].PresentComplete, nullptr);
		//	}
		//	m_Semaphores.clear();
		//	VkSemaphoreCreateInfo SemaphoreCreateInfo{};
		//	SemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		//	m_Semaphores.resize(ImageCount);
		//	for (int x = 0; x < m_Semaphores.size(); x++)
		//	{
		//		if (auto Result = vkCreateSemaphore(m_LogicalDevice->GetLogicalDevice()->GetLogicalDevice(), &SemaphoreCreateInfo, nullptr, &m_Semaphores[x].RenderComplete); Result != VK_SUCCESS)
		//		{
		//			TRE_CORE_WARN("Unable to create semaphore");
		//			assert(Result == VK_SUCCESS);
		//		}
		//		if (auto Result = vkCreateSemaphore(m_LogicalDevice->GetLogicalDevice()->GetLogicalDevice(), &SemaphoreCreateInfo, nullptr, &m_Semaphores[x].PresentComplete); Result != VK_SUCCESS)
		//		{
		//			TRE_CORE_WARN("Unable to create semaphore");
		//			assert(Result == VK_SUCCESS);
		//		}
		//	}
		//}
		//
		////if (m_WaitFences.size() != m_ImageCount)
		////{
		//	for (auto& Fence : m_WaitFences)
		//	{
		//		vkDestroyFence(m_LogicalDevice->GetLogicalDevice()->GetLogicalDevice(), Fence, nullptr);
		//	}
		//	m_WaitFences.clear();
		//	VkFenceCreateInfo FenceCreateInfo{};
		//	FenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		//	FenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		//	m_WaitFences.resize(ImageCount);
		//
		//	for (auto& Fence : m_WaitFences)
		//	{
		//		if (auto Result = vkCreateFence(m_LogicalDevice->GetLogicalDevice()->GetLogicalDevice(), &FenceCreateInfo, nullptr, &Fence); Result != VK_SUCCESS)
		//		{
		//			TRE_CORE_WARN("Unable to create fence");
		//			assert(Result == VK_SUCCESS);
		//		}
		//	}
		////}
		//
		////RenderPass
		//RenderPassInfo RenderPassCreateInfo{};
		//RenderPassCreateInfo.ImageFormat = m_SwapChainSettings.m_SurfaceFormat;
		//RenderPassCreateInfo.FinalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		//RenderPassCreateInfo.DepthImageFormat = m_SwapChainSettings.m_DepthFormat;
		//RenderPassCreateInfo.DepthFinalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	/*	//if (!m_Renderpass)
		//	m_Renderpass = std::make_shared<RenderPass>(m_LogicalDevice->GetLogicalDevice(), RenderPassCreateInfo);
		//else
		//	m_Renderpass->Recreate();*/
		//
		////FrameBuffers
		//for (auto& framebuffer : m_FrameBuffers)
		//{
		//	if (framebuffer != VK_NULL_HANDLE)
		//		vkDestroyFramebuffer(m_LogicalDevice->GetLogicalDevice()->GetLogicalDevice(), framebuffer, nullptr);
		//}
		//
		//VkFramebufferCreateInfo FrameBufferCreateInfo{};
		//FrameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		////FrameBufferCreateInfo.renderPass = m_Renderpass->GetHandle();
		//FrameBufferCreateInfo.attachmentCount = 2;
		//FrameBufferCreateInfo.width = m_Width;
		//FrameBufferCreateInfo.height = m_Height;
		//FrameBufferCreateInfo.layers = 1;
		//
		//m_FrameBuffers.resize(ImageCount);
		//
		//for (int x = 0; x < ImageCount; x++)
		//{
		//	//std::array<VkImageView, 2> Attachments = { m_SwapChainImages[x].ImageView, m_SwapChainImages[x].DepthImageView };
		//	//FrameBufferCreateInfo.pAttachments = Attachments.data();
		//	if (auto Result = vkCreateFramebuffer(m_LogicalDevice->GetLogicalDevice(), &FrameBufferCreateInfo, nullptr, &m_FrameBuffers[x]); Result != VK_SUCCESS)
		//	{
		//		TRE_CORE_WARN("Unable to create framebuffer");
		//		assert(Result == VK_SUCCESS);
		//	}
		//}
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
		RecordCommandBuffer(m_Commandbuffers[m_CurrentBufferIndex], m_CurrentImageIndex);

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

		if (Result == VK_ERROR_OUT_OF_DATE_KHR || Result == VK_SUBOPTIMAL_KHR)
		{
			TRE_CORE_INFO("Recreate");
			RecreateSwapChain();
		}

		m_CurrentBufferIndex = (m_CurrentBufferIndex + 1) % MAX_FRAMES_IN_FLIGHT; //Go to next frame
	}

	void SwapChain::FindImageFormatAndColorSpace()
	{
		auto PhysicalDevice = m_LogicalDevice->GetPhysicalDevice()->GetPhysicalDevice();
		uint32_t FormatCount;
		if (VkResult Result = vkGetPhysicalDeviceSurfaceFormatsKHR(m_LogicalDevice->GetPhysicalDevice()->GetPhysicalDevice(), m_WindowSurface, &FormatCount, nullptr); Result != VK_SUCCESS)
		{
			TRE_CORE_WARN("Surface Format bad result at {0} and line {1}", __FILE__, __LINE__);
			assert(Result == VK_SUCCESS);
		}
		
		assert(FormatCount > 0);

		std::vector<VkSurfaceFormatKHR> SurfaceFormats(FormatCount);
		//if (VkResult Result = vkGetPhysicalDeviceSurfaceFormatsKHR(m_LogicalDevice->GetLogicalDevice()->GetPhysicalDevice()->GetPhysicalDevice(), m_WindowSurface, &FormatCount, SurfaceFormats.data()); Result != VK_SUCCESS)
		//{
		//	TRE_CORE_WARN("Unable to get Surface Format at {0} and line {1}", __FILE__, __LINE__);
		//	assert(Result == VK_SUCCESS);
		//}

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

	void SwapChain::CreateSwapChain(uint32_t Width, uint32_t Height)
	{
		m_Width = Width;
		m_Height = Height;

		VkSwapchainKHR OldSwapChain = m_SwapChain;

		SwapChainDetails Details = QuerySwapChainSupprt(m_PhysicalDevice->GetPhysicalDevice());
		VkSurfaceFormatKHR surfaceformat = ChooseSwapChainFormat(Details.Formats);

		VkPresentModeKHR PresentModeInfo = VK_PRESENT_MODE_FIFO_KHR; //Guaranteed to have
		for (const auto& PresentMode : Details.PresentModes)
		{
			if (PresentMode == VK_PRESENT_MODE_MAILBOX_KHR) //Something like Triple buffering, avoid tearing, render images as newest as possible, but might use more energy
			{
				PresentModeInfo = VK_PRESENT_MODE_MAILBOX_KHR; //We want this if have
			}
		}

		if (PresentModeInfo == VK_PRESENT_MODE_FIFO_KHR)
		{
			for (const auto& PresentMode : Details.PresentModes)
			{
				if (PresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR)
				{
					PresentModeInfo = VK_PRESENT_MODE_IMMEDIATE_KHR; //Back up if no mailbox
				}
			}
			
		}

		//VkExtent2D Extent = ChooseSwapExtent(Details.Capabilities);
		//m_Extent = Extent;
		m_Extent = { m_Width, m_Height };
		m_Format = surfaceformat.format;

		uint32_t ImageCount = Details.Capabilities.minImageCount + 1; //Number of images in swapchain
		if (Details.Capabilities.maxImageCount > 0 && ImageCount > Details.Capabilities.maxImageCount) //Don't go over max
		{
			ImageCount = Details.Capabilities.maxImageCount;
		}

		VkSwapchainCreateInfoKHR CreateInfo{};
		CreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		CreateInfo.surface = m_WindowSurface;
		CreateInfo.minImageCount = ImageCount;
		CreateInfo.imageFormat = surfaceformat.format;
		CreateInfo.imageColorSpace = surfaceformat.colorSpace;
		CreateInfo.imageExtent = m_Extent;
		CreateInfo.imageArrayLayers = 1;
		CreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		CreateInfo.oldSwapchain = OldSwapChain;
		CreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		CreateInfo.queueFamilyIndexCount = 0;
		CreateInfo.pQueueFamilyIndices = nullptr;
		CreateInfo.preTransform = Details.Capabilities.currentTransform;
		CreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		CreateInfo.presentMode = PresentModeInfo;
		CreateInfo.clipped = VK_TRUE;

		if (auto Result = vkCreateSwapchainKHR(m_LogicalDevice->GetLogicalDevice(), &CreateInfo, nullptr, &m_SwapChain); Result != VK_SUCCESS)
		{
			assert(Result == VK_SUCCESS);
		}

		vkDestroySwapchainKHR(m_LogicalDevice->GetLogicalDevice(), OldSwapChain, nullptr);

		uint32_t ImageCounter;
		vkGetSwapchainImagesKHR(m_LogicalDevice->GetLogicalDevice(), m_SwapChain, &ImageCounter, nullptr);
		m_VulkanImages.resize(ImageCounter);
		vkGetSwapchainImagesKHR(m_LogicalDevice->GetLogicalDevice(), m_SwapChain, &ImageCounter, m_VulkanImages.data());
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
			viewInfo.format = m_Format;
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
		VkAttachmentDescription ColorAttachment{};
		ColorAttachment.format = m_Format;
		ColorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		ColorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; //Clear to black before drawing new frame
		ColorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		ColorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		ColorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		ColorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		ColorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference ColorAttachmentReference{};
		ColorAttachmentReference.attachment = 0;
		ColorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription SubPassDescription{};
		SubPassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		SubPassDescription.colorAttachmentCount = 1;
		SubPassDescription.pColorAttachments = &ColorAttachmentReference;

		VkSubpassDependency SubPassDependency{};
		SubPassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		SubPassDependency.dstSubpass = 0;
		SubPassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		SubPassDependency.srcAccessMask = 0;
		SubPassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		SubPassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		std::array<VkAttachmentDescription, 1> attachments = { ColorAttachment };

		VkRenderPassCreateInfo RenderPassCreateInfo{};
		RenderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		RenderPassCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		RenderPassCreateInfo.pAttachments = attachments.data();
		RenderPassCreateInfo.subpassCount = 1;
		RenderPassCreateInfo.pSubpasses = &SubPassDescription;
		RenderPassCreateInfo.dependencyCount = 1;
		RenderPassCreateInfo.pDependencies = &SubPassDependency;

		if (vkCreateRenderPass(m_LogicalDevice->GetLogicalDevice(), &RenderPassCreateInfo, nullptr, &m_RenderPass) != VK_SUCCESS)
		{
			assert(false);
		}
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
			FramebufferCreateInfo.renderPass = m_RenderPass;
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

	void SwapChain::RecordCommandBuffer(VkCommandBuffer CommandBuffer, uint32_t imageindex)
	{
		VkCommandBufferBeginInfo CommandBufferInfo{};
		CommandBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		CommandBufferInfo.flags = 0;
		CommandBufferInfo.pInheritanceInfo = nullptr;

		if (vkBeginCommandBuffer(CommandBuffer, &CommandBufferInfo) != VK_SUCCESS)
		{
			assert(false);
		}

		VkRenderPassBeginInfo RenderPassInfo{};
		RenderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		RenderPassInfo.framebuffer = m_SwapChainFramebuffers[imageindex];
		RenderPassInfo.renderPass = m_RenderPass;
		RenderPassInfo.renderArea.offset = { 0, 0 };
		RenderPassInfo.renderArea.extent = m_Extent;

		std::array<VkClearValue, 2> ClearColor{}; //Order of these should be same as order of attachments!!
		ClearColor[0].color = { { 0.0f, 0.0f, 0.0f, 1.0f } };
		ClearColor[1].depthStencil = { 1.0f, 0 };

		RenderPassInfo.clearValueCount = static_cast<uint32_t>(ClearColor.size());
		RenderPassInfo.pClearValues = ClearColor.data();

		vkCmdBeginRenderPass(CommandBuffer, &RenderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(m_Extent.width);
		viewport.height = static_cast<float>(m_Extent.height);
		viewport.maxDepth = 1.0f;
		viewport.minDepth = 0.0f;
		vkCmdSetViewport(CommandBuffer, 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.extent = m_Extent;
		scissor.offset = { 0,0 };
		vkCmdSetScissor(CommandBuffer, 0, 1, &scissor);

		vkCmdEndRenderPass(CommandBuffer);

		if (vkEndCommandBuffer(CommandBuffer) != VK_SUCCESS)
		{
			assert(false);
		}
	}

	SwapChainDetails SwapChain::QuerySwapChainSupprt(VkPhysicalDevice device)
	{
		SwapChainDetails Details;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_WindowSurface, &Details.Capabilities);

		uint32_t FormatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_WindowSurface, &FormatCount, nullptr);

		if (FormatCount != 0)
		{
			Details.Formats.resize(FormatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_WindowSurface, &FormatCount, Details.Formats.data());
		}

		uint32_t PresentCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_WindowSurface, &PresentCount, nullptr);

		if (PresentCount != 0)
		{
			Details.PresentModes.resize(PresentCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_WindowSurface, &PresentCount, Details.PresentModes.data());
		}

		return Details;
	}

	VkSurfaceFormatKHR SwapChain::ChooseSwapChainFormat(const std::vector<VkSurfaceFormatKHR>& AvailableFormats)
	{
		for (const auto& format : AvailableFormats)
		{
			if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			{
				return format;
			}
		}

		return AvailableFormats[0];
	}

	//VkExtent2D SwapChain::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& Capabilities)
	//{
	//	if (Capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
	//	{
	//		return Capabilities.currentExtent;
	//	}
	//	else
	//	{
	//		int width, height;
	//		glfwGetFramebufferSize(m_Handle, &width, &height);

	//		VkExtent2D Extent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };
	//		Extent.width = std::clamp(Extent.width, Capabilities.minImageExtent.width, Capabilities.maxImageExtent.width);
	//		Extent.height = std::clamp(Extent.height, Capabilities.minImageExtent.height, Capabilities.maxImageExtent.height);
	//		return Extent;
	//	}
	//}
}