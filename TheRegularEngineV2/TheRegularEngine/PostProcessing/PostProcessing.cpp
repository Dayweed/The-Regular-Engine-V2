#include "pch.h"
#include "PostProcessing.h"
#include "Graphics/Renderer.h"
#include "Core/Engine.h"
#include "Vignette.h"

namespace TRE
{
	PostProcessEffect::PostProcessEffect()
	{
		auto& SC = Engine::GetInstance().GetWindow()->GetSwapChain();
		const auto& device = RendererContext::GetDevice();
		RenderPassInfo RPConfig{};
		RPConfig.FinalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		RPConfig.ImageFormat = SC->GetColorFormat();
		RPConfig.DepthFinalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		RPConfig.DepthImageFormat = SC->GetDepthFormat();
		RPConfig.DepthEnabled = true;
		RPConfig.ClearColor = false;
		m_Renderpass = std::make_shared<RenderPass>(device, RPConfig);

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

		m_IndexBuffer = std::make_shared<IndexBuffer>((void*)indices.data(), sizeof(int) * indices.size(), indices.size());
		m_VertexBuffer = std::make_shared<VertexBuffer>((void*)data.data(), data.size() * sizeof(PostVertex));
	}

	void PostProcessEffect::Render(VkFramebuffer targetFramebuffer, const std::shared_ptr<CommandBuffer>& commandBuffer, const int index)
	{
		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = m_Renderpass->GetHandle();
		renderPassInfo.framebuffer = targetFramebuffer;
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = Engine::GetInstance().GetWindow()->GetSwapChain()->GetSwapChainExtent();

		vkCmdBeginRenderPass(commandBuffer->GetInUseCommandBuffer(), &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport{};
		viewport.x = 0.f;
		viewport.y = static_cast<float>(Engine::GetInstance().GetWindow()->GetSwapChain()->GetHeight());
		viewport.width = static_cast<float>(Engine::GetInstance().GetWindow()->GetSwapChain()->GetWidth());
		viewport.height = -static_cast<float>(Engine::GetInstance().GetWindow()->GetSwapChain()->GetHeight());
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(commandBuffer->GetInUseCommandBuffer(), 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = Engine::GetInstance().GetWindow()->GetSwapChain()->GetSwapChainExtent();
		vkCmdSetScissor(commandBuffer->GetInUseCommandBuffer(), 0, 1, &scissor);

		Renderer::BindPipeline(commandBuffer, m_Pipeline);
		m_Material->UpdateForRendering(m_UBO, index);
		vkCmdBindDescriptorSets(commandBuffer->GetInUseCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline->GetPipelineLayout(), 0, 1, &m_Material->GetDescriptor(index), 0, NULL);

		VkDeviceSize offsets[] = { 0 };
		auto VB = m_VertexBuffer->GetBuffer();
		vkCmdBindVertexBuffers(commandBuffer->GetInUseCommandBuffer(), 0, 1, &VB, offsets);
		vkCmdBindIndexBuffer(commandBuffer->GetInUseCommandBuffer(), m_IndexBuffer->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);

		vkCmdDrawIndexed(commandBuffer->GetInUseCommandBuffer(), m_IndexBuffer->GetIndexCount(), 1, 0, 0, 0);

		Renderer::EndRenderPass(commandBuffer);
	}

	void PostProcessingManager::Init()
	{
		m_PostEffects["Vignette"] = std::move(std::pair(1, std::make_shared<Vignette>()));
	}

	void PostProcessingManager::Render(VkFramebuffer targetFramebuffer, const std::shared_ptr<CommandBuffer>& commandBuffer, const int index)
	{
		for (const auto& effects : m_PostEffects)
		{
			effects.second.second->UpdateUBO();
			effects.second.second->Render(targetFramebuffer, commandBuffer, index);
		}
	}

	void PostProcessingManager::Shutdown()
	{
		m_PostEffects.clear();
	}

	void PostProcessingManager::AddPostEffect(std::shared_ptr<PostProcessEffect> effect, const int index, const std::string name)
	{
		m_PostEffects[name] = std::pair(index, effect);
	}

	void PostProcessingManager::RemovePostEffect(const std::string& name)
	{
		if (m_PostEffects.find(name) != m_PostEffects.end())
		{
			m_PostEffects.erase(name);
		}
	}
}