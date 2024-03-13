#include "pch.h"
#include "FontRenderer.h"
#include "Core/Logger.h"
#include "Device.h"
#include "Core/Engine.h"
#include "ECS/Components/TextComponent.h"
#include "ECS/ECS.h"
#include "ECS/Components/Transform.h"
#include "FontManager.h"
#include "Font.h"
#include "Resource/ResourceManager.h"
#include "Resource/Resource.h"
#include <ft2build.h>
#include FT_FREETYPE_H

namespace TRE
{
	static constexpr float s_DefaultFontSize = 64.f;

	FontRenderer::FontRenderer(const std::shared_ptr<Device>& Device) : m_Device(Device)
	{
		auto SC = Engine::GetInstance().GetWindow()->GetSwapChain();
		RenderPassInfo RPConfig{};
		RPConfig.FinalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		RPConfig.ImageFormat = SC->GetColorFormat();
		RPConfig.DepthFinalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		RPConfig.DepthImageFormat = SC->GetDepthFormat();
		RPConfig.DepthEnabled = true;
		RPConfig.ClearColor = false;
		m_FontRenderPass = std::make_shared<RenderPass>(m_Device, RPConfig);

		PipelineConfigurations FontPipeConfig{};
		FontPipeConfig.Primitive = PrimitiveType::Triangles;
		FontPipeConfig.CullMode = VK_CULL_MODE_NONE;
		FontPipeConfig.EnableBlending = true;
		FontPipeConfig.EnableDepthTest = false;
		FontPipeConfig.Shader = ResourceManager::Instance().GetResource<Shader>(11);
		m_FontPipeline = std::make_shared<Pipeline>(FontPipeConfig, m_FontRenderPass);

		std::vector<int> indices = { 0,1,2,2,3,0 };
		m_FontIndexBuffer = std::make_shared<IndexBuffer>(static_cast<void*>(indices.data()), UINT32_T_CAST(sizeof(int) * indices.size()), UINT32_T_CAST(indices.size()));
	}

	FontRenderer::~FontRenderer()
	{

	}

	void FontRenderer::RenderFont(VkFramebuffer TargetFramebuffer, const std::shared_ptr<CommandBuffer>& CommandBuffer)
	{
		auto SC = Engine::GetInstance().GetWindow()->GetSwapChain();
		auto Index = Engine::GetInstance().GetWindow()->GetSwapChain()->GetCurrentBufferIndex();

		glm::mat4 TranslateToMid = glm::translate(glm::identity<glm::mat4>(), glm::vec3(1920.f / 2.f, 1080.f / 2.f, 0.f));
		glm::mat4 TempProj = glm::ortho(0.f, 1920.f, 0.f, 1080.f) * TranslateToMid;

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = m_FontRenderPass->GetHandle();
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

		Renderer::BindPipeline(CommandBuffer, m_FontPipeline);
		for (const auto& Fonts : FontManager::GetInstance()->m_LoadedFonts)
		{
			ResourceManager::Instance().GetResource<Font>(Fonts.second)->GetMaterial()->UpdateForRendering(nullptr, Index);
		}

		for (auto Entity : ECSManager::Instance().GetEntities<TextComponent>())
		{
			auto& TextComp = Entity->GetComponent<TextComponent>();
			if (!TextComp.m_IsVisible)
				continue;

			const auto& FontRes = ResourceManager::Instance().GetResource<Font>(FontManager::m_LoadedFonts[TextComp.m_FontName.m_FontType]);

			std::string TextToRender;
			if (TextComp.m_IsDialogue) //If dialogue we handle what to print here
			{
				TextComp.m_Timer += Engine::GetInstance().GetWindow()->GetDeltaTime();
				int RenderSize = TextComp.m_Timer * TextComp.m_Speed;
				if (RenderSize < TextComp.m_TextContent.Text.size())
					TextToRender = TextComp.m_TextContent.Text.substr(0, RenderSize);
				else
				{
					TextToRender = TextComp.m_TextContent.Text;
					TextComp.m_IsDialogue = false;
				}
			}
			else //Non dialogue so we render straight
			{
				TextToRender = TextComp.m_TextContent.Text;
			}

			if (TextComp.m_IsFading)
			{
				if (TextComp.m_Color.w < 1)
				{
					TextComp.m_Color.w += ((Engine::GetInstance().GetWindow()->GetDeltaTime() * TextComp.m_FadingSpeed) / 255.f);
				}
				else
				{
					TextComp.m_Color.w = 1.f;
					TextComp.m_IsFading = false;
				}
			}

			float offset = 0.f;
			float y_offset = 0.f;
			for (int x = 0; x < TextToRender.size(); x++)
			{
				char Letter = TextToRender[x];
				
				float textwidth = (FontRes->m_Characters[Letter].Advance >> 6) / s_DefaultFontSize + FontRes->m_Characters[Letter].Bearing.x / s_DefaultFontSize;
				if (Letter == '\n')
				{
					y_offset += 2 * (FontRes->m_Characters[Letter].HeightAdvance >> 6) / s_DefaultFontSize;
					offset = 0;
					continue;
				}
				glm::vec2 fontscale = glm::vec2(FontRes->m_Characters[Letter].Size.x / s_DefaultFontSize, FontRes->m_Characters[Letter].Size.y / s_DefaultFontSize);
				offset += textwidth;

				Font_PushConstant pc{};
				auto TransformComp = Entity->GetComponent<Transform>();
			
				pc.Proj = TempProj
					* TransformComp.m_WorldXform
					* glm::translate(glm::mat4(1.f), glm::vec3(offset, (FontRes->m_Characters[Letter].Size.y / s_DefaultFontSize - 2 * (FontRes->m_Characters[Letter].Bearing.y / s_DefaultFontSize)) + y_offset, 0.f))
					* glm::scale(glm::mat4(1.f), glm::vec3(fontscale.x, fontscale.y, 1.f));
				pc.Color = TextComp.m_Color;

				vkCmdPushConstants(CommandBuffer->GetInUseCommandBuffer(), m_FontPipeline->GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(Font_PushConstant), &pc);
				vkCmdBindDescriptorSets(CommandBuffer->GetInUseCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_FontPipeline->GetPipelineLayout(), 0, 1, &FontRes->m_Material->GetDescriptor(Index), 0, NULL);

				VkDeviceSize offsets[] = { 0 };
				auto VB = FontRes->m_VertexData[Letter]->GetBuffer();
				vkCmdBindVertexBuffers(CommandBuffer->GetInUseCommandBuffer(), 0, 1, &VB, offsets);
				vkCmdBindIndexBuffer(CommandBuffer->GetInUseCommandBuffer(), m_FontIndexBuffer->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);

				vkCmdDrawIndexed(CommandBuffer->GetInUseCommandBuffer(), m_FontIndexBuffer->GetIndexCount(), 1, 0, 0, 0);

				offset += textwidth;
			}
		}

		Renderer::EndRenderPass(CommandBuffer);
	}
}