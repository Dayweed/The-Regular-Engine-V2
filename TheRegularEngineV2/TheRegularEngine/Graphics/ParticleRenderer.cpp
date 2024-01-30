#include "pch.h"
#include "ParticleRenderer.h"
#include "Core/Engine.h"
#include "Core/ECS.h"
#include "Particle.h"

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
		m_Renderpass = std::make_shared<RenderPass>(m_Device, RPConfig);

		PipelineConfigurations PipelineConfig{};
		PipelineConfig.Primitive = PrimitiveType::Triangles;
		PipelineConfig.Shader = ResourceManager::Instance().GetResource<Shader>(6);
		PipelineConfig.CullMode = VK_CULL_MODE_NONE;
		PipelineConfig.EnableBlending = true;
		PipelineConfig.EnableDepthTest = true;
		m_Pipeline = std::make_shared<Pipeline>(PipelineConfig, m_Renderpass);

		//m_UBO = std::make_shared<UniformBuffer>(UINT32_T_CAST(sizeof(UIUBO)), 0);

		float x = -1.f; float y = -1.f;
		float width = 2, height = 2;
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
	}

	void ParticleRenderer::Render(VkFramebuffer targetFramebuffer, const std::shared_ptr<CommandBuffer>& commandBuffer, bool isEditor)
	{
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

		Renderer::BindPipeline(commandBuffer, m_Pipeline);

		for (const auto& emitter : ECSManager::Instance().GetEntities<ParticleComponent>())
		{
			const ParticleComponent& particleComp = emitter->GetComponent<ParticleComponent>();
			//for(particleComp.)
		}
	}
}