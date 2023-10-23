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

//To be removed
#include "EditorCamera.h"

namespace TRE
{
	std::shared_ptr<DescriptorPool>& SceneRenderer::GetDescriptorPool()
	{
		return m_DescriptorPool;
	}

	std::vector<std::unique_ptr<Image>>& SceneRenderer::GetColorImages()
	{
		return m_ColorImages;
	}

	SceneRenderer::SceneRenderer(const std::shared_ptr<Device>& Device) : m_Device(Device)
	{
		uint32_t ImageCount = Engine::GetInstance().GetWindow()->GetSwapChain()->GetImageCount();

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

		for (auto material : ResourceManager::Instance().GetResourcesOfType<Material>())
		{
			material->Invalidate();
		}

		m_DebugRenderer = std::make_unique<DebugRenderer>(m_RenderPass);

		//m_Animation = std::make_unique<AnimationTest>(m_RenderPass);
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
			std::array<VkImageView, 2> attachments = { m_ColorImages[x]->GetImageView(), m_DepthImages[x]->GetImageView() };

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

		// Color attachment
		for (int x = 0; x < m_ColorImages.size(); x++)
		{
			m_ColorImages[x] = std::make_unique<Image>(SwapChain->GetWidth(), SwapChain->GetHeight(), SwapChain->GetColorFormat(), 
				VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
		}

		// Depth attachment
		for (int x = 0; x < m_DepthImages.size(); x++)
		{
			m_DepthImages[x] = std::make_unique<Image>(SwapChain->GetWidth(), SwapChain->GetHeight(), SwapChain->GetDepthFormat(),
								VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_ASPECT_DEPTH_BIT);
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
		vkDeviceWaitIdle(m_Device->GetLogicalDevice());

		for (int x = 0; x < m_ColorImages.size(); x++)
		{
			vkDestroyFramebuffer(m_Device->GetLogicalDevice(), m_FrameBuffer[x], nullptr);
		}

		m_ColorImages.clear();
		m_DepthImages.clear();
	}

	void SceneRenderer::BeginEditorFrame(const EditorCamera& RenderCamera)
	{
		UBO ubo{};

		ubo.m_ProjView = RenderCamera.GetViewProjectionMatrix();
		ubo.m_LightPosition = EditorCamera::Instance().GetPosition();
		ubo.m_CameraPosition = glm::vec4(EditorCamera::Instance().GetPosition(), 1.f);

		for (const auto& entity : ECSManager::Instance().GetEntities<DirectionalLight>())
		{
			const auto& light = entity->GetComponent<DirectionalLight>();
			ubo.m_LightDirection = glm::vec4(light.Direction, 1.f);
			ubo.m_LightAmbientColor = light.AmbientColor;
		}

		m_UBOBuffer->SetData(&ubo, sizeof(UBO));
	}

	void SceneRenderer::BeginFrame(const Camera& RenderCamera)
	{
		//UBO
		UBO ubo{};
		//const Camera& mainCamera = ECSSystemManager::Instance().GetSystem<CameraSystem>()->GetMainCamera()->GetComponent<Camera>();
		ubo.m_ProjView = RenderCamera.m_BaseCamera.m_ProjectionMatrix * RenderCamera.m_BaseCamera.m_ViewMatrix;
		ubo.m_LightPosition = { 0.f, 0.f, 0.f };
		ubo.m_CameraPosition = { 0.f, 0.f, 0.f, 0.f };

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
		auto& swapChain = Engine::GetInstance().GetWindow()->GetSwapChain();

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
		{
			DebugDrawPass(Index);
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
			PushConstant pc{};
			glm::mat4 model(1.f);
			model = glm::translate(model, tr.m_Position + sc.m_Offset);
			model = model * glm::scale(glm::mat4(1.f), glm::vec3(sc.m_Radius, sc.m_Radius, sc.m_Radius));
			pc.m_Model = model;
			vkCmdPushConstants(m_CommandBuffer->GetInUseCommandBuffer(), m_DebugRenderer->GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstant), &pc);

			m_DebugRenderer->BindDebugSphere(m_CommandBuffer->GetInUseCommandBuffer());
			m_DebugRenderer->DrawDebugSphere(m_CommandBuffer->GetInUseCommandBuffer());
		}

		for (const auto& box : ECSManager::Instance().GetEntities<BoxCollider>())
		{
			const Transform& tr = box->GetComponent<Transform>();
			const BoxCollider& bc = box->GetComponent<BoxCollider>();
			PushConstant pc{};
			glm::mat4 model(1.f);
			model = glm::translate(model, tr.m_Position + bc.m_Offset);
			model = model * glm::scale(glm::mat4(1.f), glm::vec3(bc.m_HalfExtents.x, bc.m_HalfExtents.y, bc.m_HalfExtents.z));
			pc.m_Model = model;
			vkCmdPushConstants(m_CommandBuffer->GetInUseCommandBuffer(), m_DebugRenderer->GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstant), &pc);

			m_DebugRenderer->BindDebugAABB(m_CommandBuffer->GetInUseCommandBuffer());
			m_DebugRenderer->DrawDebugAABB(m_CommandBuffer->GetInUseCommandBuffer());
		}

		/*for (const auto& lines : ECSManager::Instance().GetEntities<CapsuleCollider>())
		{

		}*/

		//for (int i = 0; i < 2; ++i)
		//{
		//	PushConstant pc{};
		//	glm::mat4 model(1.f);
		//	const float radius = 10.f;
		//	const float halfExtent = 10.f;
		//	model = glm::translate(model, glm::vec3(0, 0, 0));
		//	model = glm::rotate(model, glm::radians(90.f * i), glm::vec3(0, 1, 0));
		//	model = glm::rotate(model, glm::radians(180.f * i), glm::vec3(0, 0, 1));
		//	model = glm::scale(model, glm::vec3(radius, radius + halfExtent, radius));
		//	pc.m_Model = model;
		//	vkCmdPushConstants(m_CommandBuffer->GetInUseCommandBuffer(), m_DebugRenderer->GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstant), &pc);

		//	//Bind
		//	vkCmdBindDescriptorSets(m_CommandBuffer->GetInUseCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_DebugRenderer->GetPipelineLayout(), 0, 1, &m_DebugRenderer->GetDescriptor(Index), 0, NULL);

		//	m_DebugRenderer->BindDebugCapsule(m_CommandBuffer->GetInUseCommandBuffer());
		//	m_DebugRenderer->DrawDebugCapsule(m_CommandBuffer->GetInUseCommandBuffer());
		//}
	}
}