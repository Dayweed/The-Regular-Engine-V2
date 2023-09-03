#include "pch.h"
#include "ECS.h"
#include "Engine.h"
#include "Physics/PhysicsSystem.h"
#include "Audio/AudioSystem.h"

//TO DELETE
#pragma region TO DELETE TEST
#include "Graphics/MeshRenderer.h"
#include "Graphics/Camera.h"
#include "Graphics/Texture.h"
#include "Graphics/GeomCompiler.h"

namespace TRE
{
	void DemoScene()
	{
		GO test = _ecs_manager->CreateGO();
		test->AddComponent<Properties>().m_Name = "Test";
		test->AddComponent<Transform>().m_Position.z = 25.f;
		test->GetComponent<Transform>().m_Scale = glm::vec3(20.f, 20.f, 20.f);
		test->GetComponent<Transform>().m_Rotation = glm::vec3(0.f, 0.f, 0.f);
		std::shared_ptr<RenderObject> vase = RenderObject::CreateFromFile("../Assets/smooth_vase.obj");
		test->AddComponent<MeshRenderer>();
		test->GetComponent<MeshRenderer>().m_RenderObject = vase;

		GO test2 = _ecs_manager->CreateGO();
		test2->AddComponent<Properties>().m_Name = "Test2";
		test2->AddComponent<Transform>().m_Position.x = 2.f;
		test2->GetComponent<Transform>().m_Position.z = 50.f;
		test2->GetComponent<Transform>().m_Scale = glm::vec3(20.f, 20.f, 20.f);
		test2->GetComponent<Transform>().m_Rotation = glm::vec3(0.f, 0.f, 45.f);
		test2->AddComponent<MeshRenderer>();
		test2->GetComponent<MeshRenderer>().m_RenderObject = vase;
		
		GO cam = _ecs_manager->CreateGO();
		cam->AddComponent<Properties>().m_Name = "cam";
		cam->AddComponent<Transform>().m_Position;
		cam->AddComponent<Camera>().m_Position = glm::vec3(0.0f, 0.0f, 0.0f);
		cam->GetComponent<Camera>().m_Rotation = glm::vec3(0.0f, 0.0f, 0.0f);
		cam->GetComponent<Camera>().m_Fov = 30.0f;

		//_texture_manager->LoadTexture("../Assets/Test.png", "Test");

		/*GO audio = _ecs_manager->CreateGO();
		audio->AddComponent<Audio>();
		audio->GetComponent<Audio>().m_IsPlaying = true;*/

		_geom_compiler->Compile("../Assets/smooth_vase.obj");

		_system_manager->GetSystem<CameraSystem>()->SetIsMainCamera(cam, true);
		// _system_manager->GetSystem<PhysicsSystem>()->ConstructSphereCollider(test2, { 4, 10, 4 }, 2);
		//_system_manager->GetSystem<AudioSystem>()->LoadFile(audio);
		//_system_manager->GetSystem<AudioSystem>()->Play(audio, true);
	}
}
#pragma endregion TO DELETE TEST

namespace TRE
{
	Engine* Engine::s_Instance = nullptr;

	const std::shared_ptr<Window>& Engine::GetWindow()
	{
		return m_Window;
	}

	Engine& Engine::GetInstance()
	{
		return *s_Instance;
	}

	const std::shared_ptr<Renderer>& Engine::GetRenderer()
	{
		return m_Renderer;
	}

	const std::shared_ptr<VulkanEditor>& Engine::GetVulkanImgui()
	{
		return m_VulkanEditor;
	}

	Engine::Engine(const EngineInfo& EngineInfo)
	{
		s_Instance = this;
		m_EngineInfo = EngineInfo;
		m_Window = std::make_shared<Window>(m_EngineInfo.WindowConfigurations);
		//m_SystemsManager = std::make_unique<SystemManager>();
		
		//m_Renderer = std::make_shared<Renderer>(m_Window->GetRenderContext()->GetDevice());
		//m_Renderer->Initialize();

		//if (m_EngineInfo.EnableEditor)
		//	m_VulkanEditor = std::make_shared<VulkanEditor>(m_Window->GetRenderContext()->GetDeviceInternally());

		m_VKInstance = RendererContext::GetVKInstance();
		PhysicalDeviceSetup();
		CreateLogicalDevice();
		CreateSwapChain();
		CreateImageViews();
		CreateRenderPass();
		CreateCommandPool();
		CreateFrameBuffer();
		CreateCommandbuffer();
		CreateSyncObjects();
	}

	Engine::~Engine()
	{
	}

	void Engine::RegisterECS()
	{
		// Register Components
		_component_manager->RegisterComponent<Removal>("Removal", true);
		_component_manager->RegisterComponent<Properties>("Properties", true);
		_component_manager->RegisterComponent<Transform>("Transform");
		_component_manager->RegisterComponent<MeshRenderer>("Mesh Renderer");
		_component_manager->RegisterComponent<Camera>("Camera");
		_component_manager->RegisterComponent<SphereCollider>("SphereCollider");
		_component_manager->RegisterComponent<BoxCollider>("BoxCollider");
		_component_manager->RegisterComponent<Audio>("Audio");

		// Register Systems
		_system_manager->RegisterSystem<PhysicsSystem>();
		_system_manager->RegisterSystem<CameraSystem>();
		_system_manager->RegisterSystem<AudioSystem>();
	}

	void Engine::Update()
	{
		// To remove eventually
		//_ecs_manager->TESTRUN();

		//DemoScene();

		while (!m_Window->ShouldWindowClose())
		{

			//Update



			//Draw
			//m_Window->GetSwapChain().BeginFrame();
			//m_Renderer->BeginFrame();

			if (m_EngineInfo.EnableEditor)
			{
				//m_VulkanEditor->BeginFrame();
			}

			//_system_manager->UpdateSystem();
			//_system_manager->OnDestroyGO();
			//_ecs_manager->DestroyRemovalGO();

			if (m_EngineInfo.EnableEditor)
			{
				//m_VulkanEditor->EndFrame();
			}

			//m_Window->SwapBuffers();


			uint32_t ImageIndex; //Used to pick framebuffer

			vkWaitForFences(m_LogicalDevice, 1, &m_FlightFence[m_CurrentFrame], VK_TRUE, UINT64_MAX); //Wait for previous frame to finish //uin64_max disable timeout
			VkResult Result = vkAcquireNextImageKHR(m_LogicalDevice, m_SwapChain, UINT64_MAX, m_ImageAvailable[m_CurrentFrame], VK_NULL_HANDLE, &ImageIndex);
			if (Result == VK_ERROR_OUT_OF_DATE_KHR)
			{
				TRE_CORE_INFO("No Acquire");
				RecreateSwapChain();
				return;
			}

			vkResetFences(m_LogicalDevice, 1, &m_FlightFence[m_CurrentFrame]);

			vkResetCommandBuffer(m_Commandbuffer[m_CurrentFrame], 0);

			RecordCommandBuffer(m_Commandbuffer[m_CurrentFrame], ImageIndex);

			VkSubmitInfo SubmitInfo{};
			SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

			VkSemaphore WaitSemaphores[] = { m_ImageAvailable[m_CurrentFrame] };
			VkPipelineStageFlags WaitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
			SubmitInfo.waitSemaphoreCount = 1;
			SubmitInfo.pWaitSemaphores = WaitSemaphores;
			SubmitInfo.pWaitDstStageMask = WaitStages;
			SubmitInfo.commandBufferCount = 1;
			SubmitInfo.pCommandBuffers = &m_Commandbuffer[m_CurrentFrame];

			VkSemaphore SingalSemaphores[] = { m_ImageRendered[m_CurrentFrame] };
			SubmitInfo.signalSemaphoreCount = 1;
			SubmitInfo.pSignalSemaphores = SingalSemaphores;

			if (vkQueueSubmit(m_GraphicsQueue, 1, &SubmitInfo, m_FlightFence[m_CurrentFrame]) != VK_SUCCESS)
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
			PresentInfo.pImageIndices = &ImageIndex;
			PresentInfo.pResults = nullptr; //Only if using more than 1 swapchain

			Result = vkQueuePresentKHR(m_PresentQueue, &PresentInfo);

			if (Result == VK_ERROR_OUT_OF_DATE_KHR || Result == VK_SUBOPTIMAL_KHR)
			{
				TRE_CORE_INFO("Recreate");
				RecreateSwapChain();
			}

			m_CurrentFrame = (m_CurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT; //Go to next frame

			m_Window->PollEvents();
		}
	}

	void Engine::Shutdown()
	{
		_system_manager->ShutdownSystem();
		_ecs_manager->DestroyAll();
	}

	VkSurfaceFormatKHR Engine::ChooseSwapChainFormat(const std::vector<VkSurfaceFormatKHR>& AvailableFormats)
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

	VkPresentModeKHR Engine::ChooseSwapChainPresentMode(const std::vector<VkPresentModeKHR>& AvailableModes)
	{
		for (const auto& PresentMode : AvailableModes)
		{
			if (PresentMode == VK_PRESENT_MODE_MAILBOX_KHR) //Something like Triple buffering, avoid tearing, render images as newest as possible, but might use more energy
			{
				return PresentMode;
			}
		}
		return VK_PRESENT_MODE_FIFO_KHR; //FIFO Queue for images, less energy consumed
	}

	void Engine::CleanSwapChain()
	{
		for (auto fb : m_SwapChainFramebuffers)
		{
			vkDestroyFramebuffer(m_LogicalDevice, fb, nullptr);
		}
		for (auto imageview : m_SwapChainImageViews)
		{
			vkDestroyImageView(m_LogicalDevice, imageview, nullptr);
		}
	}
	SwapChainDetails Engine::QuerySwapChainSupprt(VkPhysicalDevice device)
	{
		SwapChainDetails Details;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_Window->GetSurface(), &Details.Capabilities);

		uint32_t FormatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_Window->GetSurface(), &FormatCount, nullptr);

		if (FormatCount != 0)
		{
			Details.Formats.resize(FormatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_Window->GetSurface(), &FormatCount, Details.Formats.data());
		}

		uint32_t PresentCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_Window->GetSurface(), &PresentCount, nullptr);

		if (PresentCount != 0)
		{
			Details.PresentModes.resize(PresentCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_Window->GetSurface(), &PresentCount, Details.PresentModes.data());
		}

		return Details;
	}
	void Engine::RecordCommandBuffer(VkCommandBuffer CommandBuffer, uint32_t imageindex)
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
	Engine::QueueFamilies Engine::FindQueueFamilies(VkPhysicalDevice dev)
	{
		QueueFamilies MyQueues;

		uint32_t Queuecount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(dev, &Queuecount, nullptr); //Get number of queue by passing nullptr

		std::vector<VkQueueFamilyProperties> QueueFamilies(Queuecount);
		vkGetPhysicalDeviceQueueFamilyProperties(dev, &Queuecount, QueueFamilies.data()); //Get the actual queues properties by passing data

		int i = 0;
		for (const auto& queuefamily : QueueFamilies)
		{
			if (queuefamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				MyQueues.Graphics = i;
			}

			VkBool32 PresentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(dev, i, m_Window->GetSurface(), &PresentSupport);

			if (PresentSupport)
			{
				MyQueues.Present = i;
			}

			if (MyQueues.IsComplete())
			{
				break;
			}
			i++;
		}
		return MyQueues;
	}
	void Engine::RecreateSwapChain()
	{
		int width = 0, height = 0;
		glfwGetFramebufferSize(m_Window->GetWindowHandle(), &width, &height);
		while (width == 0 || height == 0)
		{
			glfwGetFramebufferSize(m_Window->GetWindowHandle(), &width, &height);
			glfwWaitEvents();
		}

		vkDeviceWaitIdle(m_LogicalDevice);

		CleanSwapChain();

		CreateSwapChain();
		CreateImageViews();
		CreateFrameBuffer();
	}


	VkExtent2D Engine::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& Capabilities)
	{
		if (Capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
		{
			return Capabilities.currentExtent;
		}
		else
		{
			int width, height;
			glfwGetFramebufferSize(m_Window->GetWindowHandle(), &width, &height);

			VkExtent2D Extent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };
			Extent.width = std::clamp(Extent.width, Capabilities.minImageExtent.width, Capabilities.maxImageExtent.width);
			Extent.height = std::clamp(Extent.height, Capabilities.minImageExtent.height, Capabilities.maxImageExtent.height);
			return Extent;
		}
	}


	bool Engine::CheckDeviceExtensionSupport(VkPhysicalDevice device)
	{
		uint32_t ExtensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &ExtensionCount, nullptr); //Get the count

		std::vector<VkExtensionProperties> AvailableExtensions(ExtensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &ExtensionCount, AvailableExtensions.data()); //Get the properties

		std::set<std::string> RequiredExtension(m_DeviceExtensions.begin(), m_DeviceExtensions.end());

		for (const auto& extension : AvailableExtensions)
		{
			RequiredExtension.erase(extension.extensionName);
		}

		return RequiredExtension.empty();
	}

	void Engine::PhysicalDeviceSetup()
	{
		uint32_t PhysicalDeviceCount = GetPhysicalDeviceCount();
		std::vector<VkPhysicalDevice> PhysicalDevice(PhysicalDeviceCount);
		vkEnumeratePhysicalDevices(m_VKInstance, &PhysicalDeviceCount, PhysicalDevice.data());

		std::cout << "Available GPUs:\n";
		for (const auto& device : PhysicalDevice)
		{
			VkPhysicalDeviceProperties DeviceProp;
			vkGetPhysicalDeviceProperties(device, &DeviceProp);
			std::cout << DeviceProp.deviceName << std::endl;
		}
		for (const auto& device : PhysicalDevice)
		{
			if (IsPhysicalDeviceSuitable(device))
			{
				m_PhysicalDevice = device;
				break;
			}
		}

		assert(m_PhysicalDevice != VK_NULL_HANDLE);
	}

	bool Engine::IsPhysicalDeviceSuitable(VkPhysicalDevice pd)
	{
		QueueFamilies queues = FindQueueFamilies(pd);
		bool ExtensionSupported = CheckDeviceExtensionSupport(pd);

		bool SwapChainSupported = false;
		if (ExtensionSupported)
		{
			SwapChainDetails Details = QuerySwapChainSupprt(pd);
			SwapChainSupported = !Details.Formats.empty() && !Details.PresentModes.empty();
		}

		VkPhysicalDeviceFeatures supportedFeatures;
		vkGetPhysicalDeviceFeatures(pd, &supportedFeatures);

		return queues.IsComplete() && ExtensionSupported && SwapChainSupported && supportedFeatures.samplerAnisotropy;
	}

	uint32_t Engine::GetPhysicalDeviceCount()
	{
		uint32_t PhysicalDeviceCount = 0;
		vkEnumeratePhysicalDevices(m_VKInstance, &PhysicalDeviceCount, nullptr);
		if (PhysicalDeviceCount == 0)
		{
			assert(false);
		}
		return PhysicalDeviceCount;
	}

	void Engine::CreateLogicalDevice()
	{
		QueueFamilies queuefamily = FindQueueFamilies(m_PhysicalDevice);

		std::vector<VkDeviceQueueCreateInfo> AllQueueInfos;
		std::set<int32_t> UniqueQueueFamilies = { queuefamily.Graphics, queuefamily.Present };

		float QueuePiority = 1.f;

		for (uint32_t QueueFamily : UniqueQueueFamilies)
		{
			VkDeviceQueueCreateInfo Queueinfo{};
			Queueinfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			Queueinfo.queueFamilyIndex = QueueFamily;
			Queueinfo.queueCount = 1;
			Queueinfo.pQueuePriorities = &QueuePiority;
			AllQueueInfos.push_back(Queueinfo);
		}

		VkPhysicalDeviceFeatures physicalfeatures{}; //Later
		physicalfeatures.samplerAnisotropy = VK_TRUE;

		VkDeviceCreateInfo deviceinfo{};
		deviceinfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceinfo.pQueueCreateInfos = AllQueueInfos.data();
		deviceinfo.queueCreateInfoCount = static_cast<uint32_t>(AllQueueInfos.size());
		deviceinfo.pEnabledFeatures = &physicalfeatures;
		deviceinfo.enabledExtensionCount = static_cast<uint32_t>(m_DeviceExtensions.size());
		deviceinfo.ppEnabledExtensionNames = m_DeviceExtensions.data();

		if (EnableValidationLayer)
		{
			deviceinfo.enabledLayerCount = static_cast<uint32_t>(m_ValidationLayers.size());
			deviceinfo.ppEnabledLayerNames = m_ValidationLayers.data();
		}
		else
		{
			deviceinfo.enabledLayerCount = 0;
		}

		if (vkCreateDevice(m_PhysicalDevice, &deviceinfo, nullptr, &m_LogicalDevice) != VK_SUCCESS)
		{
			assert(false);
		}

		vkGetDeviceQueue(m_LogicalDevice, queuefamily.Graphics, 0, &m_GraphicsQueue);
		vkGetDeviceQueue(m_LogicalDevice, queuefamily.Present, 0, &m_PresentQueue);
	}

	void Engine::CreateSwapChain()
	{
		VkSwapchainKHR OldSwapChain = m_SwapChain;

		SwapChainDetails Details = QuerySwapChainSupprt(m_PhysicalDevice);
		VkSurfaceFormatKHR surfaceformat = ChooseSwapChainFormat(Details.Formats);
		VkPresentModeKHR PresentMode = ChooseSwapChainPresentMode(Details.PresentModes);
		VkExtent2D Extent = ChooseSwapExtent(Details.Capabilities);
		m_Extent = Extent;
		m_Format = surfaceformat.format;

		uint32_t ImageCount = Details.Capabilities.minImageCount + 1; //Number of images in swapchain
		if (Details.Capabilities.maxImageCount > 0 && ImageCount > Details.Capabilities.maxImageCount) //Don't go over max
		{
			ImageCount = Details.Capabilities.maxImageCount;
		}

		VkSwapchainCreateInfoKHR CreateInfo{};
		CreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		CreateInfo.surface = m_Window->GetSurface();
		CreateInfo.minImageCount = ImageCount;
		CreateInfo.imageFormat = surfaceformat.format;
		CreateInfo.imageColorSpace = surfaceformat.colorSpace;
		CreateInfo.imageExtent = Extent;
		CreateInfo.imageArrayLayers = 1;
		CreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		CreateInfo.oldSwapchain = OldSwapChain;
		CreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		CreateInfo.queueFamilyIndexCount = 0;
		CreateInfo.pQueueFamilyIndices = nullptr;
		CreateInfo.preTransform = Details.Capabilities.currentTransform;
		CreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		CreateInfo.presentMode = PresentMode;
		CreateInfo.clipped = VK_TRUE;

		if (vkCreateSwapchainKHR(m_LogicalDevice, &CreateInfo, nullptr, &m_SwapChain) != VK_SUCCESS)
		{
			assert(false);
		}

		vkDestroySwapchainKHR(m_LogicalDevice, OldSwapChain, nullptr);

		uint32_t ImageCounter;
		vkGetSwapchainImagesKHR(m_LogicalDevice, m_SwapChain, &ImageCounter, nullptr);
		m_Images.resize(ImageCounter);
		vkGetSwapchainImagesKHR(m_LogicalDevice, m_SwapChain, &ImageCounter, m_Images.data());
	}

	void Engine::CreateImageViews()
	{
		m_SwapChainImageViews.resize(m_Images.size());

		for (size_t i = 0; i < m_Images.size(); i++)
		{
			VkImageViewCreateInfo viewInfo{};
			viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			viewInfo.image = m_Images[i];
			viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			viewInfo.format = m_Format;
			viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			viewInfo.subresourceRange.baseMipLevel = 0;
			viewInfo.subresourceRange.levelCount = 1;
			viewInfo.subresourceRange.baseArrayLayer = 0;
			viewInfo.subresourceRange.layerCount = 1;

			if (vkCreateImageView(m_LogicalDevice, &viewInfo, nullptr, &m_SwapChainImageViews[i]) != VK_SUCCESS) {
				assert(false);
			}
		}
	}

	void Engine::CreateRenderPass()
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

		if (vkCreateRenderPass(m_LogicalDevice, &RenderPassCreateInfo, nullptr, &m_RenderPass) != VK_SUCCESS)
		{
			assert(false);
		}
	}

	void Engine::CreateCommandPool()
	{
		QueueFamilies Queuefam = FindQueueFamilies(m_PhysicalDevice);

		VkCommandPoolCreateInfo CommandPoolCreateInfo{};
		CommandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		CommandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		CommandPoolCreateInfo.queueFamilyIndex = Queuefam.Graphics;

		if (vkCreateCommandPool(m_LogicalDevice, &CommandPoolCreateInfo, nullptr, &m_CommandPool) != VK_SUCCESS)
		{
			assert(false);
		}
	}

	void Engine::CreateFrameBuffer()
	{
		m_SwapChainFramebuffers.resize(m_SwapChainImageViews.size());
		for (size_t i = 0; i < m_SwapChainImageViews.size(); i++)
		{
			std::array<VkImageView, 1> Attachments = { m_SwapChainImageViews[i] };

			VkFramebufferCreateInfo FramebufferCreateInfo{};
			FramebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			FramebufferCreateInfo.renderPass = m_RenderPass;
			FramebufferCreateInfo.attachmentCount = static_cast<uint32_t>(Attachments.size());
			FramebufferCreateInfo.pAttachments = Attachments.data();
			FramebufferCreateInfo.width = m_Extent.width;
			FramebufferCreateInfo.height = m_Extent.height;
			FramebufferCreateInfo.layers = 1;

			if (vkCreateFramebuffer(m_LogicalDevice, &FramebufferCreateInfo, nullptr, &m_SwapChainFramebuffers[i]) != VK_SUCCESS)
			{
				assert(false);
			}
		}
	}

	void Engine::CreateCommandbuffer()
	{
		m_Commandbuffer.resize(MAX_FRAMES_IN_FLIGHT);

		VkCommandBufferAllocateInfo CommandBufferInfo{};
		CommandBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		CommandBufferInfo.commandPool = m_CommandPool;
		CommandBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		CommandBufferInfo.commandBufferCount = static_cast<uint32_t>(m_Commandbuffer.size());

		if (vkAllocateCommandBuffers(m_LogicalDevice, &CommandBufferInfo, m_Commandbuffer.data()) != VK_SUCCESS)
		{
			assert(false);
		}
	}

	void Engine::CreateSyncObjects()
	{
		m_ImageAvailable.resize(MAX_FRAMES_IN_FLIGHT);
		m_ImageRendered.resize(MAX_FRAMES_IN_FLIGHT);
		m_FlightFence.resize(MAX_FRAMES_IN_FLIGHT);

		VkSemaphoreCreateInfo SemaphoreCreateInfo{};
		SemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo FenceCreateInfo{};
		FenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		FenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			if (vkCreateSemaphore(m_LogicalDevice, &SemaphoreCreateInfo, nullptr, &m_ImageAvailable[i]) != VK_SUCCESS)
			{
				assert(false);
			}

			if (vkCreateSemaphore(m_LogicalDevice, &SemaphoreCreateInfo, nullptr, &m_ImageRendered[i]) != VK_SUCCESS)
			{
				assert(false);
			}

			if (vkCreateFence(m_LogicalDevice, &FenceCreateInfo, nullptr, &m_FlightFence[i]) != VK_SUCCESS)
			{
				assert(false);
			}
		}
	}


}