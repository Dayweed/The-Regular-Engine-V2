#include "pch.h"
#include "SceneRenderer.h"
#include "TREIncludes.h"
#include "VulkanTexture.h"
#include "Resource/ResourceManager.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "glm/gtx/quaternion.hpp"
#include "VulkanUtilities.h"
#include "FontRenderer.h"
#include "Renderer.h"
#include "EditorCamera.h"

namespace TRE
{
	std::unordered_map<SceneRenderer::SceneImage, std::shared_ptr<Image2D>> SceneRenderer::m_SceneImages;

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
		m_DepthPrepassUBO = std::make_shared<UniformBuffer>(UINT32_T_CAST(sizeof(DepthUBO)), 0);
		m_IDPrepassUBO = std::make_shared<UniformBuffer>(UINT32_T_CAST(sizeof(IDUBO)), 0);
		m_BoxBlurPostpassUBO = std::make_shared<UniformBuffer>(UINT32_T_CAST(sizeof(BoxBlurUBO)), 0);
		m_ParticleUBO2D = std::make_shared<UniformBuffer>(UINT32_T_CAST(sizeof(ParticleUBO)), 0);
		m_ParticleUBO3D = std::make_shared<UniformBuffer>(UINT32_T_CAST(sizeof(ParticleUBO)), 0);
	}

	void SceneRenderer::Initialize() 
	{
		float x = -1.f; float y = -1.f;
		float width = 2, height = 2;
		std::vector<PostVertex> data(4);

		//UVs are flipped because of the vulkan texture coordinate system
		data[0].Position = glm::vec2(x, y);
		data[0].UV = glm::vec2(0, 0);

		data[1].Position = glm::vec2(x + width, y);
		data[1].UV = glm::vec2(1, 0);

		data[2].Position = glm::vec2(x + width, y + height);
		data[2].UV = glm::vec2(1, 1);

		data[3].Position = glm::vec2(x, y + height);
		data[3].UV = glm::vec2(0, 1);

		std::vector<int> indices = { 0,1,2,2,3,0 };

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
		PipelineConfig.EnableBlending = true;
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
#pragma region ShadowPass
		{
			ShadowPassInit();

			PipelineConfigurations Config{};
			Config.Primitive = PrimitiveType::Triangles;
			Config.Shader = ResourceManager::Instance().GetResource<Shader>(5);
			Config.CullMode = VK_CULL_MODE_NONE;// VK_CULL_MODE_FRONT_BIT;
			Config.EnableBlending = true;
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
		}
#pragma endregion ShadowPass

#pragma region DepthPrepass
		{
			DepthPrepassInit();

			PipelineConfigurations DepthPrepassPipelineConfig{};
			DepthPrepassPipelineConfig.Primitive = PrimitiveType::Triangles;
			DepthPrepassPipelineConfig.Shader = ResourceManager::Instance().GetResource<Shader>(14);
			DepthPrepassPipelineConfig.CullMode = VK_CULL_MODE_BACK_BIT;
			DepthPrepassPipelineConfig.EnableBlending = true;
			DepthPrepassPipelineConfig.DepthCompareOp = VK_COMPARE_OP_LESS;
			m_DepthPrepassPipeline = std::make_shared<Pipeline>(DepthPrepassPipelineConfig, m_DepthPrepassRenderPass);

			PipelineConfigurations DepthPrepassAnimationPipelineConfig{};
			DepthPrepassAnimationPipelineConfig.Primitive = PrimitiveType::Triangles;
			DepthPrepassAnimationPipelineConfig.Shader = ResourceManager::Instance().GetResource<Shader>(15);
			DepthPrepassAnimationPipelineConfig.CullMode = VK_CULL_MODE_BACK_BIT;
			DepthPrepassAnimationPipelineConfig.EnableBlending = true;
			DepthPrepassAnimationPipelineConfig.DepthCompareOp = VK_COMPARE_OP_LESS;
			DepthPrepassAnimationPipelineConfig.UseAutoShaderVertexInput = false;
			DepthPrepassAnimationPipelineConfig.CustomVertexBufferInputLayout =
			{
				{ { VertexInputDataType::Vec3, VertexInputDataType::Vec3, VertexInputDataType::Vec3, VertexInputDataType::Vec3, VertexInputDataType::Vec3, VertexInputDataType::Vec2 }, 0},
				{ { VertexInputDataType::Vec4, VertexInputDataType::IVec4 }, 1 }
			};
			m_DepthPrepassAnimationPipeline = std::make_shared<Pipeline>(DepthPrepassAnimationPipelineConfig, m_DepthPrepassRenderPass);

			m_DepthPrepassMaterial = std::make_shared<Material>(ResourceManager::Instance().GetResource<Shader>(14));
			m_DepthPrepassMaterial->Invalidate();
		}
#pragma endregion DepthPrepass

#pragma region IDPrepass
		{
			IDPrepassInit();

			PipelineConfigurations IDPrepassPipelineConfig{};
			IDPrepassPipelineConfig.Primitive = PrimitiveType::Triangles;
			IDPrepassPipelineConfig.Shader = ResourceManager::Instance().GetResource<Shader>(14);
			IDPrepassPipelineConfig.CullMode = VK_CULL_MODE_BACK_BIT;
			IDPrepassPipelineConfig.EnableBlending = true;
			IDPrepassPipelineConfig.DepthCompareOp = VK_COMPARE_OP_LESS;
			m_IDPrepassPipeline = std::make_shared<Pipeline>(IDPrepassPipelineConfig, m_IDPrepassRenderPass);

			PipelineConfigurations IDPrepassAnimationPipelineConfig{};
			IDPrepassAnimationPipelineConfig.Primitive = PrimitiveType::Triangles;
			IDPrepassAnimationPipelineConfig.Shader = ResourceManager::Instance().GetResource<Shader>(15);
			IDPrepassAnimationPipelineConfig.CullMode = VK_CULL_MODE_BACK_BIT;
			IDPrepassAnimationPipelineConfig.EnableBlending = true;
			IDPrepassAnimationPipelineConfig.UseAutoShaderVertexInput = false;
			IDPrepassAnimationPipelineConfig.DepthCompareOp = VK_COMPARE_OP_LESS;
			IDPrepassAnimationPipelineConfig.CustomVertexBufferInputLayout =
			{
				{ { VertexInputDataType::Vec3, VertexInputDataType::Vec3, VertexInputDataType::Vec3, VertexInputDataType::Vec3, VertexInputDataType::Vec3, VertexInputDataType::Vec2 }, 0},
				{ { VertexInputDataType::Vec4, VertexInputDataType::IVec4 }, 1 }
			};
			m_IDPrepassAnimationPipeline = std::make_shared<Pipeline>(IDPrepassAnimationPipelineConfig, m_IDPrepassRenderPass);

			m_IDPrepassMaterial = std::make_shared<Material>(ResourceManager::Instance().GetResource<Shader>(14));
			m_IDPrepassMaterial->Invalidate();
		}
#pragma endregion DepthPrepass

#pragma region BlurPostpass
		{
			BoxBlurPostpassInit();
			PipelineConfigurations blurPostpassPipelineConfig{};
			blurPostpassPipelineConfig.Primitive = PrimitiveType::Triangles;
			blurPostpassPipelineConfig.Shader = ResourceManager::Instance().GetResource<Shader>(18);
			blurPostpassPipelineConfig.CullMode = VK_CULL_MODE_NONE;
			blurPostpassPipelineConfig.EnableDepthTest = false;
			blurPostpassPipelineConfig.EnableBlending = true;
			m_BoxBlurPostpassPipeline = std::make_shared<Pipeline>(blurPostpassPipelineConfig, m_BoxBlurPostpassRenderPass);

			m_BoxBlurPostpassMaterial = std::make_shared<Material>(ResourceManager::Instance().GetResource<Shader>(18));
			m_BoxBlurPostpassMaterial->Invalidate();

			m_BoxBlurIndexBuffer = std::make_shared<IndexBuffer>(static_cast<void*>(indices.data()),
				UINT32_T_CAST(sizeof(int) * indices.size()),
				UINT32_T_CAST(indices.size()));

			m_BoxBlurVertexBuffer = std::make_shared<VertexBuffer>(static_cast<void*>(data.data()),
				UINT32_T_CAST(data.size() * sizeof(QuadVertex)));
		}
#pragma endregion BlurPrepass

		m_UIRenderer = std::make_shared<UIRenderer>(m_Device);
		m_FontRenderer = std::make_shared<FontRenderer>(m_Device);
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

		float x3D = -1.f; float y3D = -1.f;
		float width3D = 2, height3D = 2;
		std::vector<QuadVertex> Sprite3DVertices(4);

		Sprite3DVertices[0].Position = glm::vec3(x3D, y3D, 0.f);
		Sprite3DVertices[0].TexCoord = glm::vec2(0, 0);

		Sprite3DVertices[1].Position = glm::vec3(x3D + width3D, y3D, 0.f);
		Sprite3DVertices[1].TexCoord = glm::vec2(1, 0);

		Sprite3DVertices[2].Position = glm::vec3(x3D + width3D, y3D + height3D, 0.f);
		Sprite3DVertices[2].TexCoord = glm::vec2(1, 1);

		Sprite3DVertices[3].Position = glm::vec3(x3D, y3D + height3D, 0.f);
		Sprite3DVertices[3].TexCoord = glm::vec2(0, 1);

		std::vector<int> Sprite3DIndices = { 0,1,2,2,3,0 };

		PipelineConfigurations Sprite3DPipelineConfig{};
		Sprite3DPipelineConfig.Primitive = PrimitiveType::Triangles;
		Sprite3DPipelineConfig.Shader = ResourceManager::Instance().GetResource<Shader>(6);
		Sprite3DPipelineConfig.CullMode = VK_CULL_MODE_NONE;
		Sprite3DPipelineConfig.EnableBlending = true;
		Sprite3DPipelineConfig.EnableDepthTest = true;
		m_Sprite3DPipeline = std::make_shared<Pipeline>(Sprite3DPipelineConfig, m_RenderPass);

		m_Sprite3DUBO = std::make_shared<UniformBuffer>(UINT32_T_CAST(sizeof(UIUBO)), 0);

		m_Sprite3DIndexBuffer = std::make_shared<IndexBuffer>(static_cast<void*>(Sprite3DIndices.data()),
			UINT32_T_CAST(sizeof(int) * Sprite3DIndices.size()),
			UINT32_T_CAST(Sprite3DIndices.size()));

		m_Sprite3DVertexBuffer = std::make_shared<VertexBuffer>(static_cast<void*>(Sprite3DVertices.data()),
			UINT32_T_CAST(Sprite3DVertices.size() * sizeof(QuadVertex)));
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

		vkDestroyFramebuffer(m_Device->GetLogicalDevice(), m_ShadowFramebuffer[0], nullptr);
		vkDestroyFramebuffer(m_Device->GetLogicalDevice(), m_ShadowFramebuffer[1], nullptr);
		vkDestroyFramebuffer(m_Device->GetLogicalDevice(), m_DepthPrepassFramebuffer, nullptr);
		vkDestroyFramebuffer(m_Device->GetLogicalDevice(), m_IDPrepassFramebuffer, nullptr);
		vkDestroyFramebuffer(m_Device->GetLogicalDevice(), m_BoxBlurPostpassFramebuffer, nullptr);

		m_SceneImages.clear();
	}

	void SceneRenderer::BeginEditorFrame()
	{
		const EditorCamera& editorCamera = EditorCamera::Instance();
		const BaseCamera& baseCamera = editorCamera.m_BaseCamera;
		const Transform& transform = ECSSystemManager::Instance().GetSystem<CameraSystem>()->GetMainCamera()->GetComponent<Transform>();
		
		UBO ubo{};
		ubo.m_Gamma = Renderer::IsGammaOn() ? 2.2f : 1.0f;
		ubo.m_ProjView = editorCamera.GetViewProjectionMatrix();
		ubo.m_LightPosition = glm::vec4(transform.m_Position, 0.f);
		ubo.m_CameraPosition = glm::vec4(transform.m_Position, 1.f);

		m_ProjView3D = ubo.m_ProjView;

		SkyBoxUBO UBO_SkyBox;
		UBO_SkyBox.Proj = editorCamera.GetProjectionMatrix();
		UBO_SkyBox.View = editorCamera.GetViewMatrix();
		UBO_SkyBox.Gamma = Renderer::IsGammaOn() ? 2.2f : 1.0f;

		glm::mat4 shadowDepthViewMatrix(1.f);
		bool recalculateShadowFrustum = ShadowFrustumCheck(baseCamera);
		recalculateShadowFrustum = true;

		auto DLights = ECSManager::Instance().GetEntities<DirectionalLight>();
		std::sort(DLights.begin(), DLights.end(), [](const auto& first, const auto& sec)
		{
			return first->GetComponent<Properties>().m_Index < sec->GetComponent<Properties>().m_Index;
		});

		if (DLights.size() > 2)
		{
			TRE_CORE_CRITICAL("Engine does not support more than 2 directional light");
		}

		ShadowUBO UBO_Shadow{};

		for (int x = 0; x < DLights.size(); x++)
		{
			const auto& entity = DLights[x];
			const auto& lightTransform = entity->GetComponent<Transform>();
			const auto& light = entity->GetComponent<DirectionalLight>();
			ubo.m_LightDirection[x] = glm::vec4(light.m_Direction, 1.f);
			ubo.m_LightDirectionalColor[x] = light.m_DirectionalColor;
			ubo.m_LightAmbientColor[x] = light.m_AmbientColor;
			ubo.m_ShadowIntensity = light.m_ShadowIntensity;

			if (recalculateShadowFrustum)
			{
				RecreateShadowAABB(baseCamera.GetFrustumCorners(false, m_EditorShadowRatio));
				shadowDepthViewMatrix = glm::translate(glm::mat4(1.f), m_EditorShadowRenderPoint) * glm::toMat4(glm::quat(glm::radians(-lightTransform.m_Rotation)));
			}

			if (recalculateShadowFrustum)
			{
				glm::mat4 shadowDepthProjectionMatrix;
				const float deltaX = m_EditorShadowAABBMax.x - m_EditorShadowAABBMin.x;
				const float deltaY = m_EditorShadowAABBMax.y - m_EditorShadowAABBMin.y;
				const float deltaZ = m_EditorShadowAABBMax.z - m_EditorShadowAABBMin.z;
				const float orthoLength = deltaX;
				const float orthoHeight = deltaY;
				const float orthoNear = 0.1f;
				const float orthoFar = orthoNear + deltaZ;

				shadowDepthProjectionMatrix = glm::mat4(1.f);
				shadowDepthProjectionMatrix[0][0] = -2.f / (orthoLength - -orthoLength);
				shadowDepthProjectionMatrix[1][1] = -2.f / (orthoHeight - -orthoHeight);
				shadowDepthProjectionMatrix[2][2] = 2.f / (orthoFar - orthoNear);
				shadowDepthProjectionMatrix[3][0] = -(orthoLength + -orthoLength) / (orthoLength - -orthoLength);
				shadowDepthProjectionMatrix[3][1] = -(orthoHeight + -orthoHeight) / (orthoHeight - -orthoHeight);
				shadowDepthProjectionMatrix[3][2] = -(orthoNear) / (orthoFar - orthoNear);

				shadowDepthViewMatrix = glm::inverse(shadowDepthViewMatrix);
				m_EditorShadowView[x] = shadowDepthViewMatrix;
				m_EditorShadowProj[x] = shadowDepthProjectionMatrix;

				UBO_Shadow.view[x] = m_EditorShadowView[x];
				UBO_Shadow.proj[x] = m_EditorShadowProj[x];
			}
			
			ubo.m_LightSpaceMatrix[x] = m_EditorShadowProj[x] * m_EditorShadowView[x];
		}

		m_ShadowUBO->SetData(&UBO_Shadow, sizeof(ShadowUBO));

		m_UBOBuffer->SetData(&ubo, sizeof(UBO));
		m_UBOSkybox->SetData(&UBO_SkyBox, sizeof(SkyBoxUBO));

		{
			ParticleUBO particleUBO{};
			particleUBO.ProjView = baseCamera.m_ProjectionMatrix * baseCamera.m_ViewMatrix;
			particleUBO.Gamma = Renderer::IsGammaOn() ? 2.2f : 1.0f;

			m_ParticleUBO3D->SetData(&particleUBO, sizeof(ParticleUBO));

			ParticleUBO particleUBO2D{};
			particleUBO2D.Gamma = Renderer::IsGammaOn() ? 2.2f : 1.0f;
			//Why so hardcoded (:
			const auto width = 1920.f;
			const auto height = 1080.f;
			glm::mat4 TranslateToMid = glm::translate(glm::identity<glm::mat4>(), glm::vec3(width / 2.f, height / 2.f, 0.f));
			particleUBO2D.ProjView = glm::ortho(0.f, width, 0.f, height) * TranslateToMid;
			m_ParticleUBO2D->SetData(&particleUBO2D, sizeof(ParticleUBO));
		}
	}

	void SceneRenderer::BeginFrame()
	{
		//UBO
		const Entity& mainCamera = ECSSystemManager::Instance().GetSystem<CameraSystem>()->GetMainCamera();
		const Camera& cameraComponent = mainCamera->GetComponent<Camera>();
		const BaseCamera& baseCamera = cameraComponent.m_BaseCamera;
		const Transform& cameraTransform = mainCamera->GetComponent<Transform>();
		auto SwapChain = Engine::GetInstance().GetWindow()->GetSwapChain();

		UBO ubo{};
		ubo.m_Gamma = Renderer::IsGammaOn() ? 2.2f : 1.0f;
		ubo.m_ProjView = baseCamera.m_ProjectionMatrix * baseCamera.m_ViewMatrix;
		ubo.m_LightPosition = glm::vec4(cameraTransform.m_Position, 0.f);
		ubo.m_CameraPosition = glm::vec4(cameraTransform.m_Position, 1.f);
		
		m_ProjView3D = ubo.m_ProjView;

		SkyBoxUBO UBO_SkyBox;
		UBO_SkyBox.Proj = baseCamera.m_ProjectionMatrix;
		UBO_SkyBox.View = baseCamera.m_ViewMatrix;

		glm::mat4 shadowDepthViewMatrix(1.f);
		bool recalculateShadowFrustum = ShadowFrustumCheck(baseCamera);
		recalculateShadowFrustum = true;

		ShadowUBO UBO_Shadow{};

		auto DLights = ECSManager::Instance().GetEntities<DirectionalLight>();
		std::sort(DLights.begin(), DLights.end(), [](const auto& first, const auto& sec)
		{
			return first->GetComponent<Properties>().m_Index < sec->GetComponent<Properties>().m_Index;
		});

		for (int x = 0; x < DLights.size(); x++)
		{
			const auto& entityDirectional = DLights[x];
			const auto& lightTransform = entityDirectional->GetComponent<Transform>();
			const auto& light = entityDirectional->GetComponent<DirectionalLight>();
			ubo.m_LightDirection[x] = glm::vec4(light.m_Direction, 1.f);
			ubo.m_LightDirectionalColor[x] = light.m_DirectionalColor;
			ubo.m_LightAmbientColor[x] = light.m_AmbientColor;
			ubo.m_ShadowIntensity = light.m_ShadowIntensity;

			if (recalculateShadowFrustum)
			{
				//RecreateShadowAABB(baseCamera.GetFrustumCorners(false, 0.033f));
				RecreateShadowAABB(baseCamera.GetFrustumCorners(false, 0.17f));
				//m_ShadowRenderPoint.y = lightTransform.m_Position.y;
				glm::vec3 tempRotation = glm::radians(lightTransform.m_Rotation);
				glm::mat4 rotationMat = glm::toMat4(glm::quat(tempRotation));
				shadowDepthViewMatrix = glm::translate(glm::mat4(1.f), m_ShadowRenderPoint) * rotationMat;
			}

			if (recalculateShadowFrustum)
			{
				glm::mat4 shadowDepthProjectionMatrix;
				const float deltaX = m_ShadowAABBMax.x - m_ShadowAABBMin.x;
				const float deltaY = m_ShadowAABBMax.y - m_ShadowAABBMin.y;
				const float deltaZ = m_ShadowAABBMax.z - m_ShadowAABBMin.z;
				const float orthoLength = deltaX;
				const float orthoHeight = deltaY;
				const float orthoNear = 0.1f;
				const float orthoFar = orthoNear + deltaZ;

				shadowDepthProjectionMatrix = glm::mat4(1.f);
				shadowDepthProjectionMatrix[0][0] = -2.f / (orthoLength - -orthoLength);
				shadowDepthProjectionMatrix[1][1] = -2.f / (orthoHeight - -orthoHeight);
				shadowDepthProjectionMatrix[2][2] = 2.f / (orthoFar - orthoNear);
				shadowDepthProjectionMatrix[3][0] = -(orthoLength + -orthoLength) / (orthoLength - -orthoLength);
				shadowDepthProjectionMatrix[3][1] = -(orthoHeight + -orthoHeight) / (orthoHeight - -orthoHeight);
				shadowDepthProjectionMatrix[3][2] = -(orthoNear) / (orthoFar - orthoNear);

				shadowDepthViewMatrix = glm::inverse(shadowDepthViewMatrix);
				m_ShadowView[x] = shadowDepthViewMatrix;
				m_ShadowProj[x] = shadowDepthProjectionMatrix;

				UBO_Shadow.view[x] = m_ShadowView[x];
				UBO_Shadow.proj[x] = m_ShadowProj[x];
			}
			
			ubo.m_LightSpaceMatrix[x] = m_ShadowProj[x] * m_ShadowView[x];
		}

		m_ShadowUBO->SetData(&UBO_Shadow, sizeof(ShadowUBO));

		
		{
			DepthUBO depthUBO{};
			depthUBO.view = baseCamera.m_ViewMatrix;
			depthUBO.proj = baseCamera.m_ProjectionMatrix;

			m_DepthPrepassUBO->SetData(&depthUBO, sizeof(DepthUBO));
		}

		{
			IDUBO idUBO{};
			idUBO.view = baseCamera.m_ViewMatrix;
			idUBO.proj = baseCamera.m_ProjectionMatrix;

			m_IDPrepassUBO->SetData(&idUBO, sizeof(IDUBO));
		}

		{
			BoxBlurUBO boxBlurUBO{};
			boxBlurUBO.m_InvScreenSize = glm::vec2(1.f / SwapChain->GetWidth(), 1.f / SwapChain->GetHeight());

			m_BoxBlurPostpassUBO->SetData(&boxBlurUBO, sizeof(BoxBlurUBO));
		}

		m_UBOBuffer->SetData(&ubo, sizeof(UBO));
		m_UBOSkybox->SetData(&UBO_SkyBox, sizeof(SkyBoxUBO));

		{
			ParticleUBO particleUBO{};
			particleUBO.ProjView = baseCamera.m_ProjectionMatrix * baseCamera.m_ViewMatrix;
			particleUBO.Gamma = Renderer::IsGammaOn() ? 2.2f : 1.0f;

			m_ParticleUBO3D->SetData(&particleUBO, sizeof(ParticleUBO));

			ParticleUBO particleUBO2D{};
			particleUBO2D.Gamma = Renderer::IsGammaOn() ? 2.2f : 1.0f;
			const auto width = 1920.f;
			const auto height = 1080.f;
			glm::mat4 TranslateToMid = glm::translate(glm::identity<glm::mat4>(), glm::vec3(width / 2.f, height / 2.f, 0.f));
			particleUBO2D.ProjView = glm::ortho(0.f, width, 0.f, height) * TranslateToMid;
			m_ParticleUBO2D->SetData(&particleUBO2D, sizeof(ParticleUBO));
		}
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

		const auto& SC = Engine::GetInstance().GetWindow()->GetSwapChain();
		uint32_t Index = SC->GetCurrentBufferIndex();
		uint32_t ImageIndex = SC->GetCurrentImageIndex();

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

		if (m_IsEditorScene == false)
		{
			DepthPrepass(Index, materialSort);
			IDPrepass(Index, materialSort);
		}

		m_RenderPass->BeginRenderPass(m_CommandBuffer->GetInUseCommandBuffer(), m_FrameBuffer[ImageIndex]);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.f;
		viewport.width = static_cast<float>(SC->GetWidth());
		viewport.height = static_cast<float>(SC->GetHeight());
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(m_CommandBuffer->GetInUseCommandBuffer(), 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = SC->GetSwapChainExtent();
		vkCmdSetScissor(m_CommandBuffer->GetInUseCommandBuffer(), 0, 1, &scissor);

		SkyBoxPass(Index);
		GeometryPass(Index, materialSort);
		GeometryAnimationPass(Index, materialSort);
		Sprite3DPass(Index);
		DebugDrawPass(Index);
		m_ParticleRenderer->Render(m_ParticleUBO2D, m_ParticleUBO3D, m_CommandBuffer, m_IsEditorScene);
		m_ParticleRenderer->Render3D(m_UBOBuffer, m_CommandBuffer, m_IsEditorScene);

		Renderer::EndRenderPass(m_CommandBuffer);

		if (m_IsEditorScene == false)
		{
			BoxBlurPostpass(Index);
			PostProcessingManager::Instance().PreRender(m_FrameBuffer[ImageIndex], m_CommandBuffer, Index);

			m_UIRenderer->Render(m_FrameBuffer[ImageIndex], m_CommandBuffer, m_IsEditorScene);

			Profiler::Instance().StartTimer("FontPass");
			m_FontRenderer->RenderFont(m_FrameBuffer[ImageIndex], m_CommandBuffer);
			Profiler::Instance().EndTimer("FontPass");

			PostProcessingManager::Instance().PostRender(m_FrameBuffer[ImageIndex], m_CommandBuffer, Index);
		}

		m_CommandBuffer->End();
		m_CommandBuffer->Submit();
	}

	void SceneRenderer::Sprite3DPass(uint32_t Index)
	{
		UIUBO ubo{};
		ubo.m_ProjView2DSpace = m_ProjView3D;
		m_Sprite3DUBO->SetData(&ubo, sizeof(UIUBO));

		auto AllSprites = ECSManager::Instance().GetEntities<Sprite3DComponent>();
		std::sort(AllSprites.begin(), AllSprites.end(), [](const Entity& e1, const Entity& e2)
		{
			auto Comp1 = e1->GetComponent<Transform>();
			auto Comp2 = e2->GetComponent<Transform>();

			return Comp1.m_Position.z < Comp2.m_Position.z;
		});

		auto SC = Engine::GetInstance().GetWindow()->GetSwapChain();
		// uint32_t ImageIndex = SC->GetCurrentImageIndex();
		Renderer::BindPipeline(m_CommandBuffer, m_Sprite3DPipeline);
		for (auto Entity : AllSprites)
		{
			auto Comp = Entity->GetComponent<Sprite3DComponent>();
			if (!Comp.m_IsVisible || !Comp.m_Texture || !Comp.m_Material) continue;

			UI_PushConstant pc{};
			auto TransformComp = Entity->GetComponent<Transform>();
			pc.L2W = TransformComp.m_WorldXform;
			pc.Color = Comp.m_Color;

			vkCmdPushConstants(m_CommandBuffer->GetInUseCommandBuffer(), m_Sprite3DPipeline->GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(UI_PushConstant), &pc);

			Comp.m_Material->SetTexture("UI_Texture", Comp.m_Texture);

			if (m_IsEditorScene)
			{
				Comp.m_Material->UpdateForEditorSceneRendering(m_Sprite3DUBO, Index);
				vkCmdBindDescriptorSets(m_CommandBuffer->GetInUseCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_Sprite3DPipeline->GetPipelineLayout(), 0, 1, &Comp.m_Material->GetEditorDescriptor(Index), 0, NULL);
			}
			else
			{
				Comp.m_Material->UpdateForRendering(m_Sprite3DUBO, Index);
				vkCmdBindDescriptorSets(m_CommandBuffer->GetInUseCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_Sprite3DPipeline->GetPipelineLayout(), 0, 1, &Comp.m_Material->GetDescriptor(Index), 0, NULL);
			}

			VkDeviceSize offsets[] = { 0 };
			VkBuffer VB = VK_NULL_HANDLE;
			VB = m_Sprite3DVertexBuffer->GetBuffer();

			vkCmdBindVertexBuffers(m_CommandBuffer->GetInUseCommandBuffer(), 0, 1, &VB, offsets);
			vkCmdBindIndexBuffer(m_CommandBuffer->GetInUseCommandBuffer(), m_Sprite3DIndexBuffer->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);

			vkCmdDrawIndexed(m_CommandBuffer->GetInUseCommandBuffer(), m_Sprite3DIndexBuffer->GetIndexCount(), 1, 0, 0, 0);
		}
	}

	void SceneRenderer::GeometryPass(uint32_t Index, const std::multimap<ResourceHandle, Entity>& MaterialSort)
	{
		Renderer::BindPipeline(m_CommandBuffer, m_Pipeline);
		for (const auto& go_mr : MaterialSort)
		{
			const MeshRenderer& mr = go_mr.second->GetComponent<MeshRenderer>();

			if (go_mr.second->HasComponent<AnimationComponent>() && mr.m_RenderObject->IsAnimated()) //This only render static meshes
			{
				continue;
			}

			PushConstantGeometry pc{};
			pc.m_Model = go_mr.second->GetComponent<Transform>().m_WorldXform;
			pc.m_DrawShadow = mr.m_DrawShadow;
			vkCmdPushConstants(m_CommandBuffer->GetInUseCommandBuffer(), m_Pipeline->GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstantGeometry), &pc);

			ResourceHandle currentMaterialHandle = go_mr.first;

			//If no material instance attached, use default PBR material
			if (currentMaterialHandle != m_PreviousMaterialHandle)
			{
				if (mr.m_MaterialInstance == nullptr)
				{
					if (m_IsEditorScene)
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
					if (m_IsEditorScene)
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

			PushConstantGeometry pc{};
			pc.m_Model = Entity->GetComponent<Transform>().m_WorldXform;
			pc.m_DrawShadow = MeshRendererComp.m_DrawShadow;
			vkCmdPushConstants(m_CommandBuffer->GetInUseCommandBuffer(), m_AnimationPipeline->GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstantGeometry), &pc);

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
					m_DefaultAnimationPBRMaterial->UpdateForEditorAnimationRendering(m_UBOBuffer, Index, AnimationComp.m_UBO);
					vkCmdBindDescriptorSets(m_CommandBuffer->GetInUseCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_AnimationPipeline->GetPipelineLayout(), 0, 1, &m_DefaultAnimationPBRMaterial->GetEditorDescriptor(Index), 0, NULL);
				}
				else
				{
					m_DefaultAnimationPBRMaterial->UpdateForAnimationRendering(m_UBOBuffer, Index, AnimationComp.m_UBO);
					vkCmdBindDescriptorSets(m_CommandBuffer->GetInUseCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_AnimationPipeline->GetPipelineLayout(), 0, 1, &m_DefaultAnimationPBRMaterial->GetDescriptor(Index), 0, NULL);
				}
			}
			else
			{
				if (m_IsEditorScene)
				{
					MeshRendererComp.m_AnimationMaterialInstance->UpdateForEditorAnimationRendering(m_UBOBuffer, Index, AnimationComp.m_UBO);
					vkCmdBindDescriptorSets(m_CommandBuffer->GetInUseCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_AnimationPipeline->GetPipelineLayout(), 0, 1, &MeshRendererComp.m_AnimationMaterialInstance->GetEditorDescriptor(Index), 0, NULL);
				}
				else
				{
					MeshRendererComp.m_AnimationMaterialInstance->UpdateForAnimationRendering(m_UBOBuffer, Index, AnimationComp.m_UBO);
					vkCmdBindDescriptorSets(m_CommandBuffer->GetInUseCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_AnimationPipeline->GetPipelineLayout(), 0, 1, &MeshRendererComp.m_AnimationMaterialInstance->GetDescriptor(Index), 0, NULL);
				}
			}

			MeshRendererComp.m_RenderObject->BindAnimation(m_CommandBuffer->GetInUseCommandBuffer());
		}
	}

	void SceneRenderer::SkyBoxPass(uint32_t Index)
	{
		auto Skybox = Renderer::GetSkybox();
		Renderer::BindPipeline(m_CommandBuffer, m_SkyboxPipeline);
		if (m_IsEditorScene)
		{
			Skybox->UpdateMaterial(m_UBOSkybox, Index, m_ShadowDescriptInfo, m_IsEditorScene);
			vkCmdBindDescriptorSets(m_CommandBuffer->GetInUseCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_SkyboxPipeline->GetPipelineLayout(), 0, 1, &Skybox->GetMaterial()->GetEditorDescriptor(Index), 0, NULL);
		}
		else
		{
			Skybox->UpdateMaterial(m_UBOSkybox, Index, m_ShadowDescriptInfo, m_IsEditorScene);
			vkCmdBindDescriptorSets(m_CommandBuffer->GetInUseCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_SkyboxPipeline->GetPipelineLayout(), 0, 1, &Skybox->GetMaterial()->GetDescriptor(Index), 0, NULL);
		}

		VkBuffer vertexBuffers[] = { Skybox->GetVertexBuffer() };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(m_CommandBuffer->GetInUseCommandBuffer(), 0, 1, vertexBuffers, offsets);

		vkCmdBindIndexBuffer(m_CommandBuffer->GetInUseCommandBuffer(), Skybox->GetIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);

		vkCmdDrawIndexed(m_CommandBuffer->GetInUseCommandBuffer(), Skybox->GetIndexCount(), 1, 0, 0, 0);
	}

	void SceneRenderer::ShadowPass(uint32_t Index, const std::multimap<ResourceHandle, Entity>& MaterialSort)
	{
		for (int x = 0; x < 2; x++) //Render 2 shadow maps
		{
			VkClearValue clearValues[2];
			clearValues[0].depthStencil = { 1.0f, 0 };
			VkRenderPassBeginInfo renderPassInfo{};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = m_ShadowRenderPass->GetHandle();
			renderPassInfo.framebuffer = m_ShadowFramebuffer[x];
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

			for (const auto& go_mr : MaterialSort)
			{
				const MeshRenderer& mr = go_mr.second->GetComponent<MeshRenderer>();
				ResourceHandle currentMaterialHandle = go_mr.first;

				if (mr.m_DLightIndex != x)
					continue;

				PushConstant pc{};
				pc.m_Model = go_mr.second->GetComponent<Transform>().m_WorldXform;
				pc.m_DLightIndex = mr.m_DLightIndex;
				
				vkCmdPushConstants(m_CommandBuffer->GetInUseCommandBuffer(), m_ShadowPipeline->GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstant), &pc);

				if (m_IsEditorScene)
				{
					m_ShadowMaterial->UpdateForEditorSceneRendering(m_ShadowUBO, Index);
					vkCmdBindDescriptorSets(m_CommandBuffer->GetInUseCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_ShadowPipeline->GetPipelineLayout(), 0, 1, &m_ShadowMaterial->GetEditorDescriptor(Index), 0, NULL);
				}
				else
				{
					m_ShadowMaterial->UpdateForRendering(m_ShadowUBO, Index);
					vkCmdBindDescriptorSets(m_CommandBuffer->GetInUseCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_ShadowPipeline->GetPipelineLayout(), 0, 1, &m_ShadowMaterial->GetDescriptor(Index), 0, NULL);
				}

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

				if (MeshComp.m_DLightIndex != x)
					continue;

				PushConstant pc{};
				pc.m_Model = Entity->GetComponent<Transform>().m_WorldXform;
				pc.m_DLightIndex = MeshComp.m_DLightIndex;
				vkCmdPushConstants(m_CommandBuffer->GetInUseCommandBuffer(), m_ShadowAnimationPipeline->GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstant), &pc);

				if (m_IsEditorScene)
				{
					AnimComp.m_ShadowAnimationMaterial->UpdateForEditorAnimationRendering(m_ShadowUBO, Index, AnimComp.m_UBO);
					vkCmdBindDescriptorSets(m_CommandBuffer->GetInUseCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_ShadowAnimationPipeline->GetPipelineLayout(), 0, 1, &AnimComp.m_ShadowAnimationMaterial->GetEditorDescriptor(Index), 0, NULL);
				}
				else
				{
					AnimComp.m_ShadowAnimationMaterial->UpdateForAnimationRendering(m_ShadowUBO, Index, AnimComp.m_UBO);
					vkCmdBindDescriptorSets(m_CommandBuffer->GetInUseCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_ShadowAnimationPipeline->GetPipelineLayout(), 0, 1, &AnimComp.m_ShadowAnimationMaterial->GetDescriptor(Index), 0, NULL);
				}

				MeshComp.m_RenderObject->BindAnimation(m_CommandBuffer->GetInUseCommandBuffer());
			}

			Renderer::EndRenderPass(m_CommandBuffer);

		}
	}

	void SceneRenderer::DepthPrepass(uint32_t Index, const std::multimap<ResourceHandle, Entity>& MaterialSort)
	{
		VkClearValue clearValues[2];
		clearValues[0].depthStencil = { 1.0f, 0 };
		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = m_DepthPrepassRenderPass->GetHandle();
		renderPassInfo.framebuffer = m_DepthPrepassFramebuffer;
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = { m_DepthPrepassMapWidth, m_DepthPrepassMapHeight };
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = clearValues;

		vkCmdBeginRenderPass(m_CommandBuffer->GetInUseCommandBuffer(), &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport2{};
		viewport2.x = 0.0f;
		viewport2.y = 0.0f;
		viewport2.width = (float)m_DepthPrepassMapWidth;
		viewport2.height = (float)m_DepthPrepassMapHeight;
		viewport2.minDepth = 0.0f;
		viewport2.maxDepth = 1.0f;
		vkCmdSetViewport(m_CommandBuffer->GetInUseCommandBuffer(), 0, 1, &viewport2);

		VkRect2D scissor2{};
		scissor2.extent = { m_DepthPrepassMapWidth, m_DepthPrepassMapHeight };
		vkCmdSetScissor(m_CommandBuffer->GetInUseCommandBuffer(), 0, 1, &scissor2);

		Renderer::BindPipeline(m_CommandBuffer, m_DepthPrepassPipeline);

		m_DepthPrepassMaterial->UpdateForRendering(m_DepthPrepassUBO, Index);
		vkCmdBindDescriptorSets(m_CommandBuffer->GetInUseCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_DepthPrepassPipeline->GetPipelineLayout(), 0, 1, &m_DepthPrepassMaterial->GetDescriptor(Index), 0, NULL);

		for (const auto& go_mr : MaterialSort)
		{
			const MeshRenderer& mr = go_mr.second->GetComponent<MeshRenderer>();
			ResourceHandle currentMaterialHandle = go_mr.first;

			PushConstant pc{};
			pc.m_Model = go_mr.second->GetComponent<Transform>().m_WorldXform;
			vkCmdPushConstants(m_CommandBuffer->GetInUseCommandBuffer(), m_DepthPrepassPipeline->GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstant), &pc);

			mr.m_RenderObject->Bind(m_CommandBuffer->GetInUseCommandBuffer());
			mr.m_RenderObject->Draw(m_CommandBuffer->GetInUseCommandBuffer());

			m_PreviousMaterialHandle = currentMaterialHandle;
		}
		m_PreviousMaterialHandle = 0;

		Renderer::BindPipeline(m_CommandBuffer, m_DepthPrepassAnimationPipeline);

		for (const auto& Entity : ECSManager::Instance().GetEntities<AnimationComponent, MeshRenderer>())
		{
			const MeshRenderer& MeshComp = Entity->GetComponent<MeshRenderer>();
			const AnimationComponent& AnimComp = Entity->GetComponent<AnimationComponent>();

			PushConstant pc{};
			pc.m_Model = Entity->GetComponent<Transform>().m_WorldXform;
			vkCmdPushConstants(m_CommandBuffer->GetInUseCommandBuffer(), m_DepthPrepassAnimationPipeline->GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstant), &pc);

			AnimComp.m_ShadowAnimationMaterial->UpdateForAnimationRendering(m_DepthPrepassUBO, Index, AnimComp.m_UBO);
			vkCmdBindDescriptorSets(m_CommandBuffer->GetInUseCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_DepthPrepassAnimationPipeline->GetPipelineLayout(), 0, 1, &AnimComp.m_ShadowAnimationMaterial->GetDescriptor(Index), 0, NULL);

			MeshComp.m_RenderObject->BindAnimation(m_CommandBuffer->GetInUseCommandBuffer());
		}

		Renderer::EndRenderPass(m_CommandBuffer);
	}

	void SceneRenderer::IDPrepass(uint32_t Index, const std::multimap<ResourceHandle, Entity>& MaterialSort)
	{
		VkClearValue clearValues[2];
		clearValues[0].depthStencil = { 1.0f, 0 };
		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = m_IDPrepassRenderPass->GetHandle();
		renderPassInfo.framebuffer = m_IDPrepassFramebuffer;
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = { m_IDPrepassMapWidth, m_IDPrepassMapHeight };
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = clearValues;

		vkCmdBeginRenderPass(m_CommandBuffer->GetInUseCommandBuffer(), &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport2{};
		viewport2.x = 0.0f;
		viewport2.y = 0.0f;
		viewport2.width = (float)m_IDPrepassMapWidth;
		viewport2.height = (float)m_IDPrepassMapHeight;
		viewport2.minDepth = 0.0f;
		viewport2.maxDepth = 1.0f;
		vkCmdSetViewport(m_CommandBuffer->GetInUseCommandBuffer(), 0, 1, &viewport2);

		VkRect2D scissor2{};
		scissor2.extent = { m_IDPrepassMapWidth, m_IDPrepassMapHeight };
		vkCmdSetScissor(m_CommandBuffer->GetInUseCommandBuffer(), 0, 1, &scissor2);

		Renderer::BindPipeline(m_CommandBuffer, m_IDPrepassPipeline);

		m_IDPrepassMaterial->UpdateForRendering(m_IDPrepassUBO, Index);
		vkCmdBindDescriptorSets(m_CommandBuffer->GetInUseCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_IDPrepassPipeline->GetPipelineLayout(), 0, 1, &m_IDPrepassMaterial->GetDescriptor(Index), 0, NULL);

		for (const auto& go_mr : MaterialSort)
		{
			if (go_mr.second->GetComponent<Properties>().m_Tag == "Red" || go_mr.second->GetComponent<Properties>().m_Tag == "Blue")
			{
				const MeshRenderer& mr = go_mr.second->GetComponent<MeshRenderer>();
				ResourceHandle currentMaterialHandle = go_mr.first;

				PushConstant pc{};
				pc.m_Model = go_mr.second->GetComponent<Transform>().m_WorldXform;
				vkCmdPushConstants(m_CommandBuffer->GetInUseCommandBuffer(), m_IDPrepassPipeline->GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstant), &pc);

				mr.m_RenderObject->Bind(m_CommandBuffer->GetInUseCommandBuffer());
				mr.m_RenderObject->Draw(m_CommandBuffer->GetInUseCommandBuffer());

				m_PreviousMaterialHandle = currentMaterialHandle;
			}
		}
		m_PreviousMaterialHandle = 0;

		Renderer::BindPipeline(m_CommandBuffer, m_IDPrepassAnimationPipeline);

		for (const auto& Entity : ECSManager::Instance().GetEntities<AnimationComponent, MeshRenderer>())
		{
			if (Entity->GetComponent<Properties>().m_Tag == "Red" || Entity->GetComponent<Properties>().m_Tag == "Blue")
			{
				const MeshRenderer& MeshComp = Entity->GetComponent<MeshRenderer>();
				const AnimationComponent& AnimComp = Entity->GetComponent<AnimationComponent>();

				PushConstant pc{};
				pc.m_Model = Entity->GetComponent<Transform>().m_WorldXform;
				vkCmdPushConstants(m_CommandBuffer->GetInUseCommandBuffer(), m_IDPrepassAnimationPipeline->GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstant), &pc);

				AnimComp.m_ShadowAnimationMaterial->UpdateForAnimationRendering(m_IDPrepassUBO, Index, AnimComp.m_UBO);
				vkCmdBindDescriptorSets(m_CommandBuffer->GetInUseCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_IDPrepassAnimationPipeline->GetPipelineLayout(), 0, 1, &AnimComp.m_ShadowAnimationMaterial->GetDescriptor(Index), 0, NULL);

				MeshComp.m_RenderObject->BindAnimation(m_CommandBuffer->GetInUseCommandBuffer());
			}
		}

		Renderer::EndRenderPass(m_CommandBuffer);
	}

	void SceneRenderer::BoxBlurPostpass(uint32_t Index)
	{
		VkClearValue clearValues[2];
		clearValues[0].depthStencil = { 1.0f, 0 };
		clearValues[1].color = { 0.0f, 0.0f, 0.0f, 0.0f };
		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = m_BoxBlurPostpassRenderPass->GetHandle();
		renderPassInfo.framebuffer = m_BoxBlurPostpassFramebuffer;
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = { m_BoxBlurPostpassMapWidth, m_BoxBlurPostpassMapHeight };
		renderPassInfo.clearValueCount = 2;
		renderPassInfo.pClearValues = clearValues;

		vkCmdBeginRenderPass(m_CommandBuffer->GetInUseCommandBuffer(), &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport2{};
		viewport2.x = 0.0f;
		viewport2.y = 0.0f;
		viewport2.width = (float)m_BoxBlurPostpassMapWidth;
		viewport2.height = (float)m_BoxBlurPostpassMapHeight;
		viewport2.minDepth = 0.0f;
		viewport2.maxDepth = 1.0f;
		vkCmdSetViewport(m_CommandBuffer->GetInUseCommandBuffer(), 0, 1, &viewport2);

		VkRect2D scissor2{};
		scissor2.extent = { m_BoxBlurPostpassMapWidth, m_BoxBlurPostpassMapHeight };
		vkCmdSetScissor(m_CommandBuffer->GetInUseCommandBuffer(), 0, 1, &scissor2);

		Renderer::BindPipeline(m_CommandBuffer, m_BoxBlurPostpassPipeline);

		m_BoxBlurPostpassMaterial->UpdateForRendering(m_BoxBlurPostpassUBO, Index);
		vkCmdBindDescriptorSets(m_CommandBuffer->GetInUseCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_BoxBlurPostpassPipeline->GetPipelineLayout(), 0, 1, &m_BoxBlurPostpassMaterial->GetDescriptor(Index), 0, NULL);

		VkDeviceSize offsets[] = { 0 };
		auto VB = m_BoxBlurVertexBuffer->GetBuffer();
		vkCmdBindVertexBuffers(m_CommandBuffer->GetInUseCommandBuffer(), 0, 1, &VB, offsets);
		vkCmdBindIndexBuffer(m_CommandBuffer->GetInUseCommandBuffer(), m_BoxBlurIndexBuffer->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);

		vkCmdDrawIndexed(m_CommandBuffer->GetInUseCommandBuffer(), m_BoxBlurIndexBuffer->GetIndexCount(), 1, 0, 0, 0);

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
				// const Transform& tr = cylinder->GetComponent<Transform>();
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

			for (const auto& particles : ECSManager::Instance().GetEntities<Particle2DComponent>())
			{
				const Transform& tr = particles->GetComponent<Transform>();
				const Particle2DComponent& particleComp = particles->GetComponent<Particle2DComponent>();

				if(particleComp.m_Show == false)
					continue;

				PushConstant pc{};
				glm::mat4 model(1.f);
				model = glm::translate(model, tr.m_Position);
				model = glm::scale(model, glm::vec3(particleComp.m_SpawnRadius, particleComp.m_SpawnRadius, particleComp.m_SpawnRadius));
				pc.m_Model = model;

				vkCmdPushConstants(m_CommandBuffer->GetInUseCommandBuffer(), m_DebugRenderer->GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstant), &pc);
				vkCmdBindDescriptorSets(m_CommandBuffer->GetInUseCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_DebugRenderer->GetPipelineLayout(), 0, 1, &m_DebugRenderer->GetDescriptor(Index), 0, NULL);

				m_DebugRenderer->BindDebugSphere(m_CommandBuffer->GetInUseCommandBuffer());
				m_DebugRenderer->DrawDebugSphere(m_CommandBuffer->GetInUseCommandBuffer());
			}
		}
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

		if (m_SceneImages.contains(SceneImage::ShadowMap) == false)
			m_SceneImages[SceneImage::ShadowMap] = std::make_shared<Image2D>(ImgConfig);

		if (m_SceneImages.contains(SceneImage::shadowMap2) == false)
			m_SceneImages[SceneImage::shadowMap2] = std::make_shared<Image2D>(ImgConfig);

		m_ShadowRenderPass = std::make_shared<RenderPass>(m_Device, true);

		auto attachments = m_SceneImages[SceneImage::ShadowMap]->GetImageData().ImageView;
		VkFramebufferCreateInfo framebufferCreateInfo{};
		framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferCreateInfo.renderPass = m_ShadowRenderPass->GetHandle();
		framebufferCreateInfo.attachmentCount = 1;
		framebufferCreateInfo.pAttachments = &attachments;
		framebufferCreateInfo.width = m_ShadowMapWidth;
		framebufferCreateInfo.height = m_ShadowMapHeight;
		framebufferCreateInfo.layers = 1;

		if (auto Result = vkCreateFramebuffer(m_Device->GetLogicalDevice(), &framebufferCreateInfo, nullptr, &m_ShadowFramebuffer[0]); Result != VK_SUCCESS)
		{
			assert(Result == VK_SUCCESS && "Unable to create image sampler for shadow");
		}


		auto attachments2 = m_SceneImages[SceneImage::shadowMap2]->GetImageData().ImageView;
		VkFramebufferCreateInfo framebufferCreateInfo2{};
		framebufferCreateInfo2.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferCreateInfo2.renderPass = m_ShadowRenderPass->GetHandle();
		framebufferCreateInfo2.attachmentCount = 1;
		framebufferCreateInfo2.width = m_ShadowMapWidth;
		framebufferCreateInfo2.height = m_ShadowMapHeight;
		framebufferCreateInfo2.layers = 1;
		framebufferCreateInfo2.pAttachments = &attachments2;

		if (auto Result = vkCreateFramebuffer(m_Device->GetLogicalDevice(), &framebufferCreateInfo2, nullptr, &m_ShadowFramebuffer[1]); Result != VK_SUCCESS)
		{
			assert(Result == VK_SUCCESS && "Unable to create image sampler for shadow");
		}

		constexpr float minFloat = std::numeric_limits<float>::min();
		constexpr float maxFloat = std::numeric_limits<float>::max();
		m_ShadowAABBMin = glm::vec3(maxFloat, maxFloat, maxFloat);
		m_ShadowAABBMax = glm::vec3(minFloat, minFloat, minFloat);
		m_EditorShadowAABBMin = glm::vec3(maxFloat, maxFloat, maxFloat);
		m_EditorShadowAABBMax = glm::vec3(minFloat, minFloat, minFloat);
	}

	void SceneRenderer::DepthPrepassInit()
	{
		m_DepthPrepassMapWidth = 4096;
		m_DepthPrepassMapHeight = 4096;

		ImageConfig ImgConfig{};
		ImgConfig.DebugName = "Depth Pass";
		ImgConfig.Format = ImageFormat::DEPTH16UN;
		ImgConfig.Width = m_DepthPrepassMapWidth;
		ImgConfig.Height = m_DepthPrepassMapHeight;
		ImgConfig.Usage = ImageUsage::Attachment;
		ImgConfig.CreateSampler = true;
		ImgConfig.Transfer = false;
		ImgConfig.AddressMode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;

		if (m_SceneImages.contains(SceneImage::DepthMap) == false)
			m_SceneImages[SceneImage::DepthMap] = std::make_shared<Image2D>(ImgConfig);
		m_DepthPrepassRenderPass = std::make_shared<RenderPass>(m_Device, true);

		auto attachments = m_SceneImages[SceneImage::DepthMap]->GetImageData().ImageView;
		VkFramebufferCreateInfo framebufferCreateInfo{};
		framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferCreateInfo.renderPass = m_DepthPrepassRenderPass->GetHandle();
		framebufferCreateInfo.attachmentCount = 1;
		framebufferCreateInfo.pAttachments = &attachments;
		framebufferCreateInfo.width = m_DepthPrepassMapWidth;
		framebufferCreateInfo.height = m_DepthPrepassMapHeight;
		framebufferCreateInfo.layers = 1;

		if (auto Result = vkCreateFramebuffer(m_Device->GetLogicalDevice(), &framebufferCreateInfo, nullptr, &m_DepthPrepassFramebuffer); Result != VK_SUCCESS)
		{
			assert(Result == VK_SUCCESS && "Unable to create image sampler for depth pass");
		}
	}

	void SceneRenderer::IDPrepassInit()
	{
		m_IDPrepassMapWidth = 4096;
		m_IDPrepassMapHeight = 4096;

		ImageConfig ImgConfig{};
		ImgConfig.DebugName = "ID Pass";
		ImgConfig.Format = ImageFormat::DEPTH16UN;
		ImgConfig.Width = m_IDPrepassMapWidth;
		ImgConfig.Height = m_IDPrepassMapHeight;
		ImgConfig.Usage = ImageUsage::Attachment;
		ImgConfig.CreateSampler = true;
		ImgConfig.Transfer = false;
		ImgConfig.AddressMode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;

		if (m_SceneImages.contains(SceneImage::IDMap) == false)
			m_SceneImages[SceneImage::IDMap] = std::make_shared<Image2D>(ImgConfig);
		m_IDPrepassRenderPass = std::make_shared<RenderPass>(m_Device, true);

		auto attachments = m_SceneImages[SceneImage::IDMap]->GetImageData().ImageView;
		VkFramebufferCreateInfo framebufferCreateInfo{};
		framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferCreateInfo.renderPass = m_IDPrepassRenderPass->GetHandle();
		framebufferCreateInfo.attachmentCount = 1;
		framebufferCreateInfo.pAttachments = &attachments;
		framebufferCreateInfo.width = m_IDPrepassMapWidth;
		framebufferCreateInfo.height = m_IDPrepassMapHeight;
		framebufferCreateInfo.layers = 1;

		if (auto Result = vkCreateFramebuffer(m_Device->GetLogicalDevice(), &framebufferCreateInfo, nullptr, &m_IDPrepassFramebuffer); Result != VK_SUCCESS)
		{
			assert(Result == VK_SUCCESS && "Unable to create image sampler for ID pass");
		}
	}

	void SceneRenderer::BoxBlurPostpassInit()
	{
		m_BoxBlurPostpassMapWidth = 512;
		m_BoxBlurPostpassMapHeight = 512;

		//Color
		ImageConfig ImgConfig{};
		ImgConfig.DebugName = "Box Blur Pass";
		ImgConfig.Format = ImageFormat::RGBA;
		ImgConfig.Width = m_BoxBlurPostpassMapWidth;
		ImgConfig.Height = m_BoxBlurPostpassMapHeight;
		ImgConfig.Usage = ImageUsage::Attachment;
		ImgConfig.CreateSampler = true;
		ImgConfig.Transfer = false;
		ImgConfig.AddressMode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;

		if (m_SceneImages.contains(SceneImage::BoxBlurMap) == false)
			m_SceneImages[SceneImage::BoxBlurMap] = std::make_shared<Image2D>(ImgConfig);

		RenderPassInfo RenderPassCreateInfo{};
		RenderPassCreateInfo.FinalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		RenderPassCreateInfo.ImageFormat = Engine::GetInstance().GetWindow()->GetSwapChain()->GetColorFormat();
		RenderPassCreateInfo.DepthEnabled = false;
		m_BoxBlurPostpassRenderPass = std::make_shared<RenderPass>(m_Device, RenderPassCreateInfo);

		//std::array<VkImageView,2> attachments = { m_SceneImages[SceneImage::BoxBlurMap]->GetImageData().ImageView, m_SceneImages[SceneImage::BoxBlurDepthMap]->GetImageData().ImageView };
		auto attachments = m_SceneImages[SceneImage::BoxBlurMap]->GetImageData().ImageView;
		VkFramebufferCreateInfo framebufferCreateInfo{};
		framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferCreateInfo.renderPass = m_BoxBlurPostpassRenderPass->GetHandle();
		framebufferCreateInfo.attachmentCount = 1;//static_cast<uint32_t>(attachments.size());
		framebufferCreateInfo.pAttachments = &attachments;//attachments.data();
		framebufferCreateInfo.width = m_BoxBlurPostpassMapWidth;
		framebufferCreateInfo.height = m_BoxBlurPostpassMapHeight;
		framebufferCreateInfo.layers = 1;

		if (auto Result = vkCreateFramebuffer(m_Device->GetLogicalDevice(), &framebufferCreateInfo, nullptr, &m_BoxBlurPostpassFramebuffer); Result != VK_SUCCESS)
		{
			assert(Result == VK_SUCCESS && "Unable to create image sampler for Box blur pass");
		}
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