#include "pch.h"
#include "ParticleRenderer.h"
#include "Core/Engine.h"
#include "Core/ECS.h"
#include "Particle.h"
#include "Camera.h"
#include "Core/ECS.h"

namespace TRE
{
	ParticleRenderer::ParticleRenderer(const std::shared_ptr<Device>& device)
	{
		auto SC = Engine::GetInstance().GetWindow()->GetSwapChain();
		RenderPassInfo RPConfig{};
		RPConfig.FinalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		RPConfig.ImageFormat = SC->GetColorFormat();
		RPConfig.DepthFinalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		RPConfig.DepthImageFormat = SC->GetDepthFormat();
		RPConfig.DepthEnabled = true;
		RPConfig.ClearColor = false;
		m_Renderpass = std::make_shared<RenderPass>(device, RPConfig);

		PipelineConfigurations PipelineConfig{};
		PipelineConfig.Primitive = PrimitiveType::Triangles;
		PipelineConfig.Shader = ResourceManager::Instance().GetResource<Shader>(12);
		PipelineConfig.CullMode = VK_CULL_MODE_NONE;
		PipelineConfig.EnableBlending = true;
		PipelineConfig.EnableDepthTest = false;
		m_Pipeline = std::make_shared<Pipeline>(PipelineConfig, m_Renderpass);

		m_UBO = std::make_shared<UniformBuffer>(UINT32_T_CAST(sizeof(ParticleUBO)), 0);

		float x = -0.5f; float y = -0.5f;
		float width = 1, height = 1;
		std::vector<QuadVertex> data(4);

		data[0].Position = glm::vec3(x, y, 0.0f);
		data[0].TexCoord = glm::vec2(0, 0);

		data[1].Position = glm::vec3(x + width, y, 0.0f);
		data[1].TexCoord = glm::vec2(1, 0);

		data[2].Position = glm::vec3(x + width, y + height, 0.0f);
		data[2].TexCoord = glm::vec2(1, 1);

		data[3].Position = glm::vec3(x, y + height, 0.0f);
		data[3].TexCoord = glm::vec2(0, 1);

		std::vector<int> indices = { 0,1,2,2,3,0 };

		m_IndexBuffer = std::make_shared<IndexBuffer>(static_cast<void*>(indices.data()),
			UINT32_T_CAST(sizeof(int) * indices.size()),
			UINT32_T_CAST(indices.size()));

		m_VertexBuffer = std::make_shared<VertexBuffer>(static_cast<void*>(data.data()),
			UINT32_T_CAST(data.size() * sizeof(QuadVertex)));
	
		m_Material = std::make_shared<Material>(ResourceManager::Instance().GetResource<Shader>(12));
		m_Material->Invalidate();

	}

	ParticleRenderer::~ParticleRenderer()
	{

	}

	void ParticleRenderer::Render(VkFramebuffer targetFramebuffer, const std::shared_ptr<CommandBuffer>& commandBuffer, bool isEditor)
	{
		ParticleUBO ubo{};
		const Camera& mainCamera = ECSSystemManager::Instance().GetSystem<CameraSystem>()->GetMainCamera()->GetComponent<Camera>();
		ubo.ProjView = mainCamera.m_BaseCamera.m_ProjectionMatrix * mainCamera.m_BaseCamera.m_ViewMatrix;
		ubo.Color = glm::vec4(1.f, 1.f, 1.f, 1.f);

		m_UBO->SetData(&ubo, sizeof(ParticleUBO));

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = m_Renderpass->GetHandle();
		renderPassInfo.framebuffer = targetFramebuffer;
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = Engine::GetInstance().GetWindow()->GetSwapChain()->GetSwapChainExtent();

		vkCmdBeginRenderPass(commandBuffer->GetInUseCommandBuffer(), &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.f;
		viewport.width = static_cast<float>(Engine::GetInstance().GetWindow()->GetSwapChain()->GetWidth());
		viewport.height = static_cast<float>(Engine::GetInstance().GetWindow()->GetSwapChain()->GetHeight());
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(commandBuffer->GetInUseCommandBuffer(), 0, 1, &viewport);

		const auto& SC = Engine::GetInstance().GetWindow()->GetSwapChain();

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = SC->GetSwapChainExtent();
		vkCmdSetScissor(commandBuffer->GetInUseCommandBuffer(), 0, 1, &scissor);

		auto index = Engine::GetInstance().GetWindow()->GetSwapChain()->GetCurrentBufferIndex();

		Renderer::BindPipeline(commandBuffer, m_Pipeline);
		m_Material->UpdateForRendering(m_UBO, index);
		vkCmdBindDescriptorSets(commandBuffer->GetInUseCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline->GetPipelineLayout(), 0, 1, &m_Material->GetDescriptor(index), 0, NULL);
		for (const auto& emitter : ECSManager::Instance().GetEntities<ParticleComponent>())
		{
			const ParticleComponent& particleComp = emitter->GetComponent<ParticleComponent>();
			if (particleComp.m_Running)
			{
				for (auto& particle : particleComp.m_Particles)
				{
					Particle_PushConstant pc{};
					pc.L2W = particle.L2W;

					vkCmdPushConstants(commandBuffer->GetInUseCommandBuffer(), m_Pipeline->GetPipelineLayout(),
						VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(Particle_PushConstant), &pc);

					VkDeviceSize offsets[] = { 0 };
					VkBuffer VB = m_VertexBuffer->GetBuffer();

					vkCmdBindVertexBuffers(commandBuffer->GetInUseCommandBuffer(), 0, 1, &VB, offsets);
					vkCmdBindIndexBuffer(commandBuffer->GetInUseCommandBuffer(), m_IndexBuffer->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);

					vkCmdDrawIndexed(commandBuffer->GetInUseCommandBuffer(), m_IndexBuffer->GetIndexCount(), 1, 0, 0, 0);
				}
			}
		}

		Renderer::EndRenderPass(commandBuffer);
	}
}