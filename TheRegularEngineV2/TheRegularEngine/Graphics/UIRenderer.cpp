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

		float x = -1; float y = -1;
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

		m_TestMaterial = std::make_shared<Material>(PipelineConfig.Shader);
		m_TestMaterial->Invalidate();
	}

	UIRenderer::~UIRenderer()
	{

	}

	void UIRenderer::Render(VkFramebuffer TargetFramebuffer, const std::shared_ptr<CommandBuffer>& CommandBuffer)
	{
		auto Index = Engine::GetInstance().GetWindow()->GetSwapChain()->GetCurrentBufferIndex();
		auto Skybox1 = Resource::GetGUIDFromHex("e562694e2c3833ec");
		auto Texture1 = ResourceManager::Instance().GetResource<VulkanTexture>(Skybox1);

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
			if (!Entity->GetComponent<UIComponent>().m_IsVisible)
				continue;

			PushConstant pc{};
			pc.m_Model = Entity->GetComponent<Transform>().m_WorldXform;
			vkCmdPushConstants(CommandBuffer->GetInUseCommandBuffer(), m_UIPipeline->GetPipelineLayout(), 
				VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstant), &pc);

			auto& Material = Entity->GetComponent<UIComponent>().m_Material;
			if (Material == nullptr)
				Material = m_TestMaterial;

			m_TestMaterial->SetTexture("UI_Texture", Texture1);
			m_TestMaterial->UpdateForEditorSceneRendering(m_UIUBO, Index);

			vkCmdBindDescriptorSets(CommandBuffer->GetInUseCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_UIPipeline->GetPipelineLayout(), 0, 1, &Material->GetEditorDescriptor(Index), 0, NULL);

			VkDeviceSize offsets[] = { 0 };
			auto VB = m_TestVertexBuffer->GetBuffer();
			vkCmdBindVertexBuffers(CommandBuffer->GetInUseCommandBuffer(), 0, 1, &VB, offsets);
			vkCmdBindIndexBuffer(CommandBuffer->GetInUseCommandBuffer(), m_TestIndexBuffer->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);

			vkCmdDrawIndexed(CommandBuffer->GetInUseCommandBuffer(), m_TestIndexBuffer->GetIndexCount(), 1, 0, 0, 0);
		}

		Renderer::EndRenderPass(CommandBuffer);
	}
}