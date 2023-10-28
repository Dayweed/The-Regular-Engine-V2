#include "pch.h"
#include "Core/Engine.h"
#include "Core/Transform.h"
#include "SceneRenderer.h"
#include "RendererContext.h"
#include "MeshRenderer.h"
#include "Camera.h"
#include "Core/Logger.h"
#include "ShaderReflection.h"
#include "VulkanTexture.h"
#include "Resource/ResourceManager.h"
#include "Physics/PhysicsComponents.h"
#include "Light.h"

#include "glm/gtx/transform.hpp"
#include "glm/gtx/quaternion.hpp"
#include "VulkanUtilities.h"

//To be removed
#include "EditorCamera.h"

namespace TRE
{
	std::shared_ptr<DescriptorPool>& SceneRenderer::GetDescriptorPool()
	{
		return m_DescriptorPool;
	}

	std::vector<std::unique_ptr<Image2D>>& SceneRenderer::GetColorImages()
	{
		return m_ColorImages;
	}

	SceneRenderer::SceneRenderer(const std::shared_ptr<Device>& Device) : m_Device(Device)
	{
		// uint32_t ImageCount = Engine::GetInstance().GetWindow()->GetSwapChain()->GetImageCount();

		Create();

		m_CommandBuffer = std::make_shared<CommandBuffer>("SceneRendererCommmandBuffer");

		m_DescriptorPool = DescriptorPool::Builder().SetMaxSets(100).AddPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 100).AddPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 100).Build();

		m_UBOBuffer = std::make_shared<UniformBuffer>(sizeof(UBO), 0);

		//m_AnimationUBO = std::make_shared<UniformBuffer>(sizeof(AnimationUBO), 0);
		//m_L2W = glm::identity<glm::mat4>();
		//m_L2W = glm::scale(m_L2W, glm::vec3(0.1f, 0.1f, 0.1f));
		//m_L2W = glm::translate(m_L2W, glm::vec3(0.1f, -100.f, 150.f));
		//for (int x = 0; x < 256; x++)
		//{
		//	m_AnimationBuffer.L2W[x] = glm::identity<glm::mat4>();
		//}
	}

	void SceneRenderer::Initialize()
	{
		auto SwapChain = Engine::GetInstance().GetWindow()->GetSwapChain();

		RenderPassInfo RenderPassCreateInfo{};
		RenderPassCreateInfo.FinalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		RenderPassCreateInfo.ImageFormat = SwapChain->GetColorFormat();
		RenderPassCreateInfo.DepthImageFormat = SwapChain->GetDepthFormat();
		RenderPassCreateInfo.DepthFinalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		RenderPassCreateInfo.DepthEnabled = true;
		m_RenderPass = std::make_shared<RenderPass>(m_Device, RenderPassCreateInfo);

		CreateFrameBuffer(m_RenderPass);

		auto PBRShader = ResourceManager::Instance().GetResource<Shader>(PBR::GetShaderHandle());

		PipelineConfigurations PipelineConfig{};
		PipelineConfig.Primitive = PrimitiveType::Triangles;
		PipelineConfig.Shader = PBRShader;
		m_Pipeline = std::make_unique<Pipeline>(PipelineConfig, m_RenderPass);

		PipelineConfigurations SkyboxPipelineConfig{};
		SkyboxPipelineConfig.Primitive = PrimitiveType::Triangles;
		SkyboxPipelineConfig.Shader = ResourceManager::Instance().GetResource<Shader>(3);
		m_SkyboxPipeline = std::make_unique<Pipeline>(SkyboxPipelineConfig, m_RenderPass);

		for (auto material : ResourceManager::Instance().GetResourcesOfType<Material>())
		{
			material->Invalidate();
		}

		m_DebugRenderer = std::make_unique<DebugRenderer>(m_RenderPass);

		//m_Animation = std::make_unique<AnimationTest>(m_RenderPass);

		LoadCubeMap();

		m_SkyboxMaterial = std::make_unique<Material>(SkyboxPipelineConfig.Shader);
		m_SkyboxMaterial->Invalidate();
		m_SkyboxMaterial->SetSkyboxTexture("SamplerCubeMap", m_SkyboxTexture);
	}

	void SceneRenderer::CreateFrameBuffer(std::shared_ptr<RenderPass>& renderpass)
	{
		uint32_t ImageCount = Engine::GetInstance().GetWindow()->GetSwapChain()->GetImageCount();
		auto SwapChain = Engine::GetInstance().GetWindow()->GetSwapChain();
		m_FrameBuffer.resize(ImageCount);
		for (int x = 0; x < m_FrameBuffer.size(); x++)
		{
			VkFramebufferCreateInfo fbufCreateInfo{};
			fbufCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			fbufCreateInfo.renderPass = renderpass->GetHandle();
			std::array<VkImageView, 2> attachments = { m_ColorImages[x]->GetImageData().ImageView, m_DepthImages[x]->GetImageData().ImageView };

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

	void SceneRenderer::Create()
	{
		uint32_t ImageCount = Engine::GetInstance().GetWindow()->GetSwapChain()->GetImageCount();
		m_ColorImages.resize(ImageCount);
		m_DepthImages.resize(ImageCount);

		auto SwapChain = Engine::GetInstance().GetWindow()->GetSwapChain();

		ImageConfig ImageCon{};
		ImageCon.CreateSampler = true;
		ImageCon.DebugName = "SceneRendererColorAttachment";
		ImageCon.Transfer = true;
		ImageCon.Format = ImageFormat::RGBA;
		ImageCon.Height = SwapChain->GetHeight();
		ImageCon.Width = SwapChain->GetWidth();
		ImageCon.Usage = ImageUsage::Attachment;
		for (int x = 0; x < m_ColorImages.size(); x++)
		{
			m_ColorImages[x] = std::make_unique<Image2D>(ImageCon);
		}

		ImageConfig ImageCon2{};
		ImageCon2.CreateSampler = true;
		ImageCon2.DebugName = "SceneRenderer";
		ImageCon2.Transfer = true;
		ImageCon2.Height = SwapChain->GetHeight();
		ImageCon2.Width = SwapChain->GetWidth();
		ImageCon2.Usage = ImageUsage::Attachment;
		ImageCon2.Format = ImageFormat::DEPTH24STENCIL8;
		for (int x = 0; x < m_DepthImages.size(); x++)
		{
			m_DepthImages[x] = std::make_unique<Image2D>(ImageCon2);
		}
	}

	void SceneRenderer::Resize()
	{
		for (int x = 0; x < m_FrameBuffer.size(); x++)
		{
			vkDestroyFramebuffer(m_Device->GetLogicalDevice(), m_FrameBuffer[x], nullptr);
		}

		m_ColorImages.clear();
		m_DepthImages.clear();

		Create();
		CreateFrameBuffer(m_RenderPass);
	}

	SceneRenderer::~SceneRenderer()
	{
		Shutdown();
	}

	void SceneRenderer::Shutdown()
	{
		auto Device = m_Device->GetLogicalDevice();

		vkDeviceWaitIdle(Device);

		for (int x = 0; x < m_ColorImages.size(); x++)
		{
			vkDestroyFramebuffer(Device, m_FrameBuffer[x], nullptr);
		}

		m_ColorImages.clear();
		m_DepthImages.clear();

		vkDestroyImageView(Device, m_SkyboxTexture->imageview, nullptr);
		vkDestroyImage(Device, m_SkyboxTexture->image, nullptr);
		vkFreeMemory(Device, m_SkyboxTexture->deviceMemory, nullptr);
		vkDestroySampler(Device, m_SkyboxTexture->sampler, nullptr);
	}

	void SceneRenderer::BeginEditorFrame()
	{
		const EditorCamera& editorCamera = EditorCamera::Instance();
		const Transform& transform = ECSSystemManager::Instance().GetSystem<CameraSystem>()->GetMainCamera()->GetComponent<Transform>();
		
		UBO ubo{};
		ubo.m_ProjView = editorCamera.GetViewProjectionMatrix();
		ubo.m_LightPosition = transform.m_Position;
		ubo.m_CameraPosition = glm::vec4(transform.m_Position, 1.f);
		ubo.Proj = editorCamera.GetProjectionMatrix();
		ubo.View = editorCamera.GetViewMatrix();

		for (const auto& entity : ECSManager::Instance().GetEntities<DirectionalLight>())
		{
			const auto& light = entity->GetComponent<DirectionalLight>();
			ubo.m_LightDirection = glm::vec4(light.Direction, 1.f);
			ubo.m_LightAmbientColor = light.AmbientColor;
		}

		m_UBOBuffer->SetData(&ubo, sizeof(UBO));
	}

	void SceneRenderer::BeginFrame()
	{
		//UBO
		const Entity& mainCamera = ECSSystemManager::Instance().GetSystem<CameraSystem>()->GetMainCamera();
		UBO ubo{};
		const Camera& cameraComponent = mainCamera->GetComponent<Camera>();
		const Transform& transform = mainCamera->GetComponent<Transform>();
		ubo.m_ProjView = cameraComponent.m_BaseCamera.m_ProjectionMatrix * cameraComponent.m_BaseCamera.m_ViewMatrix;
		ubo.m_LightPosition = transform.m_Position;
		ubo.m_CameraPosition = glm::vec4(transform.m_Position, 1.f);

		ubo.Proj = cameraComponent.m_BaseCamera.m_ProjectionMatrix;
		ubo.View = cameraComponent.m_BaseCamera.m_ViewMatrix;

		for (const auto& entity : ECSManager::Instance().GetEntities<DirectionalLight>())
		{
			const auto& light = entity->GetComponent<DirectionalLight>();
			ubo.m_LightDirection = glm::vec4(light.Direction, 1.f);
			ubo.m_LightAmbientColor = light.AmbientColor;
		}

		m_UBOBuffer->SetData(&ubo, sizeof(UBO));
		//m_AnimationBuffer.ProjView = mainCamera.m_ProjectionMatrix * mainCamera.m_ViewMatrix;
		//m_Animation->UpdateAnimations(m_AnimationBuffer, m_L2W);
		//m_AnimationUBO->SetData(&m_AnimationBuffer, sizeof(AnimationUBO));
	}

	void SceneRenderer::EndFrame(bool IsEditorScene)
	{
		uint32_t Index = Engine::GetInstance().GetWindow()->GetSwapChain()->GetCurrentBufferIndex();
		uint32_t ImageIndex = Engine::GetInstance().GetWindow()->GetSwapChain()->GetCurrentImageIndex();
		// auto& swapChain = Engine::GetInstance().GetWindow()->GetSwapChain();

		m_CommandBuffer->Begin();

		m_RenderPass->BeginRenderPass(m_CommandBuffer->GetInUseCommandBuffer(), m_FrameBuffer[ImageIndex]);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.f;
		viewport.width = static_cast<float>(Engine::GetInstance().GetWindow()->GetSwapChain()->GetWidth());
		viewport.height = static_cast<float>(Engine::GetInstance().GetWindow()->GetSwapChain()->GetHeight());
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(m_CommandBuffer->GetInUseCommandBuffer(), 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = Engine::GetInstance().GetWindow()->GetSwapChain()->GetSwapChainExtent();
		vkCmdSetScissor(m_CommandBuffer->GetInUseCommandBuffer(), 0, 1, &scissor);

		vkCmdBindPipeline(m_CommandBuffer->GetInUseCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline->GetPipeline());

		//std::set<ResourceHandle> renderedMaterials;
		std::multimap<ResourceHandle, Entity> materialSort;

		for (const auto& go_mr : ECSManager::Instance().GetEntities<MeshRenderer>())
		{
			const MeshRenderer& mr = go_mr->GetComponent<MeshRenderer>();
			if(mr.m_RenderObject == nullptr)
				continue;

			if (mr.m_IsVisible == false)
				continue;

			ResourceHandle materialHandle;

			//If entity has no material, use default
			if (mr.m_MaterialInstance == nullptr)
			{
				materialHandle = PBR::GetDefaultMaterial();
				if (m_DefaultPBRMaterial == nullptr)
				{
					m_DefaultPBRMaterial = ResourceManager::Instance().GetResource<Material>(materialHandle);
					m_DefaultPBRMaterial->Invalidate();
				}
			}
			else
			{
				materialHandle = mr.m_MaterialInstance->GetHandle();
			}
			materialSort.insert(std::make_pair(materialHandle, go_mr));
		}

		//Geom Pass
		for (const auto& go_mr : materialSort)
		{
			const MeshRenderer& mr = go_mr.second->GetComponent<MeshRenderer>();

			PushConstant pc{};
			pc.m_Model = go_mr.second->GetComponent<Transform>().m_WorldXform;
			vkCmdPushConstants(m_CommandBuffer->GetInUseCommandBuffer(), m_Pipeline->GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstant), &pc);

			ResourceHandle currentMaterialHandle = go_mr.first;

			//If no material instance attached, use default PBR material
			if (currentMaterialHandle != m_PreviousMaterialHandle)
			{
				if (mr.m_MaterialInstance == nullptr)
				{
					if (IsEditorScene)
					{
						m_DefaultPBRMaterial->UpdateForEditorSceneRendering(m_UBOBuffer, Index);
						vkCmdBindDescriptorSets(m_CommandBuffer->GetInUseCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline->GetPipelineLayout(), 0, 1, &m_DefaultPBRMaterial->GetEditorDescriptor(Index), 0, NULL);
					}
					else
					{
						m_DefaultPBRMaterial->UpdateForRendering(m_UBOBuffer, Index);
						vkCmdBindDescriptorSets(m_CommandBuffer->GetInUseCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline->GetPipelineLayout(), 0, 1, &m_DefaultPBRMaterial->GetDescriptor(Index), 0, NULL);
					}
				}
				else
				{
					if (IsEditorScene)
					{
						mr.m_MaterialInstance->UpdateForEditorSceneRendering(m_UBOBuffer, Index);
						vkCmdBindDescriptorSets(m_CommandBuffer->GetInUseCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline->GetPipelineLayout(), 0, 1, &mr.m_MaterialInstance->GetEditorDescriptor(Index), 0, NULL);
					}
					else
					{
						mr.m_MaterialInstance->UpdateForRendering(m_UBOBuffer, Index);
						vkCmdBindDescriptorSets(m_CommandBuffer->GetInUseCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline->GetPipelineLayout(), 0, 1, &mr.m_MaterialInstance->GetDescriptor(Index), 0, NULL);
					}
				}
			}

			mr.m_RenderObject->Bind(m_CommandBuffer->GetInUseCommandBuffer());
			mr.m_RenderObject->Draw(m_CommandBuffer->GetInUseCommandBuffer());

			m_PreviousMaterialHandle = currentMaterialHandle;
		}

		m_PreviousMaterialHandle = 0;
		
		//Debug Drawing Pass

		if (IsEditorScene)
		{
			DebugDrawPass(Index);
		}

		//Skybox Pass
		{
			vkCmdBindPipeline(m_CommandBuffer->GetInUseCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_SkyboxPipeline->GetPipeline());
			if (IsEditorScene)
			{
				m_SkyboxMaterial->UpdateSkyboxPassEditor(m_UBOBuffer, Index);
				vkCmdBindDescriptorSets(m_CommandBuffer->GetInUseCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_SkyboxPipeline->GetPipelineLayout(), 0, 1, &m_SkyboxMaterial->GetEditorDescriptor(Index), 0, NULL);
			}
			else
			{
				m_SkyboxMaterial->UpdateSkyboxPass(m_UBOBuffer, Index);
				vkCmdBindDescriptorSets(m_CommandBuffer->GetInUseCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_SkyboxPipeline->GetPipelineLayout(), 0, 1, &m_SkyboxMaterial->GetDescriptor(Index), 0, NULL);
			}

			VkBuffer vertexBuffers[] = { m_SkyboxVertexBuffer->GetBuffer() };
			VkDeviceSize offsets[] = { 0 };
			vkCmdBindVertexBuffers(m_CommandBuffer->GetInUseCommandBuffer(), 0, 1, vertexBuffers, offsets);

			vkCmdBindIndexBuffer(m_CommandBuffer->GetInUseCommandBuffer(), m_SkyboxIndexBuffer->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);

			vkCmdDrawIndexed(m_CommandBuffer->GetInUseCommandBuffer(), m_SkyboxIndexCount, 1, 0, 0, 0);
		}

		//Animation Pass
		{
			//m_Animation->BindPipeline(m_CommandBuffer->GetInUseCommandBuffer());
			//m_Animation->UpdateMaterial(m_AnimationUBO, Index);

			//vkCmdBindDescriptorSets(m_CommandBuffer->GetInUseCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_Animation->GetPipelineLayout(), 0, 1, &m_Animation->GetDescriptorSet(Index), 0, NULL);
			//m_Animation->BindBuffers(m_CommandBuffer->GetInUseCommandBuffer());
			//m_Animation->Draw(m_CommandBuffer->GetInUseCommandBuffer());
		}

		m_RenderPass->EndRenderPass(m_CommandBuffer->GetInUseCommandBuffer());

		m_CommandBuffer->End();
		m_CommandBuffer->Submit();
	}

	void SceneRenderer::DebugDrawPass(uint32_t Index) //Debug Pass
	{
		m_DebugRenderer->BindPipeline(m_CommandBuffer->GetInUseCommandBuffer());
		m_DebugRenderer->UpdateMaterial(m_UBOBuffer, Index);
		vkCmdBindDescriptorSets(m_CommandBuffer->GetInUseCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_DebugRenderer->GetPipelineLayout(), 0, 1, &m_DebugRenderer->GetDescriptor(Index), 0, NULL);

		for (const auto& spheres : ECSManager::Instance().GetEntities<SphereCollider>())
		{
			const Transform& tr = spheres->GetComponent<Transform>();
			const SphereCollider& sc = spheres->GetComponent<SphereCollider>();
			if(sc.m_IsVisible == false)
				continue;

			PushConstant pc{};
			glm::mat4 model(1.f);
			model = glm::translate(model, tr.m_Position + sc.m_Offset);
			const float radius = sc.m_Radius;
			model = glm::scale(model, glm::vec3(radius, radius, radius));
			pc.m_Model = model;
			vkCmdPushConstants(m_CommandBuffer->GetInUseCommandBuffer(), m_DebugRenderer->GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstant), &pc);

			m_DebugRenderer->BindDebugSphere(m_CommandBuffer->GetInUseCommandBuffer());
			m_DebugRenderer->DrawDebugSphere(m_CommandBuffer->GetInUseCommandBuffer());
		}

		for (const auto& box : ECSManager::Instance().GetEntities<BoxCollider>())
		{
			const Transform& tr = box->GetComponent<Transform>();
			const BoxCollider& bc = box->GetComponent<BoxCollider>();
			if (bc.m_IsVisible == false)
				continue;

			PushConstant pc{};
			glm::mat4 model(1.f);
			model = glm::translate(model, tr.m_Position + bc.m_Offset);
			model = model * glm::mat4_cast(glm::quat(glm::radians(tr.m_Rotation)));
			const glm::vec3 scale = bc.m_HalfExtents * 2.f;
			model = model * glm::scale(glm::mat4(1.f), scale);
			pc.m_Model = model;
			vkCmdPushConstants(m_CommandBuffer->GetInUseCommandBuffer(), m_DebugRenderer->GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstant), &pc);

			m_DebugRenderer->BindDebugAABB(m_CommandBuffer->GetInUseCommandBuffer());
			m_DebugRenderer->DrawDebugAABB(m_CommandBuffer->GetInUseCommandBuffer());
		}

		for (const auto& capsule : ECSManager::Instance().GetEntities<CapsuleCollider>())
		{
			for (int i = 0; i < 2; ++i)
			{
				const Transform& tr = capsule->GetComponent<Transform>();
				const CapsuleCollider& cc = capsule->GetComponent<CapsuleCollider>();
				if (cc.m_IsVisible == false)
					continue;

				PushConstant pc{};
				glm::mat4 model(1.f);
				const float radius = cc.m_Radius * 2.f;
				const float halfExtent = cc.m_HalfHeight;
				model = glm::translate(model, tr.m_Position + cc.m_Offset);
				model = glm::rotate(model, glm::radians(90.f * i), glm::vec3(0, 1, 0));
				model = glm::rotate(model, glm::radians(180.f * i), glm::vec3(0, 0, 1));
				model = model * glm::mat4_cast(glm::quat(glm::radians(tr.m_Rotation)));
				model = glm::scale(model, glm::vec3(radius, radius + halfExtent, radius));
				pc.m_Model = model;
				vkCmdPushConstants(m_CommandBuffer->GetInUseCommandBuffer(), m_DebugRenderer->GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstant), &pc);

				//Bind
				vkCmdBindDescriptorSets(m_CommandBuffer->GetInUseCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_DebugRenderer->GetPipelineLayout(), 0, 1, &m_DebugRenderer->GetDescriptor(Index), 0, NULL);

				m_DebugRenderer->BindDebugCapsule(m_CommandBuffer->GetInUseCommandBuffer());
				m_DebugRenderer->DrawDebugCapsule(m_CommandBuffer->GetInUseCommandBuffer());
			}
		}

		for (const auto& camera : ECSManager::Instance().GetEntities<Camera>())
		{
			const Transform& tr = camera->GetComponent<Transform>();
			const Camera& cc = camera->GetComponent<Camera>();

			PushConstant pc{};
			glm::mat4 model(1.f);
			const float scale = 100.f;/*cc.m_BaseCamera.m_Far - cc.m_BaseCamera.m_Near;*/
			model = glm::translate(model, tr.m_Position);
			model = model * glm::mat4_cast(glm::quat(glm::radians(tr.m_Rotation)));
			model = glm::scale(model, glm::vec3(scale, scale, scale));
			pc.m_Model = model;

			vkCmdPushConstants(m_CommandBuffer->GetInUseCommandBuffer(), m_DebugRenderer->GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstant), &pc);
			vkCmdBindDescriptorSets(m_CommandBuffer->GetInUseCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_DebugRenderer->GetPipelineLayout(), 0, 1, &m_DebugRenderer->GetDescriptor(Index), 0, NULL);

			m_DebugRenderer->BindDebugCameraFrustum(m_CommandBuffer->GetInUseCommandBuffer());
			m_DebugRenderer->DrawDebugCameraFrustum(m_CommandBuffer->GetInUseCommandBuffer());
		}
	}

	void SceneRenderer::LoadCubeMap()
	{
		auto Skybox1 = Resource::GetGUIDFromHex("e562694e2c3833ec");
		auto Skybox2 = Resource::GetGUIDFromHex("612fbc6691dcd0bb");
		auto Skybox3 = Resource::GetGUIDFromHex("d7317320914622e8");
		auto Skybox4 = Resource::GetGUIDFromHex("5994bacabaa99f19");
		auto Skybox5 = Resource::GetGUIDFromHex("bb22164f64671a56");
		auto Skybox6 = Resource::GetGUIDFromHex("47335a309e5eef62");

		auto Texture1 = ResourceManager::Instance().GetResource<VulkanTexture>(Skybox1);
		auto Texture2 = ResourceManager::Instance().GetResource<VulkanTexture>(Skybox2);
		auto Texture3 = ResourceManager::Instance().GetResource<VulkanTexture>(Skybox3);
		auto Texture4 = ResourceManager::Instance().GetResource<VulkanTexture>(Skybox4);
		auto Texture5 = ResourceManager::Instance().GetResource<VulkanTexture>(Skybox5);
		auto Texture6 = ResourceManager::Instance().GetResource<VulkanTexture>(Skybox6);

		std::vector<std::shared_ptr<VulkanTexture>> Textures
		{
			Texture4,
			Texture2,
			Texture6,
			Texture5,
			Texture1,
			Texture3
		};

		auto SwapChain = Engine::GetInstance().GetWindow()->GetSwapChain();
		m_SkyboxTexture = std::make_shared<SkyboxTexture>();
		m_SkyboxTexture->width = SwapChain->GetWidth();
		m_SkyboxTexture->height = SwapChain->GetHeight();
		m_SkyboxTexture->mipLevels = 1;
		
		VkDeviceSize ImageCubeMapSize = Texture1->GetWidth() * Texture1->GetHeight() * 4 * 6;
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingMemory;

		VkBufferCreateInfo bufferCreateInfo{}; // This buffer is used as a transfer source for the buffer copy
		bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		bufferCreateInfo.size = ImageCubeMapSize;

		if (auto Result = vkCreateBuffer(m_Device->GetLogicalDevice(), &bufferCreateInfo, nullptr, &stagingBuffer); Result != VK_SUCCESS)
		{
			assert(Result == VK_SUCCESS && "Unable to create staging buffer for cubemap");
		}

		VkMemoryRequirements memReqs;
		vkGetBufferMemoryRequirements(m_Device->GetLogicalDevice(), stagingBuffer, &memReqs);
		
		VkMemoryAllocateInfo memAllocInfo{};
		memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memAllocInfo.allocationSize;
		memAllocInfo.allocationSize = memReqs.size;
		memAllocInfo.memoryTypeIndex = m_Device->FindMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		
		if (auto Result = vkAllocateMemory(m_Device->GetLogicalDevice(), &memAllocInfo, nullptr, &stagingMemory); Result != VK_SUCCESS)
		{
			assert(Result == VK_SUCCESS && "Unable to allocate memory for cubemap");
		}

		if (auto Result = vkBindBufferMemory(m_Device->GetLogicalDevice(), stagingBuffer, stagingMemory, 0); Result != VK_SUCCESS)
		{
			assert(Result == VK_SUCCESS && "Unable to bind memory for cubemap");
		}

		uint8_t* data;
		if (auto Result = vkMapMemory(m_Device->GetLogicalDevice(), stagingMemory, 0, memReqs.size, 0, (void**)&data); Result != VK_SUCCESS)
		{
			assert(Result == VK_SUCCESS && "Unable to map memory for cubemap");
		}
		
		VkDeviceSize layersize = ImageCubeMapSize / 6;
		for (uint32_t x = 0; x < 6; x++)
		{
			memcpy(data + layersize * x, Textures[x]->GetBuffer(), layersize);
		}

		VkImageCreateInfo ImageCreateInfo{};
		ImageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		ImageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
		ImageCreateInfo.format = Texture1->GetFormat();
		ImageCreateInfo.mipLevels = 1;
		ImageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		ImageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		ImageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		ImageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		ImageCreateInfo.extent = { Texture1->GetWidth(), Texture1->GetHeight(), 1};
		ImageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		ImageCreateInfo.arrayLayers = 6;
		ImageCreateInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

		if (auto Result = vkCreateImage(m_Device->GetLogicalDevice(), &ImageCreateInfo, nullptr, &m_SkyboxTexture->image); Result != VK_SUCCESS)
		{
			assert(Result == VK_SUCCESS && "Unable to create image for cubemap");
		}

		VkMemoryRequirements ImagememReqs;
		VkMemoryAllocateInfo ImagememAlloc{};
		
		ImagememAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		vkGetImageMemoryRequirements(m_Device->GetLogicalDevice(), m_SkyboxTexture->image, &memReqs);
		
		ImagememAlloc.allocationSize = memReqs.size;
		ImagememAlloc.memoryTypeIndex = RendererContext::GetDevice()->FindMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		if (auto result = vkAllocateMemory(m_Device->GetLogicalDevice(), &ImagememAlloc, nullptr, &m_SkyboxTexture->deviceMemory); result != VK_SUCCESS)
		{
			assert(result == VK_SUCCESS && "Failed to allocate memory for image");
		}
		if (auto result = vkBindImageMemory(m_Device->GetLogicalDevice(), m_SkyboxTexture->image, m_SkyboxTexture->deviceMemory, 0); result != VK_SUCCESS)
		{
			assert(result == VK_SUCCESS && "Failed to bind image memory");
		}

		VkImageViewCreateInfo ImageViewCreateInfo{};
		ImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		ImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
		ImageViewCreateInfo.format = Texture1->GetFormat();;
		ImageViewCreateInfo.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
		ImageViewCreateInfo.subresourceRange.layerCount = 6;
		ImageViewCreateInfo.image = m_SkyboxTexture->image;
		ImageViewCreateInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };

		if (auto Result = vkCreateImageView(m_Device->GetLogicalDevice(), &ImageViewCreateInfo, nullptr, &m_SkyboxTexture->imageview); Result != VK_SUCCESS)
		{
			assert(Result == VK_SUCCESS && "Unable to create cubemap image view");
		}

		auto cmd = m_Device->AllocateCommandBuffer(true);

		VkImageSubresourceRange range;
		range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		range.baseMipLevel = 0;
		range.levelCount = 1;
		range.baseArrayLayer = 0;
		range.layerCount = 6;

		std::vector<VkBufferImageCopy> bufferCopyRegions;
		for (int x = 0; x < 6; x++)
		{
			VkBufferImageCopy bufferCopyRegion = {};
			bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			bufferCopyRegion.imageSubresource.mipLevel = 0;
			bufferCopyRegion.imageSubresource.baseArrayLayer = x;
			bufferCopyRegion.imageSubresource.layerCount = 1;
			bufferCopyRegion.imageExtent.width = Texture1->GetWidth();
			bufferCopyRegion.imageExtent.height = Texture1->GetHeight();
			bufferCopyRegion.imageExtent.depth = 1;
			bufferCopyRegion.bufferOffset = layersize * x;
			bufferCopyRegions.push_back(bufferCopyRegion);

		}

		VkImageMemoryBarrier imageBarrier_toTransfer{};
		imageBarrier_toTransfer.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		imageBarrier_toTransfer.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageBarrier_toTransfer.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		imageBarrier_toTransfer.image = m_SkyboxTexture->image;
		imageBarrier_toTransfer.subresourceRange = range;

		imageBarrier_toTransfer.srcAccessMask = 0;
		imageBarrier_toTransfer.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageBarrier_toTransfer);

		vkCmdCopyBufferToImage(cmd, stagingBuffer, m_SkyboxTexture->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, static_cast<uint32_t>(bufferCopyRegions.size()), bufferCopyRegions.data());

		VkImageMemoryBarrier imageBarrier_toReadable = imageBarrier_toTransfer;
		imageBarrier_toReadable.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		imageBarrier_toReadable.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageBarrier_toReadable.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		imageBarrier_toReadable.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageBarrier_toReadable);

		m_Device->SubmitCommands(cmd);

		vkUnmapMemory(m_Device->GetLogicalDevice(), stagingMemory);

		VkSamplerCreateInfo samplerCreateInfo = {};
		samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerCreateInfo.maxAnisotropy = 1.0f;
		samplerCreateInfo.magFilter = VK_FILTER_NEAREST;
		samplerCreateInfo.minFilter = VK_FILTER_NEAREST;
		samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerCreateInfo.addressModeV = samplerCreateInfo.addressModeU;
		samplerCreateInfo.addressModeW = samplerCreateInfo.addressModeU;
		samplerCreateInfo.mipLodBias = 0.0f;
		samplerCreateInfo.minLod = 0.0f;
		samplerCreateInfo.maxLod = 1000.0f;
		samplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
		if (auto Result = vkCreateSampler(m_Device->GetLogicalDevice(), &samplerCreateInfo, nullptr, &m_SkyboxTexture->sampler); Result != VK_SUCCESS)
		{
			assert(Result == VK_SUCCESS && "Sampler cannot be created");
		}

		m_SkyboxTexture->descriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		m_SkyboxTexture->descriptor.imageView = m_SkyboxTexture->imageview;
		m_SkyboxTexture->descriptor.sampler = m_SkyboxTexture->sampler;

		//Backface culling
		std::vector<glm::vec3> vertices
		{
				glm::vec3(-0.5f, -0.5f, -0.5f), // Vertex 0
				glm::vec3(0.5f, -0.5f, -0.5f), // Vertex 1
				glm::vec3(0.5f, 0.5f, -0.5f), // Vertex 2
				glm::vec3(-0.5f, 0.5f, -0.5f), // Vertex 3
				glm::vec3(-0.5f, -0.5f, 0.5f), // Vertex 4
				glm::vec3(0.5f, -0.5f, 0.5f), // Vertex 5
				glm::vec3(0.5f, 0.5f, 0.5f), // Vertex 6
				glm::vec3(-0.5f, 0.5f, 0.5f) // Vertex 7
		};
		std::vector<uint32_t> indices
		{
			0, 1, 2, // Triangle 1 (front face)
			2, 3, 0, // Triangle 2 (front face)
			1, 5, 6, // Triangle 3 (right face)
			6, 2, 1, // Triangle 4 (right face)
			7, 6, 5, // Triangle 5 (back face)
			5, 4, 7, // Triangle 6 (back face)
			4, 0, 3, // Triangle 7 (left face)
			3, 7, 4, // Triangle 8 (left face)
			4, 5, 1, // Triangle 9 (bottom face)
			1, 0, 4, // Triangle 10 (bottom face)
			3, 2, 6, // Triangle 11 (top face)
			6, 7, 3  // Triangle 12 (top face)
		};

		int VertexCount = (int)vertices.size();
		uint32_t vertexSize = sizeof(vertices[0]);
		Buffer CubestagingBuffer(vertexSize, VertexCount, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		//Create a staging buffer to copy the vertex data to
		CubestagingBuffer.Map();
		CubestagingBuffer.WriteToBuffer((void*)vertices.data());
		CubestagingBuffer.Unmap();

		//Flush data from staging buffer to vertex buffer
		m_SkyboxVertexBuffer = std::make_unique<Buffer>(vertexSize, VertexCount, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		VkDeviceSize bufferSize = vertexSize * VertexCount;
		vkUtils::CopyBuffer(CubestagingBuffer.GetBuffer(), m_SkyboxVertexBuffer->GetBuffer(), bufferSize);

		//Index
		m_SkyboxIndexCount = (uint32_t)indices.size();

		uint32_t indexSize = sizeof(int);
		Buffer stagingBufferindex(indexSize, m_SkyboxIndexCount, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		stagingBufferindex.Map();
		stagingBufferindex.WriteToBuffer((void*)indices.data());
		stagingBufferindex.Unmap();
		m_SkyboxIndexBuffer = std::make_unique<Buffer>(indexSize, m_SkyboxIndexCount, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		VkDeviceSize indexbufferSize = indexSize * m_SkyboxIndexCount;
		vkUtils::CopyBuffer(stagingBufferindex.GetBuffer(), m_SkyboxIndexBuffer->GetBuffer(), indexbufferSize);

		vkDestroyBuffer(m_Device->GetLogicalDevice(), stagingBuffer, nullptr);
		vkFreeMemory(m_Device->GetLogicalDevice(), stagingMemory, nullptr);
	}
}