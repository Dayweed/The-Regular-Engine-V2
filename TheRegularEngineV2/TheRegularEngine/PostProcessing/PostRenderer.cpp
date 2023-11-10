#include "pch.h"
#include "PostRenderer.h"
#include "Graphics/Renderer.h"
#include "Core/Engine.h"

namespace TRE
{
	PostProcessingRenderer::PostProcessingRenderer(const std::shared_ptr<Device>& Device) : m_Device(Device)
	{
		auto& SC = Engine::GetInstance().GetWindow()->GetSwapChain();
		RenderPassInfo RPConfig{};
		RPConfig.FinalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		RPConfig.ImageFormat = SC->GetColorFormat();
		RPConfig.DepthFinalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		RPConfig.DepthImageFormat = SC->GetDepthFormat();
		RPConfig.DepthEnabled = true;
		RPConfig.ClearColor = false;
		m_PostRenderpass = std::make_shared<RenderPass>(m_Device, RPConfig);

		PipelineConfigurations PipelineConfig{};
		PipelineConfig.Primitive = PrimitiveType::Triangles;
		PipelineConfig.Shader = ResourceManager::Instance().GetResource<Shader>(8); //Vignette shader
		PipelineConfig.EnableCull = false;
		PipelineConfig.EnableBlending = true;
		PipelineConfig.EnableDepthTest = false;
		m_PostPipeline = std::make_shared<Pipeline>(PipelineConfig, m_PostRenderpass);

		m_PostEffects[1] = std::make_shared<Vignette>();
		for (auto& effects : m_PostEffects)
		{
			effects.second->SetupUBO();
			effects.second->SetupShader(PipelineConfig.Shader);
		}

		float x = -1.f; float y = -1.f;
		float width = 2, height = 2;
		std::vector<PostVertex> data(4);

		data[0].Position = glm::vec2(x, y);
		data[0].UV = glm::vec2(0, 0);

		data[1].Position = glm::vec2(x + width, y);
		data[1].UV = glm::vec2(1, 0);

		data[2].Position = glm::vec2(x + width, y + height);
		data[2].UV = glm::vec2(1, 1);

		data[3].Position = glm::vec2(x, y + height);
		data[3].UV = glm::vec2(0, 1);

		std::vector<int> indices = { 0,1,2,2,3,0 };

		m_PostIndexBuffer = std::make_shared<IndexBuffer>((void*)indices.data(), sizeof(int) * indices.size(), indices.size());
		m_PostVertexBuffer = std::make_shared<VertexBuffer>((void*)data.data(), data.size() * sizeof(PostVertex));
	}

	PostProcessingRenderer::~PostProcessingRenderer()
	{

	}

	void PostProcessingRenderer::Render(VkFramebuffer TargetFramebuffer, const std::shared_ptr<CommandBuffer>& CommandBuffer)
	{
		for (const auto& effects : m_PostEffects)
		{
			effects.second->UpdateUBO();
		}

		const auto Index = Engine::GetInstance().GetWindow()->GetSwapChain()->GetCurrentBufferIndex();

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = m_PostRenderpass->GetHandle();
		renderPassInfo.framebuffer = TargetFramebuffer;
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = Engine::GetInstance().GetWindow()->GetSwapChain()->GetSwapChainExtent();

		vkCmdBeginRenderPass(CommandBuffer->GetInUseCommandBuffer(), &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport{};
		viewport.x = 0.f;
		viewport.y = static_cast<float>(Engine::GetInstance().GetWindow()->GetSwapChain()->GetHeight());
		viewport.width = static_cast<float>(Engine::GetInstance().GetWindow()->GetSwapChain()->GetWidth());
		viewport.height = -static_cast<float>(Engine::GetInstance().GetWindow()->GetSwapChain()->GetHeight());
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(CommandBuffer->GetInUseCommandBuffer(), 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = Engine::GetInstance().GetWindow()->GetSwapChain()->GetSwapChainExtent();
		vkCmdSetScissor(CommandBuffer->GetInUseCommandBuffer(), 0, 1, &scissor);

		Renderer::BindPipeline(CommandBuffer, m_PostPipeline);
		for (const auto& effects : m_PostEffects)
		{
			effects.second->Render(m_PostPipeline, CommandBuffer, Index);

			VkDeviceSize offsets[] = { 0 };
			auto VB = m_PostVertexBuffer->GetBuffer();
			vkCmdBindVertexBuffers(CommandBuffer->GetInUseCommandBuffer(), 0, 1, &VB, offsets);
			vkCmdBindIndexBuffer(CommandBuffer->GetInUseCommandBuffer(), m_PostIndexBuffer->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);

			vkCmdDrawIndexed(CommandBuffer->GetInUseCommandBuffer(), m_PostIndexBuffer->GetIndexCount(), 1, 0, 0, 0);
		}

		Renderer::EndRenderPass(CommandBuffer);
	}

	void PostProcessingRenderer::AddPostEffect(const std::shared_ptr<PostProcessEffect>& effect, const int index, const std::string name)
	{
		/*m_PostEffects[index] = effect;
		m_PostEffects[index]->SetupUBO();
		m_PostEffects[index]->SetupShader(m_PostPipeline->GetShader());
		m_PostEffects[index]->SetName(name);*/
	}
}