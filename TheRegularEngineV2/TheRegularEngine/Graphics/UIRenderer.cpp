#include "pch.h"
#include "UIRenderer.h"
#include "Renderer.h"
#include "Core/ECS.h"
#include "UIComponent.h"
#include "Core/Engine.h"
#include "Resource/ResourceManager.h"
#include "Core/Transform.h"
#include "EditorCamera.h"
#include "glm/gtc/matrix_transform.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "glm/gtx/quaternion.hpp"
#include "Core/Logger.h"

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
		PipelineConfig.EnableCull = false;
		m_UIPipeline = std::make_shared<Pipeline>(PipelineConfig, m_UIRenderpass);

		m_UIUBO = std::make_shared<UniformBuffer>(sizeof(UIUBO), 0);

		float x = -1.f; float y = -1.f;
		float width = 2, height = 2;
		std::vector<UIVertex> data(4);

		data[0].Position = glm::vec3(x, y, 0.0f);
		data[0].UV = glm::vec2(0, 0);

		data[1].Position = glm::vec3(x + width, y, 0.0f);
		data[1].UV = glm::vec2(1, 0);

		data[2].Position = glm::vec3(x + width, y + height, 0.0f);
		data[2].UV = glm::vec2(1, 1);

		data[3].Position = glm::vec3(x, y + height, 0.0f);
		data[3].UV = glm::vec2(0, 1);

		std::vector<int> indices = { 0,1,2,2,3,0 };

		m_TestIndexBuffer = std::make_shared<IndexBuffer>((void*)indices.data(), sizeof(int) * indices.size(), indices.size());
		m_TestVertexBuffer = std::make_shared<VertexBuffer>((void*)data.data(), data.size() * sizeof(UIVertex));

		m_UIMaterial = std::make_shared<Material>(PipelineConfig.Shader);
		m_UIMaterial->Invalidate();

		auto TextureHandle = Resource::GetGUIDFromHex("d180b66ce70dea24");
		auto Texture1 = ResourceManager::Instance().GetResource<VulkanTexture>(TextureHandle);
		m_UIMaterial->SetTexture("UI_Texture", Texture1);
	}

	UIRenderer::~UIRenderer()
	{

	}

	void UIRenderer::Render(VkFramebuffer TargetFramebuffer, const std::shared_ptr<CommandBuffer>& CommandBuffer)
	{
		UIUBO UBO{};
		glm::mat4 TranslateToMid = glm::translate(glm::identity<glm::mat4>(), glm::vec3(960.f, 540.f, 0.f)); //Translate by viewport width or height / 2
		UBO.m_ProjView2DSpace = glm::ortho(0.f, 1920.f, 0.f, 1080.f) * TranslateToMid;

		m_UIUBO->SetData(&UBO, sizeof(UIUBO));

		auto Index = Engine::GetInstance().GetWindow()->GetSwapChain()->GetCurrentBufferIndex();

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = m_UIRenderpass->GetHandle();
		renderPassInfo.framebuffer = TargetFramebuffer;
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = Engine::GetInstance().GetWindow()->GetSwapChain()->GetSwapChainExtent();

		vkCmdBeginRenderPass(CommandBuffer->GetInUseCommandBuffer(), &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.f;
		viewport.width = static_cast<float>(Engine::GetInstance().GetWindow()->GetSwapChain()->GetWidth());
		viewport.height = static_cast<float>(Engine::GetInstance().GetWindow()->GetSwapChain()->GetHeight());
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(CommandBuffer->GetInUseCommandBuffer(), 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = Engine::GetInstance().GetWindow()->GetSwapChain()->GetSwapChainExtent();
		vkCmdSetScissor(CommandBuffer->GetInUseCommandBuffer(), 0, 1, &scissor);

		Renderer::BindPipeline(CommandBuffer, m_UIPipeline);
		for (const auto& Entity : ECSManager::Instance().GetEntities<UIComponent>())
		{
			auto& UIComp = Entity->GetComponent<UIComponent>();
			if (!UIComp.m_IsVisible)
				continue;

			UI_PushConstant pc{};
			auto TransformComp = Entity->GetComponent<Transform>();
			pc.L2W = TransformComp.m_WorldXform;
			pc.L2W = glm::scale(pc.L2W, glm::vec3(UIComp.m_Width, UIComp.m_Height, 0.f));
			pc.Color = UIComp.m_Color;

			vkCmdPushConstants(CommandBuffer->GetInUseCommandBuffer(), m_UIPipeline->GetPipelineLayout(), 
				VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(UI_PushConstant), &pc);

			if (UIComp.m_Texture)
				m_UIMaterial->SetTexture("UI_Texture", UIComp.m_Texture);

			m_UIMaterial->UpdateForEditorSceneRendering(m_UIUBO, Index);

			vkCmdBindDescriptorSets(CommandBuffer->GetInUseCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_UIPipeline->GetPipelineLayout(), 0, 1, &m_UIMaterial->GetEditorDescriptor(Index), 0, NULL);

			VkDeviceSize offsets[] = { 0 };
			auto VB = m_TestVertexBuffer->GetBuffer();
			vkCmdBindVertexBuffers(CommandBuffer->GetInUseCommandBuffer(), 0, 1, &VB, offsets);
			vkCmdBindIndexBuffer(CommandBuffer->GetInUseCommandBuffer(), m_TestIndexBuffer->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);

			vkCmdDrawIndexed(CommandBuffer->GetInUseCommandBuffer(), m_TestIndexBuffer->GetIndexCount(), 1, 0, 0, 0);
		}

		Renderer::EndRenderPass(CommandBuffer);
	}
}