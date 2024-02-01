#include "pch.h"
#include "Core/Engine.h"
#include "Core/Transform.h"
#include "SceneRenderer.h"
#include "RendererContext.h"
#include "MeshRenderer.h"
#include "Camera.h"
#include "Core/Logger.h"
#include "VulkanTexture.h"
#include "Resource/ResourceManager.h"
#include "Physics/SphereCollider.h"
#include "Physics/BoxCollider.h"
#include "Physics/CapsuleCollider.h"
#include "Physics/CylinderCollider.h"
#include "Light.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "glm/gtx/quaternion.hpp"
#include "VulkanUtilities.h"
#include "AnimationComponent.h"
#include "FontRenderer.h"

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

	SceneRenderer::SceneRenderer(bool IsEditorScene) : m_IsEditorScene(IsEditorScene)
	{
		m_Device = RendererContext::GetDevice();

		Create();

		m_CommandBuffer = std::make_shared<CommandBuffer>("SceneRendererCommmandBuffer");
		m_DescriptorPool = DescriptorPool::Builder().SetPoolFlags(VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT).SetMaxSets(5000).AddPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10000).AddPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 10000).Build();
		m_UBOBuffer = std::make_shared<UniformBuffer>(UINT32_T_CAST(sizeof(UBO)), 0);
		m_UBOSkybox = std::make_shared<UniformBuffer>(UINT32_T_CAST(sizeof(SkyBoxUBO)), 0);
		m_ShadowUBO = std::make_shared<UniformBuffer>(UINT32_T_CAST(sizeof(ShadowUBO)), 0);
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

		SkyBoxPassInit();
		ShadowPassInit();

		PipelineConfigurations Config{};
		Config.Primitive = PrimitiveType::Triangles;
		Config.Shader = ResourceManager::Instance().GetResource<Shader>(5);
		Config.CullMode = VK_CULL_MODE_FRONT_BIT;
		m_ShadowPipeline = std::make_shared<Pipeline>(Config, m_ShadowRenderPass);

		PipelineConfigurations ShadowAnimationPipelineConfig{};
		ShadowAnimationPipelineConfig.Primitive = PrimitiveType::Triangles;
		ShadowAnimationPipelineConfig.Shader = ResourceManager::Instance().GetResource<Shader>(10);
		ShadowAnimationPipelineConfig.CullMode = VK_CULL_MODE_NONE;// VK_CULL_MODE_BACK_BIT;// VK_CULL_MODE_FRONT_BIT;
		ShadowAnimationPipelineConfig.UseAutoShaderVertexInput = false;
		ShadowAnimationPipelineConfig.CustomVertexBufferInputLayout =
		{
			{ { VertexInputDataType::Vec3, VertexInputDataType::Vec3, VertexInputDataType::Vec3, VertexInputDataType::Vec3, VertexInputDataType::Vec3, VertexInputDataType::Vec2 }, 0},
			{ { VertexInputDataType::Vec4, VertexInputDataType::IVec4 }, 1 }
		};

		m_ShadowAnimationPipeline = std::make_shared<Pipeline>(ShadowAnimationPipelineConfig, m_ShadowRenderPass);

		m_ShadowMaterial = std::make_shared<Material>(ResourceManager::Instance().GetResource<Shader>(5));
		m_ShadowMaterial->Invalidate();

		m_UIRenderer = std::make_shared<UIRenderer>(m_Device);
		m_ParticleRenderer = std::make_shared<ParticleRenderer>(m_Device);

		PostProcessingManager::Instance().Init();

		PipelineConfigurations AnimationPipelineConfig{};
		AnimationPipelineConfig.Primitive = PrimitiveType::Triangles;
		AnimationPipelineConfig.Shader = ResourceManager::Instance().GetResource<Shader>(9);
		AnimationPipelineConfig.UseAutoShaderVertexInput = false;
		AnimationPipelineConfig.CustomVertexBufferInputLayout =
		{
			{ { VertexInputDataType::Vec3, VertexInputDataType::Vec3, VertexInputDataType::Vec3, VertexInputDataType::Vec3, VertexInputDataType::Vec3, VertexInputDataType::Vec2 }, 0},
			{ { VertexInputDataType::Vec4, VertexInputDataType::IVec4 }, 1 }
		};
		m_AnimationPipeline = std::make_shared<Pipeline>(AnimationPipelineConfig, m_RenderPass);
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
		PostProcessingManager::Instance().Shutdown();

		auto Device = m_Device->GetLogicalDevice();

		vkDeviceWaitIdle(Device);

		for (int x = 0; x < m_ColorImages.size(); x++)
		{
			vkDestroyFramebuffer(Device, m_FrameBuffer[x], nullptr);
		}

		m_ColorImages.clear();
		m_DepthImages.clear();

		vkDestroyFramebuffer(m_Device->GetLogicalDevice(), m_ShadowFramebuffer, nullptr);
	}

	void SceneRenderer::BeginEditorFrame()
	{
		const EditorCamera& editorCamera = EditorCamera::Instance();
		const BaseCamera& baseCamera = editorCamera.m_BaseCamera;
		const Transform& transform = ECSSystemManager::Instance().GetSystem<CameraSystem>()->GetMainCamera()->GetComponent<Transform>();
		
		UBO ubo{};
		ubo.m_ProjView = editorCamera.GetViewProjectionMatrix();
		ubo.m_LightPosition = transform.m_Position;
		ubo.m_CameraPosition = glm::vec4(transform.m_Position, 1.f);

		SkyBoxUBO UBO_SkyBox;
		UBO_SkyBox.Proj = editorCamera.GetProjectionMatrix();
		UBO_SkyBox.View = editorCamera.GetViewMatrix();

		glm::mat4 depthViewMatrix(1.f);
		bool recalculateShadowFrustum = ShadowFrustumCheck(baseCamera);
		for (const auto& entity : ECSManager::Instance().GetEntities<DirectionalLight>())
		{
			const auto& lightTransform = entity->GetComponent<Transform>();
			const auto& light = entity->GetComponent<DirectionalLight>();
			//std::cout << std::fixed  << "Light Direction: " << light.m_Direction.x << ", " << light.m_Direction.y << ", " << light.m_Direction.z << std::endl;
			ubo.m_LightDirection = glm::vec4(light.m_Direction, 1.f);
			ubo.m_LightDirectionalColor = light.m_DirectionalColor;
			ubo.m_LightAmbientColor = light.m_AmbientColor;
			
			if (recalculateShadowFrustum)
			{
				RecreateShadowAABB(baseCamera.GetFrustumCorners(false, m_EditorShadowRatio));
				depthViewMatrix = glm::translate(glm::mat4(1.f), m_EditorShadowRenderPoint) * glm::toMat4(glm::quat(glm::radians(-lightTransform.m_Rotation)));
			}
		}

		if (recalculateShadowFrustum)
		{
			ShadowUBO UBO_Shadow;
			glm::mat4 depthProjectionMatrix;
			const float deltaX = m_EditorShadowAABBMax.x - m_EditorShadowAABBMin.x;
			const float deltaY = m_EditorShadowAABBMax.y - m_EditorShadowAABBMin.y;
			const float deltaZ = m_EditorShadowAABBMax.z - m_EditorShadowAABBMin.z;
			const float orthoLength = deltaX;
			const float orthoHeight = deltaY;
			const float orthoNear = 0.1f;
			const float orthoFar = orthoNear + deltaZ;

			depthProjectionMatrix = glm::mat4(1.f);
			depthProjectionMatrix[0][0] = -2.f / (orthoLength - -orthoLength);
			depthProjectionMatrix[1][1] = -2.f / (orthoHeight - -orthoHeight);
			depthProjectionMatrix[2][2] = 2.f / (orthoFar - orthoNear);
			depthProjectionMatrix[3][0] = -(orthoLength + -orthoLength) / (orthoLength - -orthoLength);
			depthProjectionMatrix[3][1] = -(orthoHeight + -orthoHeight) / (orthoHeight - -orthoHeight);
			depthProjectionMatrix[3][2] = -(orthoNear) / (orthoFar - orthoNear);

			depthViewMatrix = glm::inverse(depthViewMatrix);
			m_EditorShadowView = depthViewMatrix;
			m_EditorShadowProj = depthProjectionMatrix;

			UBO_Shadow.view = m_EditorShadowView;
			UBO_Shadow.proj = m_EditorShadowProj;

			m_ShadowUBO->SetData(&UBO_Shadow, sizeof(ShadowUBO));
		}
		
		ubo.m_LightSpaceMatrix = m_EditorShadowProj * m_EditorShadowView;

		m_UBOBuffer->SetData(&ubo, sizeof(UBO));
		m_UBOSkybox->SetData(&UBO_SkyBox, sizeof(SkyBoxUBO));
	}

	void SceneRenderer::BeginFrame()
	{
		//UBO
		const Entity& mainCamera = ECSSystemManager::Instance().GetSystem<CameraSystem>()->GetMainCamera();
		const Camera& cameraComponent = mainCamera->GetComponent<Camera>();
		const BaseCamera& baseCamera = cameraComponent.m_BaseCamera;
		const Transform& cameraTransform = mainCamera->GetComponent<Transform>();

		UBO ubo{};
		ubo.m_ProjView = baseCamera.m_ProjectionMatrix * baseCamera.m_ViewMatrix;
		ubo.m_LightPosition = cameraTransform.m_Position;
		ubo.m_CameraPosition = glm::vec4(cameraTransform.m_Position, 1.f);
		
		SkyBoxUBO UBO_SkyBox;
		UBO_SkyBox.Proj = baseCamera.m_ProjectionMatrix;
		UBO_SkyBox.View = baseCamera.m_ViewMatrix;

		glm::mat4 depthViewMatrix(1.f);
		bool recalculateShadowFrustum = ShadowFrustumCheck(baseCamera);
		recalculateShadowFrustum = true;
		for (const auto& entityDirectional : ECSManager::Instance().GetEntities<DirectionalLight>())
		{
			const auto& lightTransform = entityDirectional->GetComponent<Transform>();
			const auto& light = entityDirectional->GetComponent<DirectionalLight>();
			ubo.m_LightDirection = glm::vec4(light.m_Direction, 1.f);
			ubo.m_LightDirectionalColor = light.m_DirectionalColor;
			ubo.m_LightAmbientColor = light.m_AmbientColor;
			ubo.m_ShadowIntensity = light.m_ShadowIntensity;

			if (recalculateShadowFrustum)
			{
				//RecreateShadowAABB(baseCamera.GetFrustumCorners(false, 0.033f));
				RecreateShadowAABB(baseCamera.GetFrustumCorners(false, 0.1f));
				m_ShadowRenderPoint.y = lightTransform.m_Position.y;
				glm::vec3 tempRotation = glm::radians(lightTransform.m_Rotation);
				glm::mat4 rotationMat = glm::toMat4(glm::quat(tempRotation));
				depthViewMatrix = glm::translate(glm::mat4(1.f), m_ShadowRenderPoint) * rotationMat;
			}
		}

		if (recalculateShadowFrustum)
		{
			ShadowUBO UBO_Shadow{};
			glm::mat4 depthProjectionMatrix;
			const float deltaX = m_ShadowAABBMax.x - m_ShadowAABBMin.x;
			const float deltaY = m_ShadowAABBMax.y - m_ShadowAABBMin.y;
			const float deltaZ = m_ShadowAABBMax.z - m_ShadowAABBMin.z;
			const float orthoLength = deltaX;
			const float orthoHeight = deltaY;
			const float orthoNear = 0.1f;
			const float orthoFar = orthoNear + deltaZ;

			depthProjectionMatrix = glm::mat4(1.f);
			depthProjectionMatrix[0][0] = -2.f / (orthoLength - -orthoLength);
			depthProjectionMatrix[1][1] = -2.f / (orthoHeight - -orthoHeight);
			depthProjectionMatrix[2][2] = 2.f / (orthoFar - orthoNear);
			depthProjectionMatrix[3][0] = -(orthoLength + -orthoLength) / (orthoLength - -orthoLength);
			depthProjectionMatrix[3][1] = -(orthoHeight + -orthoHeight) / (orthoHeight - -orthoHeight);
			depthProjectionMatrix[3][2] = -(orthoNear) / (orthoFar - orthoNear);

			depthViewMatrix = glm::inverse(depthViewMatrix);
			m_ShadowView = depthViewMatrix;
			m_ShadowProj = depthProjectionMatrix;

			UBO_Shadow.view = m_ShadowView;
			UBO_Shadow.proj = m_ShadowProj;

			m_ShadowUBO->SetData(&UBO_Shadow, sizeof(ShadowUBO));
		}

		ubo.m_LightSpaceMatrix = m_ShadowProj * m_ShadowView;
		
		m_UBOBuffer->SetData(&ubo, sizeof(UBO));
		m_UBOSkybox->SetData(&UBO_SkyBox, sizeof(SkyBoxUBO));
	}

	void SceneRenderer::EndFrame()
	{
		for (const auto& Entity : ECSManager::Instance().GetEntities<MeshRenderer, AnimationComponent>())
		{
			const auto& TransformComp = Entity->GetComponent<Transform>();
			auto& MRComp = Entity->GetComponent<MeshRenderer>();
			auto& AnimComp = Entity->GetComponent<AnimationComponent>();
			if (!MRComp.m_IsVisible)
				continue;

			if (AnimComp.m_IsAnimating)
			{
				AnimComp.m_FPS = AnimComp.m_FPS > 60 ? 60 : AnimComp.m_FPS;
				MRComp.m_RenderObject->UpdateAnimation(AnimComp.m_BufferData.L2W, TransformComp.m_WorldXform, AnimComp.m_FPS, AnimComp.m_AnimationSpeed);
			}

			AnimComp.m_UBO->SetData(&AnimComp.m_BufferData, sizeof(AnimationUBO));
		}

		uint32_t Index = Engine::GetInstance().GetWindow()->GetSwapChain()->GetCurrentBufferIndex();
		uint32_t ImageIndex = Engine::GetInstance().GetWindow()->GetSwapChain()->GetCurrentImageIndex();

		std::multimap<ResourceHandle, Entity> materialSort;
		for (const auto& go_mr : ECSManager::Instance().GetEntities<MeshRenderer>())
		{
			const MeshRenderer& mr = go_mr->GetComponent<MeshRenderer>();
			if (go_mr->HasComponent<AnimationComponent>() && mr.m_RenderObject->IsRigged())
				continue;

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

		m_CommandBuffer->Begin();

		ShadowPass(Index, materialSort);

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

		GeometryPass(Index, materialSort);
		GeometryAnimationPass(Index, materialSort);
		DebugDrawPass(Index);
		SkyBoxPass(Index);

		Renderer::EndRenderPass(m_CommandBuffer);

		if (m_IsEditorScene == false)
		{
			m_UIRenderer->Render(m_FrameBuffer[ImageIndex], m_CommandBuffer, m_IsEditorScene);
			m_ParticleRenderer->Render(m_FrameBuffer[ImageIndex], m_CommandBuffer, m_IsEditorScene);
			FontRenderer::GetInstance()->RenderFont(m_FrameBuffer[ImageIndex], m_CommandBuffer);
			PostProcessingManager::Instance().Render(m_FrameBuffer[ImageIndex], m_CommandBuffer, Index);
		}

		m_CommandBuffer->End();
		m_CommandBuffer->Submit();
	}

	void SceneRenderer::GeometryPass(uint32_t Index, const std::multimap<ResourceHandle, Entity>& MaterialSort)
	{
		Renderer::BindPipeline(m_CommandBuffer, m_Pipeline);
		for (const auto& go_mr : MaterialSort)
		{
			if (go_mr.second->HasComponent<AnimationComponent>()) //This only render static meshes
			{
				continue;
			}

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
					if (m_IsEditorScene)
					{
						m_DefaultPBRMaterial->UpdateForEditorSceneRendering(m_UBOBuffer, Index, m_ShadowImages->GetDescriptorImageInfo());
						vkCmdBindDescriptorSets(m_CommandBuffer->GetInUseCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline->GetPipelineLayout(), 0, 1, &m_DefaultPBRMaterial->GetEditorDescriptor(Index), 0, NULL);
					}
					else
					{
						m_DefaultPBRMaterial->UpdateForRendering(m_UBOBuffer, Index, m_ShadowImages->GetDescriptorImageInfo());
						vkCmdBindDescriptorSets(m_CommandBuffer->GetInUseCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline->GetPipelineLayout(), 0, 1, &m_DefaultPBRMaterial->GetDescriptor(Index), 0, NULL);
					}
				}
				else
				{
					if (m_IsEditorScene)
					{
						mr.m_MaterialInstance->UpdateForEditorSceneRendering(m_UBOBuffer, Index, m_ShadowImages->GetDescriptorImageInfo());
						vkCmdBindDescriptorSets(m_CommandBuffer->GetInUseCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline->GetPipelineLayout(), 0, 1, &mr.m_MaterialInstance->GetEditorDescriptor(Index), 0, NULL);
					}
					else
					{
						mr.m_MaterialInstance->UpdateForRendering(m_UBOBuffer, Index, m_ShadowImages->GetDescriptorImageInfo());
						vkCmdBindDescriptorSets(m_CommandBuffer->GetInUseCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline->GetPipelineLayout(), 0, 1, &mr.m_MaterialInstance->GetDescriptor(Index), 0, NULL);
					}
				}
			}

			mr.m_RenderObject->Bind(m_CommandBuffer->GetInUseCommandBuffer());
			mr.m_RenderObject->Draw(m_CommandBuffer->GetInUseCommandBuffer());

			m_PreviousMaterialHandle = currentMaterialHandle;
		}

		m_PreviousMaterialHandle = 0;
	}

	void SceneRenderer::GeometryAnimationPass(uint32_t Index, const std::multimap<ResourceHandle, Entity>& MaterialSort)
	{
		(void)MaterialSort;
		Renderer::BindPipeline(m_CommandBuffer, m_AnimationPipeline);
		for (const auto& Entity : ECSManager::Instance().GetEntities<AnimationComponent, MeshRenderer>())
		{
			MeshRenderer& MeshRendererComp = Entity->GetComponent<MeshRenderer>();
			if (!MeshRendererComp.m_IsVisible)
				continue;

			AnimationComponent& AnimationComp = Entity->GetComponent<AnimationComponent>();

			PushConstant pc{};
			pc.m_Model = Entity->GetComponent<Transform>().m_WorldXform;
			vkCmdPushConstants(m_CommandBuffer->GetInUseCommandBuffer(), m_AnimationPipeline->GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstant), &pc);

			if (MeshRendererComp.m_AnimationMaterialInstance == nullptr)
			{
				ResourceHandle materialHandle = PBR::GetDefaultAnimationMaterial();
				if (m_DefaultAnimationPBRMaterial == nullptr)
				{
					m_DefaultAnimationPBRMaterial = ResourceManager::Instance().GetResource<Material>(materialHandle);
					m_DefaultAnimationPBRMaterial->Invalidate();
				}

				if (m_IsEditorScene)
				{
					m_DefaultAnimationPBRMaterial->UpdateForEditorAnimationRendering(m_UBOBuffer, Index, AnimationComp.m_UBO, m_ShadowImages->GetDescriptorImageInfo());
					vkCmdBindDescriptorSets(m_CommandBuffer->GetInUseCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_AnimationPipeline->GetPipelineLayout(), 0, 1, &m_DefaultAnimationPBRMaterial->GetEditorDescriptor(Index), 0, NULL);
				}
				else
				{
					m_DefaultAnimationPBRMaterial->UpdateForAnimationRendering(m_UBOBuffer, Index, AnimationComp.m_UBO, m_ShadowImages->GetDescriptorImageInfo());
					vkCmdBindDescriptorSets(m_CommandBuffer->GetInUseCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_AnimationPipeline->GetPipelineLayout(), 0, 1, &m_DefaultAnimationPBRMaterial->GetDescriptor(Index), 0, NULL);
				}
			}
			else
			{
				if (m_IsEditorScene)
				{
					MeshRendererComp.m_AnimationMaterialInstance->UpdateForEditorAnimationRendering(m_UBOBuffer, Index, AnimationComp.m_UBO, m_ShadowImages->GetDescriptorImageInfo());
					vkCmdBindDescriptorSets(m_CommandBuffer->GetInUseCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_AnimationPipeline->GetPipelineLayout(), 0, 1, &MeshRendererComp.m_AnimationMaterialInstance->GetEditorDescriptor(Index), 0, NULL);
				}
				else
				{
					MeshRendererComp.m_AnimationMaterialInstance->UpdateForAnimationRendering(m_UBOBuffer, Index, AnimationComp.m_UBO, m_ShadowImages->GetDescriptorImageInfo());
					vkCmdBindDescriptorSets(m_CommandBuffer->GetInUseCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_AnimationPipeline->GetPipelineLayout(), 0, 1, &MeshRendererComp.m_AnimationMaterialInstance->GetDescriptor(Index), 0, NULL);
				}
			}

			MeshRendererComp.m_RenderObject->BindAnimation(m_CommandBuffer->GetInUseCommandBuffer());
		}
	}

	void SceneRenderer::SkyBoxPass(uint32_t Index)
	{
		Renderer::BindPipeline(m_CommandBuffer, m_SkyboxPipeline);
		if (m_IsEditorScene)
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

	void SceneRenderer::ShadowPass(uint32_t Index, const std::multimap<ResourceHandle, Entity>& MaterialSort)
	{
		m_ShadowMapWidth = 8192;
		m_ShadowMapHeight = 8192;
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
		viewport2.width = (float)m_ShadowMapWidth;
		viewport2.height = (float)m_ShadowMapHeight;
		viewport2.minDepth = 0.0f;
		viewport2.maxDepth = 1.0f;
		vkCmdSetViewport(m_CommandBuffer->GetInUseCommandBuffer(), 0, 1, &viewport2);

		VkRect2D scissor2{};
		scissor2.extent = { m_ShadowMapWidth, m_ShadowMapHeight };
		vkCmdSetScissor(m_CommandBuffer->GetInUseCommandBuffer(), 0, 1, &scissor2);

		vkCmdSetDepthBias(m_CommandBuffer->GetInUseCommandBuffer(), depthBiasConstant, 0.0f, depthBiasSlope);

		Renderer::BindPipeline(m_CommandBuffer, m_ShadowPipeline);

		if (m_IsEditorScene)
		{
			m_ShadowMaterial->UpdateForEditorSceneRendering(m_ShadowUBO, Index, m_ShadowImages->GetDescriptorImageInfo());
			vkCmdBindDescriptorSets(m_CommandBuffer->GetInUseCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_ShadowPipeline->GetPipelineLayout(), 0, 1, &m_ShadowMaterial->GetEditorDescriptor(Index), 0, NULL);
		}
		else
		{
			m_ShadowMaterial->UpdateForRendering(m_ShadowUBO, Index, m_ShadowImages->GetDescriptorImageInfo());
			vkCmdBindDescriptorSets(m_CommandBuffer->GetInUseCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_ShadowPipeline->GetPipelineLayout(), 0, 1, &m_ShadowMaterial->GetDescriptor(Index), 0, NULL);
		}

		for (const auto& go_mr : MaterialSort)
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
		m_PreviousMaterialHandle = 0; 

		Renderer::BindPipeline(m_CommandBuffer, m_ShadowAnimationPipeline);

		for (const auto& Entity : ECSManager::Instance().GetEntities<AnimationComponent, MeshRenderer>())
		{
			const MeshRenderer& MeshComp = Entity->GetComponent<MeshRenderer>();
			const AnimationComponent& AnimComp = Entity->GetComponent<AnimationComponent>();

			PushConstant pc{};
			pc.m_Model = Entity->GetComponent<Transform>().m_WorldXform;
			vkCmdPushConstants(m_CommandBuffer->GetInUseCommandBuffer(), m_ShadowAnimationPipeline->GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstant), &pc);

			if (m_IsEditorScene)
			{
				AnimComp.m_ShadowAnimationMaterial->UpdateForEditorAnimationRendering(m_ShadowUBO, Index, AnimComp.m_UBO, m_ShadowImages->GetDescriptorImageInfo());
				vkCmdBindDescriptorSets(m_CommandBuffer->GetInUseCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_ShadowAnimationPipeline->GetPipelineLayout(), 0, 1, &AnimComp.m_ShadowAnimationMaterial->GetEditorDescriptor(Index), 0, NULL);
			}
			else
			{
				AnimComp.m_ShadowAnimationMaterial->UpdateForAnimationRendering(m_ShadowUBO, Index, AnimComp.m_UBO, m_ShadowImages->GetDescriptorImageInfo());
				vkCmdBindDescriptorSets(m_CommandBuffer->GetInUseCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_ShadowAnimationPipeline->GetPipelineLayout(), 0, 1, &AnimComp.m_ShadowAnimationMaterial->GetDescriptor(Index), 0, NULL);
			}

			MeshComp.m_RenderObject->BindAnimation(m_CommandBuffer->GetInUseCommandBuffer());
		}

		Renderer::EndRenderPass(m_CommandBuffer);
	}

	void SceneRenderer::DebugDrawPass(uint32_t Index) //Debug Pass
	{
		if (m_IsEditorScene)
		{
			Renderer::BindPipeline(m_CommandBuffer, m_DebugRenderer->GetPipeline());
			m_DebugRenderer->UpdateMaterial(m_UBOBuffer, Index);
			vkCmdBindDescriptorSets(m_CommandBuffer->GetInUseCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_DebugRenderer->GetPipelineLayout(), 0, 1, &m_DebugRenderer->GetDescriptor(Index), 0, NULL);

			for (const auto& spheres : ECSManager::Instance().GetEntities<SphereCollider>())
			{
				const Transform& tr = spheres->GetComponent<Transform>();
				const SphereCollider& sc = spheres->GetComponent<SphereCollider>();
				if (sc.m_IsVisible == false)
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
				const CapsuleCollider& cpc = capsule->GetComponent<CapsuleCollider>();
				if (cpc.m_IsVisible == false)
					continue;

				for (int i = 0; i < 2; ++i)
				{
					for (int j = 0; j < 2; ++j)
					{
						PushConstant pc{};
						glm::mat4 model(1.f);
						const float radius = cpc.m_Radius;
						const float halfExtent = cpc.m_HalfHeight;
						if (i == 0)
							model = glm::translate(model, tr.m_Position + cpc.m_Offset + glm::vec3(0, halfExtent, 0));
						else
							model = glm::translate(model, tr.m_Position + cpc.m_Offset + glm::vec3(0, -halfExtent, 0));
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
						model2 = glm::translate(model2, tr.m_Position + cpc.m_Offset);
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

			for (const auto& cylinder : ECSManager::Instance().GetEntities<CylinderCollider>())
			{
				const Transform& tr = cylinder->GetComponent<Transform>();
				const CylinderCollider& cyc = cylinder->GetComponent<CylinderCollider>();
				if (cyc.m_IsVisible == false)
					continue;

				; // debug draw for cylinders? :P
			}

			for (const auto& camera : ECSManager::Instance().GetEntities<Camera>())
			{
				const Transform& tr = camera->GetComponent<Transform>();
				const Camera& cc = camera->GetComponent<Camera>();

				PushConstant pc{};
				glm::mat4 model(1.f);
				const float scale = cc.m_BaseCamera.m_Far - cc.m_BaseCamera.m_Near;
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
				model = model * glm::mat4_cast(glm::quat(glm::radians(glm::vec3(tr.m_Rotation.x, tr.m_Rotation.y, tr.m_Rotation.z))));
				model = glm::scale(model, glm::vec3(scale, scale, scale));
				pc.m_Model = model;

				vkCmdPushConstants(m_CommandBuffer->GetInUseCommandBuffer(), m_DebugRenderer->GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstant), &pc);
				vkCmdBindDescriptorSets(m_CommandBuffer->GetInUseCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_DebugRenderer->GetPipelineLayout(), 0, 1, &m_DebugRenderer->GetDescriptor(Index), 0, NULL);

				m_DebugRenderer->BindDebugDirectionalLight(m_CommandBuffer->GetInUseCommandBuffer());
				m_DebugRenderer->DrawDebugDirectionalLight(m_CommandBuffer->GetInUseCommandBuffer());
			}

		}
	}

	void SceneRenderer::SkyBoxPassInit()
	{
		auto Skybox1 = Resource::GetGUIDFromHex("86e229134d7c2f4e");
		auto Skybox2 = Resource::GetGUIDFromHex("584d1fc06a88a4a6");
		auto Skybox3 = Resource::GetGUIDFromHex("b0551365b3b9c5c2");
		auto Skybox4 = Resource::GetGUIDFromHex("8ffa171d290d63db");
		auto Skybox5 = Resource::GetGUIDFromHex("c3ed8c144c6f7bb4");
		auto Skybox6 = Resource::GetGUIDFromHex("855ecfb3bc347f5d");
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
		m_ShadowMapWidth = 8192;
		m_ShadowMapHeight = 8192;

		ImageConfig ImgConfig{};
		ImgConfig.DebugName = "Shadow Pass";
		ImgConfig.Format = ImageFormat::DEPTH16UN;
		ImgConfig.Width = m_ShadowMapWidth;
		ImgConfig.Height = m_ShadowMapHeight;
		ImgConfig.Usage = ImageUsage::Attachment;
		ImgConfig.CreateSampler = true;
		ImgConfig.Transfer = false;
		ImgConfig.AddressMode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;

		m_ShadowImages = std::make_shared<Image2D>(ImgConfig);
		m_ShadowRenderPass = std::make_shared<RenderPass>(m_Device, true);

		auto attachments = m_ShadowImages->GetImageData().ImageView;
		VkFramebufferCreateInfo framebufferCreateInfo{};
		framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferCreateInfo.renderPass = m_ShadowRenderPass->GetHandle();
		framebufferCreateInfo.attachmentCount = 1;
		framebufferCreateInfo.pAttachments = &attachments;
		framebufferCreateInfo.width = m_ShadowMapWidth;
		framebufferCreateInfo.height = m_ShadowMapHeight;
		framebufferCreateInfo.layers = 1;

		if (auto Result = vkCreateFramebuffer(m_Device->GetLogicalDevice(), &framebufferCreateInfo, nullptr, &m_ShadowFramebuffer); Result != VK_SUCCESS)
		{
			assert(Result == VK_SUCCESS && "Unable to create image sampler for shadow");
		}

		const float minFloat = std::numeric_limits<float>::min();
		const float maxFloat = std::numeric_limits<float>::max();
		m_ShadowAABBMin = glm::vec3(maxFloat, maxFloat, maxFloat);
		m_ShadowAABBMax = glm::vec3(minFloat, minFloat, minFloat);
		m_EditorShadowAABBMin = glm::vec3(maxFloat, maxFloat, maxFloat);
		m_EditorShadowAABBMax = glm::vec3(minFloat, minFloat, minFloat);
	}

	bool SceneRenderer::ShadowFrustumCheck(const BaseCamera& baseCamera)
	{
		//Check if frustum points outside of shadow AABB
		for (int i = 0; i < 8; i++)
		{
			if (m_IsEditorScene)
			{
				const auto cameraFrustum = baseCamera.GetFrustumCorners(false, m_EditorShadowRatio);

				if (cameraFrustum[i].x < m_EditorShadowAABBMin.x || cameraFrustum[i].x > m_EditorShadowAABBMax.x ||
					cameraFrustum[i].y < m_EditorShadowAABBMin.y || cameraFrustum[i].y > m_EditorShadowAABBMax.y ||
					cameraFrustum[i].z < m_EditorShadowAABBMin.z || cameraFrustum[i].z > m_EditorShadowAABBMax.z)
				{
					return true;
				}
			}
			else
			{
				//Actual camera frustum
				const auto cameraFrustum = baseCamera.GetFrustumCorners(false, 0.03f);

				if (cameraFrustum[i].x < m_ShadowAABBMin.x || cameraFrustum[i].x > m_ShadowAABBMax.x ||
					cameraFrustum[i].y < m_ShadowAABBMin.y || cameraFrustum[i].y > m_ShadowAABBMax.y ||
					cameraFrustum[i].z < m_ShadowAABBMin.z || cameraFrustum[i].z > m_ShadowAABBMax.z)
				{
					return true;
				}
			}
		}
		
		return false;
	}

	void SceneRenderer::RecreateShadowAABB(const std::array<glm::vec3, 8>& cameraFrustum)
	{
		//Calculate AABB in world space
		{
			float minX = std::numeric_limits<float>::max();
			float minY = std::numeric_limits<float>::max();
			float minZ = std::numeric_limits<float>::max();
			float maxX = std::numeric_limits<float>::min();
			float maxY = std::numeric_limits<float>::min();
			float maxZ = std::numeric_limits<float>::min();

			for (int i = 0; i < 8; i++)
			{
				minX = std::min(minX, cameraFrustum[i].x);
				minY = std::min(minY, cameraFrustum[i].y);
				minZ = std::min(minZ, cameraFrustum[i].z);
				maxX = std::max(maxX, cameraFrustum[i].x);
				maxY = std::max(maxY, cameraFrustum[i].y);
				maxZ = std::max(maxZ, cameraFrustum[i].z);
			}

			const glm::vec3 padding = glm::vec3(m_ShadowAABBPadding, m_ShadowAABBPadding, m_ShadowAABBPadding);
			if (m_IsEditorScene)
			{
				m_EditorShadowAABBMin = glm::vec3(minX, minY, minZ);
				m_EditorShadowAABBMin -= padding;
				m_EditorShadowAABBMax = glm::vec3(maxX, maxY, maxZ);
				m_EditorShadowAABBMax += padding;
				m_EditorShadowRenderPoint = (m_EditorShadowAABBMax + m_EditorShadowAABBMin) * 0.5f;
				m_EditorShadowRenderPoint.y = m_EditorShadowAABBMax.y; //Always render from top of AABB
			}
			else
			{
				m_ShadowAABBMin = glm::vec3(minX, minY, minZ);
				m_ShadowAABBMin -= padding;
				m_ShadowAABBMax = glm::vec3(maxX, maxY, maxZ);
				m_ShadowAABBMax += padding;
				m_ShadowRenderPoint = (m_ShadowAABBMax + m_ShadowAABBMin) * 0.5f;
				m_ShadowRenderPoint.y = m_ShadowAABBMax.y; //Always render from top of AABB
			}
		}
	}
}