#include "pch.h"
#include "UIRenderer.h"
#include "Renderer.h"
#include "Core/ECS.h"
#include "UIComponent.h"
#include "Core/Engine.h"
#include "Resource/ResourceManager.h"
#include "Core/Transform.h"

namespace TRE
{
	UIRenderer::UIRenderer(const std::shared_ptr<Device>& Device) : m_Device(Device)
	{
		auto SC = Engine::GetInstance().GetWindow()->GetSwapChain();
		RenderPassInfo RPConfig{};
		RPConfig.FinalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		RPConfig.ImageFormat = SC->GetColorFormat();
		RPConfig.DepthFinalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		RPConfig.DepthImageFormat = SC->GetDepthFormat();
		RPConfig.DepthEnabled = true;
		RPConfig.ClearColor = false;
		m_UIRenderpass = std::make_shared<RenderPass>(m_Device, RPConfig);

		PipelineConfigurations PipelineConfig{};
		PipelineConfig.Primitive = PrimitiveType::Triangles;
		PipelineConfig.Shader = ResourceManager::Instance().GetResource<Shader>(6);
		m_UIPipeline = std::make_shared<Pipeline>(PipelineConfig, m_UIRenderpass);

		m_UIUBO = std::make_shared<UniformBuffer>(sizeof(UIUBO), 0);
	}

	UIRenderer::~UIRenderer()
	{

	}

	void UIRenderer::Render(VkFramebuffer TargetFramebuffer, const std::shared_ptr<CommandBuffer>& CommandBuffer)
	{
		auto Index = Engine::GetInstance().GetWindow()->GetSwapChain()->GetCurrentBufferIndex();
		
		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = m_UIRenderpass->GetHandle();
		renderPassInfo.framebuffer = TargetFramebuffer;
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = Engine::GetInstance().GetWindow()->GetSwapChain()->GetSwapChainExtent();

		vkCmdBeginRenderPass(CommandBuffer->GetInUseCommandBuffer(), &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		Renderer::BindPipeline(CommandBuffer, m_UIPipeline);
		for (const auto& Entity : ECSManager::Instance().GetEntities<UIComponent>())
		{
			PushConstant pc{};
			pc.m_Model = Entity->GetComponent<Transform>().m_WorldXform;
			vkCmdPushConstants(CommandBuffer->GetInUseCommandBuffer(), m_UIPipeline->GetPipelineLayout(), 
				VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstant), &pc);

			const auto& Material = Entity->GetComponent<UIComponent>().m_Material;
			Material->UpdateForEditorSceneRendering(m_UIUBO, Index);

			vkCmdBindDescriptorSets(CommandBuffer->GetInUseCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_UIPipeline->GetPipelineLayout(), 0, 1, &Material->GetEditorDescriptor(Index), 0, NULL);
		}

		Renderer::EndRenderPass(CommandBuffer);
	}
}