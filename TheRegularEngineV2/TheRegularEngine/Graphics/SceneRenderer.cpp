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

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
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

	std::vector<std::shared_ptr<Image2D>> SceneRenderer::GetColorImages()
	{
		return m_ColorImages;
	}

	SceneRenderer::SceneRenderer(const std::shared_ptr<Device>& Device) : m_Device(Device)
	{
		Create();

		m_CommandBuffer = std::make_shared<CommandBuffer>("SceneRendererCommmandBuffer");
		m_DescriptorPool = DescriptorPool::Builder().SetMaxSets(100).AddPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000).AddPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 100).Build();
		m_UBOBuffer = std::make_shared<UniformBuffer>(UINT32_T_CAST(sizeof(UBO)), 0);
		m_UBOSkybox = std::make_shared<UniformBuffer>(UINT32_T_CAST(sizeof(SkyBoxUBO)), 0);
		m_ShadowUBO = std::make_shared<UniformBuffer>(UINT32_T_CAST(sizeof(ShadowUBO)), 0);

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
		ShadowPassInit();

		PipelineConfigurations Config{};
		Config.Primitive = PrimitiveType::Triangles;
		Config.Shader = ResourceManager::Instance().GetResource<Shader>(5);
		m_ShadowPipeline = std::make_shared<Pipeline>(Config, m_ShadowRenderPass);

		m_ShadowMaterial = std::make_shared<Material>(ResourceManager::Instance().GetResource<Shader>(5));
		m_ShadowMaterial->Invalidate();
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
			m_ColorImages[x] = std::make_shared<Image2D>(ImageCon);
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

		vkDestroyFramebuffer(m_Device->GetLogicalDevice(), m_ShadowFramebuffer, nullptr);
		vkDestroyImage(m_Device->GetLogicalDevice(), m_Depth.image, nullptr);
		vkDestroyImageView(m_Device->GetLogicalDevice(), m_Depth.imageview, nullptr);
		vkFreeMemory(m_Device->GetLogicalDevice(), m_Depth.devicememory, nullptr);
		vkDestroySampler(m_Device->GetLogicalDevice(), m_Depth.sampler, nullptr);

		ShadowPassInit();

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
	}

	void SceneRenderer::BeginEditorFrame()
	{
		const EditorCamera& editorCamera = EditorCamera::Instance();
		const Transform& transform = ECSSystemManager::Instance().GetSystem<CameraSystem>()->GetMainCamera()->GetComponent<Transform>();
		
		UBO ubo{};
		ubo.m_ProjView = editorCamera.GetViewProjectionMatrix();
		ubo.m_LightPosition = transform.m_Position;
		ubo.m_CameraPosition = glm::vec4(transform.m_Position, 1.f);

		SkyBoxUBO UBO_SkyBox;
		UBO_SkyBox.Proj = editorCamera.GetProjectionMatrix();
		UBO_SkyBox.View = editorCamera.GetViewMatrix();

		glm::mat4 depthViewMatrix(1.f);
		for (const auto& entity : ECSManager::Instance().GetEntities<DirectionalLight>())
		{
			const auto& lightTransform = entity->GetComponent<Transform>();
			const auto& light = entity->GetComponent<DirectionalLight>();
			ubo.m_LightDirection = glm::vec4(light.Direction, 1.f);
			ubo.m_LightDirectionalColor = light.DirectionalColor;
			ubo.m_LightAmbientColor = light.AmbientColor;
			
			depthViewMatrix = glm::translate(glm::mat4(1.f), EditorCamera::Instance().GetPosition()) * glm::toMat4(glm::quat(glm::radians(-lightTransform.m_Rotation)));
		}

		ShadowUBO UBO_Shadow;
		float lightFOV = 45.0f;
		float orthoSize = 480.0f; // Adjust this to suit your scene's dimensions
		float orthoNear = 0.1f;
		float orthoFar = 1000.0f;
		
		glm::mat4 depthProjectionMatrix;
		depthProjectionMatrix = glm::mat4(1.f);
		depthProjectionMatrix[0][0] = 2.f / (orthoSize - -orthoSize);
		depthProjectionMatrix[1][1] = 2.f / (orthoSize - -orthoSize);
		depthProjectionMatrix[2][2] = 2.f / (orthoFar - orthoNear);
		depthProjectionMatrix[3][0] = -(orthoSize + -orthoSize) / (orthoSize - -orthoSize);
		depthProjectionMatrix[3][1] = -(orthoSize + -orthoSize) / (orthoSize - -orthoSize);
		depthProjectionMatrix[3][2] = -(orthoNear) / (orthoFar - orthoNear);
		depthProjectionMatrix[0][0] *= -1.f;
		depthProjectionMatrix[1][1] *= -1.f;
		depthViewMatrix = glm::inverse(depthViewMatrix);
		UBO_Shadow.view = depthViewMatrix;
		UBO_Shadow.proj = depthProjectionMatrix;
		
		ubo.m_LightSpaceMatrix = depthProjectionMatrix * depthViewMatrix;

		m_UBOBuffer->SetData(&ubo, sizeof(UBO));
		m_UBOSkybox->SetData(&UBO_SkyBox, sizeof(SkyBoxUBO));
		m_ShadowUBO->SetData(&UBO_Shadow, sizeof(ShadowUBO));
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
		
		SkyBoxUBO UBO_SkyBox;
		UBO_SkyBox.Proj = cameraComponent.m_BaseCamera.m_ProjectionMatrix;
		UBO_SkyBox.View = cameraComponent.m_BaseCamera.m_ViewMatrix;

		ShadowUBO UBO_Shadow;
		float lightFOV = 45.0f;
		glm::mat4 depthProjectionMatrix;
		depthProjectionMatrix = glm::mat4(1.f);
		const float tanHalfFov = glm::tan(glm::radians(45.f) / 2.f);
		depthProjectionMatrix[0][0] = 1.f / (tanHalfFov * 16.f/9.f);
		depthProjectionMatrix[1][1] = 1.f / tanHalfFov;
		depthProjectionMatrix[2][2] = 0.5f;
		depthProjectionMatrix[2][3] = 1.f;
		depthProjectionMatrix[3][2] = -(1000 * 0.1) / (1000 - 0.1);
		depthProjectionMatrix[3][3] = 0.f;
		//Fip x and y axis
		//depthProjectionMatrix[0][0] *= -1.f;
		//depthProjectionMatrix[1][1] *= -1.f;
		//glm::mat4 depthProjectionMatrix = glm::perspective(glm::radians(lightFOV), 1.0f, zNear, zFar);
		//glm::mat4 depthViewMatrix = glm::lookAt(transform.m_Position, glm::vec3(0.0f), glm::vec3(0, 1, 0));
		glm::mat4 depthViewMatrix;
		depthViewMatrix = glm::translate(glm::mat4(1.f), transform.m_Position);// *glm::toMat4(camera.GetOrientation());
		depthViewMatrix = glm::inverse(depthViewMatrix);
		glm::mat4 depthModelMatrix = glm::mat4(1.0f);
		UBO_Shadow.view = depthViewMatrix;
		UBO_Shadow.proj = depthProjectionMatrix;

		for (const auto& entity : ECSManager::Instance().GetEntities<DirectionalLight>())
		{
			const auto& light = entity->GetComponent<DirectionalLight>();
			ubo.m_LightDirection = glm::vec4(light.Direction, 1.f);
			ubo.m_LightDirectionalColor = light.DirectionalColor;
			ubo.m_LightAmbientColor = light.AmbientColor;
		}

		m_UBOBuffer->SetData(&ubo, sizeof(UBO));
		m_UBOSkybox->SetData(&UBO_SkyBox, sizeof(SkyBoxUBO));
		m_ShadowUBO->SetData(&UBO_Shadow, sizeof(ShadowUBO));
		//m_AnimationBuffer.ProjView = mainCamera.m_ProjectionMatrix * mainCamera.m_ViewMatrix;
		//m_Animation->UpdateAnimations(m_AnimationBuffer, m_L2W);
		//m_AnimationUBO->SetData(&m_AnimationBuffer, sizeof(AnimationUBO));
	}

	void SceneRenderer::EndFrame(bool IsEditorScene)
	{
		uint32_t Index = Engine::GetInstance().GetWindow()->GetSwapChain()->GetCurrentBufferIndex();
		uint32_t ImageIndex = Engine::GetInstance().GetWindow()->GetSwapChain()->GetCurrentImageIndex();

		m_CommandBuffer->Begin();

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

		VkClearValue clearValues[2];
		clearValues[0].depthStencil = { 1.0f, 0 };
		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = m_ShadowRenderPass->GetHandle();
		renderPassInfo.framebuffer = m_ShadowFramebuffer;
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = { m_ShadowMapWidth, m_ShadowMapHeight };
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = clearValues;

		vkCmdBeginRenderPass(m_CommandBuffer->GetInUseCommandBuffer(), &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport2{};
		viewport2.x = 0.0f;
		viewport2.y = 0.0f;
		m_ShadowMapWidth = static_cast<float>(Engine::GetInstance().GetWindow()->GetSwapChain()->GetWidth());
		m_ShadowMapHeight = static_cast<float>(Engine::GetInstance().GetWindow()->GetSwapChain()->GetHeight());
		viewport2.width = m_ShadowMapWidth;
		viewport2.height = m_ShadowMapHeight;
		viewport2.minDepth = 0.0f;
		viewport2.maxDepth = 1.0f;
		vkCmdSetViewport(m_CommandBuffer->GetInUseCommandBuffer(), 0, 1, &viewport2);

		VkRect2D scissor2{};
		scissor2.extent = { m_ShadowMapWidth, m_ShadowMapHeight };
		vkCmdSetScissor(m_CommandBuffer->GetInUseCommandBuffer(), 0, 1, &scissor2);

		vkCmdSetDepthBias(m_CommandBuffer->GetInUseCommandBuffer(), depthBiasConstant, 0.0f, depthBiasSlope);

		//Shadow Pass
		Renderer::BindPipeline(m_CommandBuffer, m_ShadowPipeline);
		//vkCmdBindPipeline(m_CommandBuffer->GetInUseCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_ShadowPipeline->GetPipeline());
		m_ShadowMaterial->UpdateForEditorSceneRendering(m_ShadowUBO, Index, m_ShadowDescriptInfo);
		vkCmdBindDescriptorSets(m_CommandBuffer->GetInUseCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_ShadowPipeline->GetPipelineLayout(), 0, 1, &m_ShadowMaterial->GetEditorDescriptor(Index), 0, NULL);
		for (const auto& go_mr : materialSort)
		{
			const MeshRenderer& mr = go_mr.second->GetComponent<MeshRenderer>();
			ResourceHandle currentMaterialHandle = go_mr.first;

			PushConstant pc{};
			pc.m_Model = go_mr.second->GetComponent<Transform>().m_WorldXform;
			vkCmdPushConstants(m_CommandBuffer->GetInUseCommandBuffer(), m_ShadowPipeline->GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstant), &pc);

			mr.m_RenderObject->Bind(m_CommandBuffer->GetInUseCommandBuffer());
			mr.m_RenderObject->Draw(m_CommandBuffer->GetInUseCommandBuffer());

			m_PreviousMaterialHandle = currentMaterialHandle;
		}

		vkCmdEndRenderPass(m_CommandBuffer->GetInUseCommandBuffer());

		m_ShadowDescriptInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
		m_ShadowDescriptInfo.imageView = m_Depth.imageview;
		m_ShadowDescriptInfo.sampler = m_Depth.sampler;

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

		//Geom Pass
		Renderer::BindPipeline(m_CommandBuffer, m_Pipeline);
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
						m_DefaultPBRMaterial->UpdateForEditorSceneRendering(m_UBOBuffer, Index, m_ShadowDescriptInfo);
						vkCmdBindDescriptorSets(m_CommandBuffer->GetInUseCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline->GetPipelineLayout(), 0, 1, &m_DefaultPBRMaterial->GetEditorDescriptor(Index), 0, NULL);
					}
					else
					{
						m_DefaultPBRMaterial->UpdateForRendering(m_UBOBuffer, Index, m_ShadowDescriptInfo);
						vkCmdBindDescriptorSets(m_CommandBuffer->GetInUseCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline->GetPipelineLayout(), 0, 1, &m_DefaultPBRMaterial->GetDescriptor(Index), 0, NULL);
					}
				}
				else
				{
					if (IsEditorScene)
					{
						mr.m_MaterialInstance->UpdateForEditorSceneRendering(m_UBOBuffer, Index, m_ShadowDescriptInfo);
						vkCmdBindDescriptorSets(m_CommandBuffer->GetInUseCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline->GetPipelineLayout(), 0, 1, &mr.m_MaterialInstance->GetEditorDescriptor(Index), 0, NULL);
					}
					else
					{
						mr.m_MaterialInstance->UpdateForRendering(m_UBOBuffer, Index, m_ShadowDescriptInfo);
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
			Renderer::BindPipeline(m_CommandBuffer, m_SkyboxPipeline);
			if (IsEditorScene)
			{
				m_SkyboxMaterial->UpdateForEditorSceneRendering(m_UBOSkybox, Index, m_ShadowDescriptInfo);
				vkCmdBindDescriptorSets(m_CommandBuffer->GetInUseCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_SkyboxPipeline->GetPipelineLayout(), 0, 1, &m_SkyboxMaterial->GetEditorDescriptor(Index), 0, NULL);
			}
			else
			{
				m_SkyboxMaterial->UpdateForRendering(m_UBOSkybox, Index, m_ShadowDescriptInfo);
				vkCmdBindDescriptorSets(m_CommandBuffer->GetInUseCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_SkyboxPipeline->GetPipelineLayout(), 0, 1, &m_SkyboxMaterial->GetDescriptor(Index), 0, NULL);
			}

			VkBuffer vertexBuffers[] = { m_SkyboxVertexBuffer->GetBuffer() };
			VkDeviceSize offsets[] = { 0 };
			vkCmdBindVertexBuffers(m_CommandBuffer->GetInUseCommandBuffer(), 0, 1, vertexBuffers, offsets);

			vkCmdBindIndexBuffer(m_CommandBuffer->GetInUseCommandBuffer(), m_SkyboxIndexBuffer->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);

			vkCmdDrawIndexed(m_CommandBuffer->GetInUseCommandBuffer(), m_SkyboxIndexBuffer->GetIndexCount(), 1, 0, 0, 0);
		}

		//Animation Pass
		{
			//m_Animation->BindPipeline(m_CommandBuffer->GetInUseCommandBuffer());
			//m_Animation->UpdateMaterial(m_AnimationUBO, Index);

			//vkCmdBindDescriptorSets(m_CommandBuffer->GetInUseCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_Animation->GetPipelineLayout(), 0, 1, &m_Animation->GetDescriptorSet(Index), 0, NULL);
			//m_Animation->BindBuffers(m_CommandBuffer->GetInUseCommandBuffer());
			//m_Animation->Draw(m_CommandBuffer->GetInUseCommandBuffer());
		}

		Renderer::EndRenderPass(m_CommandBuffer);

		m_CommandBuffer->End();
		m_CommandBuffer->Submit();
	}

	void SceneRenderer::DebugDrawPass(uint32_t Index) //Debug Pass
	{
		Renderer::BindPipeline(m_CommandBuffer, m_DebugRenderer->GetPipeline());
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
			model = model * glm::mat4_cast(glm::quat(glm::radians(tr.m_Rotation)));
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
			const Transform& tr = capsule->GetComponent<Transform>();
			const CapsuleCollider& cc = capsule->GetComponent<CapsuleCollider>();
			if (cc.m_IsVisible == false)
				continue;
			
			for (int i = 0; i < 2; ++i)
			{
				for (int j = 0; j < 2; ++j)
				{
					PushConstant pc{};
					glm::mat4 model(1.f);
					const float radius = cc.m_Radius;
					const float halfExtent = cc.m_HalfHeight;
					if (i == 0)
						model = glm::translate(model, tr.m_Position + cc.m_Offset + glm::vec3(0, halfExtent, 0));
					else
						model = glm::translate(model, tr.m_Position + cc.m_Offset + glm::vec3(0, -halfExtent, 0));
					model = model * glm::mat4_cast(glm::quat(glm::radians(tr.m_Rotation)));
					model = glm::rotate(model, glm::radians(180.f * i), glm::vec3(1, 0, 0));
					model = glm::rotate(model, glm::radians(90.f * j), glm::vec3(0, 1, 0));
					model = glm::scale(model, glm::vec3(radius, radius, radius));
					pc.m_Model = model;
					vkCmdPushConstants(m_CommandBuffer->GetInUseCommandBuffer(), m_DebugRenderer->GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstant), &pc);

					//Bind
					vkCmdBindDescriptorSets(m_CommandBuffer->GetInUseCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_DebugRenderer->GetPipelineLayout(), 0, 1, &m_DebugRenderer->GetDescriptor(Index), 0, NULL);

					m_DebugRenderer->BindDebugCapsuleRadius(m_CommandBuffer->GetInUseCommandBuffer());
					m_DebugRenderer->DrawDebugCapsuleRadius(m_CommandBuffer->GetInUseCommandBuffer());

					PushConstant pc2{};
					glm::mat4 model2(1.f);
					model2 = glm::translate(model2, tr.m_Position + cc.m_Offset);
					model2 = model2 * glm::mat4_cast(glm::quat(glm::radians(tr.m_Rotation)));
					model2 = glm::rotate(model2, glm::radians(180.f * i), glm::vec3(1, 0, 0));
					model2 = glm::rotate(model2, glm::radians(90.f * j), glm::vec3(0, 1, 0));
					model2 = glm::scale(model2, glm::vec3(radius, halfExtent * 2.f, radius));
					pc2.m_Model = model2;
					vkCmdPushConstants(m_CommandBuffer->GetInUseCommandBuffer(), m_DebugRenderer->GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstant), &pc2);

					//Bind
					vkCmdBindDescriptorSets(m_CommandBuffer->GetInUseCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_DebugRenderer->GetPipelineLayout(), 0, 1, &m_DebugRenderer->GetDescriptor(Index), 0, NULL);

					m_DebugRenderer->BindDebugCapsuleHalfExtent(m_CommandBuffer->GetInUseCommandBuffer());
					m_DebugRenderer->DrawDebugCapsuleHalfExtent(m_CommandBuffer->GetInUseCommandBuffer());
				}
			}
		}

		for (const auto& camera : ECSManager::Instance().GetEntities<Camera>())
		{
			const Transform& tr = camera->GetComponent<Transform>();
			// const Camera& cc = camera->GetComponent<Camera>();

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

		for (const auto& directionalLight : ECSManager::Instance().GetEntities<DirectionalLight>())
		{
			const Transform& tr = directionalLight->GetComponent<Transform>();
			// const DirectionalLight& dl = directionalLight->GetComponent<DirectionalLight>();
			
			PushConstant pc{};
			glm::mat4 model(1.f);
			const float scale = 10.f;
			model = glm::translate(model, tr.m_Position);
			model = model * glm::mat4_cast(glm::quat(glm::radians(glm::vec3(-tr.m_Rotation.x, tr.m_Rotation.y, tr.m_Rotation.z))));
			model = glm::scale(model, glm::vec3(scale, scale, scale));
			pc.m_Model = model;

			vkCmdPushConstants(m_CommandBuffer->GetInUseCommandBuffer(), m_DebugRenderer->GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstant), &pc);
			vkCmdBindDescriptorSets(m_CommandBuffer->GetInUseCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_DebugRenderer->GetPipelineLayout(), 0, 1, &m_DebugRenderer->GetDescriptor(Index), 0, NULL);

			m_DebugRenderer->BindDebugDirectionalLight(m_CommandBuffer->GetInUseCommandBuffer());
			m_DebugRenderer->DrawDebugDirectionalLight(m_CommandBuffer->GetInUseCommandBuffer());
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

		CubeMapConfig CubeConfig{};
		CubeConfig.Filter = VK_FILTER_NEAREST;
		CubeConfig.Format = Texture1->GetFormat();
		CubeConfig.Height = Texture1->GetHeight();
		CubeConfig.Width = Texture1->GetWidth();
		CubeConfig.SamplerAddressMode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		CubeConfig.Textures = { Texture4, Texture2, Texture6, Texture5, Texture1, Texture3 };

		m_SkyboxTexture = std::make_shared<VulkanTexture>(CubeConfig);
		
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

		m_SkyboxVertexBuffer = std::make_unique<VertexBuffer>(static_cast<void*>(vertices.data()),
			UINT32_T_CAST(vertices.size() * sizeof(vertices[0])));

		m_SkyboxIndexBuffer = std::make_unique<IndexBuffer>(static_cast<void*>(indices.data()),
			UINT32_T_CAST(indices.size() * sizeof(uint32_t)),
			UINT32_T_CAST(indices.size()));

		m_SkyboxMaterial = std::make_unique<Material>(m_SkyboxPipeline->GetConfig().Shader);
		m_SkyboxMaterial->Invalidate();
		m_SkyboxMaterial->SetTexture("SamplerCubeMap", m_SkyboxTexture);
	}

	void SceneRenderer::ShadowPassInit()
	{
		auto SC = Engine::GetInstance().GetWindow()->GetSwapChain();

		m_ShadowMapWidth = SC->GetWidth();
		m_ShadowMapHeight = SC->GetHeight();

		VkImageCreateInfo imageCreateInfo{};
		imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
		imageCreateInfo.extent.width = m_ShadowMapWidth;
		imageCreateInfo.extent.height = m_ShadowMapHeight;
		imageCreateInfo.extent.depth = 1;
		imageCreateInfo.mipLevels = 1;
		imageCreateInfo.arrayLayers = 1;
		imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageCreateInfo.format = VK_FORMAT_D16_UNORM;
		imageCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		if (auto Result = vkCreateImage(m_Device->GetLogicalDevice(), &imageCreateInfo, nullptr, &m_Depth.image); Result != VK_SUCCESS)
		{
			assert(Result == VK_SUCCESS && "Unable to create image for shadow");
		}

		VkMemoryAllocateInfo memAllocInfo{};
		memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		VkMemoryRequirements memReqs;
		vkGetImageMemoryRequirements(m_Device->GetLogicalDevice(), m_Depth.image, &memReqs);
		memAllocInfo.allocationSize = memReqs.size;
		memAllocInfo.memoryTypeIndex = m_Device->FindMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		if (auto Result = vkAllocateMemory(m_Device->GetLogicalDevice(), &memAllocInfo, nullptr, &m_Depth.devicememory); Result != VK_SUCCESS)
		{
			assert(Result == VK_SUCCESS && "Unable to allocate memory for shadow image");
		}

		if (auto Result = vkBindImageMemory(m_Device->GetLogicalDevice(), m_Depth.image, m_Depth.devicememory, 0); Result != VK_SUCCESS)
		{
			assert(Result == VK_SUCCESS && "Unable to bind memory for shadow image");
		}

		VkImageViewCreateInfo imageViewCreateInfo{};
		imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageViewCreateInfo.format = VK_FORMAT_D16_UNORM;
		imageViewCreateInfo.subresourceRange = {};
		imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
		imageViewCreateInfo.subresourceRange.levelCount = 1;
		imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
		imageViewCreateInfo.subresourceRange.layerCount = 1;
		imageViewCreateInfo.image = m_Depth.image;
		if (auto Result = vkCreateImageView(m_Device->GetLogicalDevice(), &imageViewCreateInfo, nullptr, &m_Depth.imageview); Result != VK_SUCCESS)
		{
			assert(Result == VK_SUCCESS && "Unable to create image view for shadow");
		}

		VkFilter shadowmap_filter = vkUtils::formatIsFilterable(m_Device->GetPhysicalDevice()->GetPhysicalDevice(), VK_FORMAT_D16_UNORM, VK_IMAGE_TILING_OPTIMAL) ? VK_FILTER_LINEAR : VK_FILTER_NEAREST;
		VkSamplerCreateInfo samplerCreateInfo{};
		samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerCreateInfo.maxAnisotropy = 1.0f;
		samplerCreateInfo.magFilter = shadowmap_filter;
		samplerCreateInfo.minFilter = shadowmap_filter;
		samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerCreateInfo.addressModeV = samplerCreateInfo.addressModeU;
		samplerCreateInfo.addressModeW = samplerCreateInfo.addressModeU;
		samplerCreateInfo.mipLodBias = 0.0f;
		samplerCreateInfo.maxAnisotropy = 1.0f;
		samplerCreateInfo.minLod = 0.0f;
		samplerCreateInfo.maxLod = 1.0f;
		samplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
		if (auto Result = vkCreateSampler(m_Device->GetLogicalDevice(), &samplerCreateInfo, nullptr, &m_Depth.sampler); Result != VK_SUCCESS)
		{
			assert(Result == VK_SUCCESS && "Unable to create image sampler for shadow");
		}

		m_ShadowRenderPass = std::make_shared<RenderPass>(m_Device, true);

		VkFramebufferCreateInfo framebufferCreateInfo{};
		framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferCreateInfo.renderPass = m_ShadowRenderPass->GetHandle();
		framebufferCreateInfo.attachmentCount = 1;
		framebufferCreateInfo.pAttachments = &m_Depth.imageview;
		framebufferCreateInfo.width = m_ShadowMapWidth;
		framebufferCreateInfo.height = m_ShadowMapHeight;
		framebufferCreateInfo.layers = 1;

		if (auto Result = vkCreateFramebuffer(m_Device->GetLogicalDevice(), &framebufferCreateInfo, nullptr, &m_ShadowFramebuffer); Result != VK_SUCCESS)
		{
			assert(Result == VK_SUCCESS && "Unable to create image sampler for shadow");
		}
	}
}