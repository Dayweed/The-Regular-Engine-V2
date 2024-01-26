#include "pch.h"
#include "FontRenderer.h"
#include "Core/Logger.h"
#include "Device.h"
#include "Core/Engine.h"
#include "TextComponent.h"
#include "Core/ECS.h"
#include "Core/Transform.h"

namespace TRE
{
	std::vector<std::string> FontRenderer::m_AvailableFonts{};

	std::vector<std::string>& FontRenderer::GetLoadedFonts()
	{
		return m_AvailableFonts;
	}

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

		m_FontMaterial = std::make_shared<Material>(ResourceManager::Instance().GetResource<Shader>(11));
		m_FontMaterial->Invalidate();

		//Initialize a default font to render with
		std::string FontType = GetFontType(m_DefaultFontFilepath);
		CreateNewFontFace(m_DefaultFontFilepath, FontType);

		float x = -1.f; float y = -1.f;
		float width = 2, height = 2;
		std::vector<FontVertex> data(4);

		data[0].Pos = glm::vec3(x, y, 0.0f);
		data[0].UV = m_Characters['r'].UV[3];

		data[1].Pos = glm::vec3(x + width, y, 0.0f);
		data[1].UV = m_Characters['r'].UV[2];

		data[2].Pos = glm::vec3(x + width, y + height, 0.0f);
		data[2].UV = m_Characters['r'].UV[1];

		data[3].Pos = glm::vec3(x, y + height, 0.0f);
		data[3].UV = m_Characters['r'].UV[0];

		std::vector<int> indices = { 0,1,2,2,3,0 };

		m_FontIndexBuffer = std::make_shared<IndexBuffer>(static_cast<void*>(indices.data()),
			UINT32_T_CAST(sizeof(int) * indices.size()),
			UINT32_T_CAST(indices.size()));

		m_FontVertexBuffer = std::make_shared<VertexBuffer>(static_cast<void*>(data.data()),
			UINT32_T_CAST(data.size() * sizeof(FontVertex)));
	}

	FontRenderer::~FontRenderer()
	{

	}

	void FontRenderer::CreateNewFontFace(std::string Filepath, std::string FontType)
	{
		FT_Library m_FTLibrary;
		if (FT_Error Error = FT_Init_FreeType(&m_FTLibrary); !Error)
		{
			TRE_CORE_INFO("Free Type Init");
		}

		FT_Face face;
		if (FT_New_Face(m_FTLibrary, Filepath.c_str(), 0, &face))
		{
			TRE_CORE_ERROR("Unable to load font: {0}", FontType);
		}

		FontRenderer::GetLoadedFonts().push_back(FontType);

		FT_Set_Pixel_Sizes(face, 0, 48); //Scale the font size using transform comp

		int width = 0;
		int height = 0;
		for (unsigned char c = 0; c < 128; c++)
		{
			if (FT_Load_Char(face, c, FT_LOAD_RENDER))
			{
				assert(false && "Unable to load Char");
			}

			width += face->glyph->bitmap.width;
			
			if (static_cast<unsigned int>(height) < face->glyph->bitmap.rows)
				height = face->glyph->bitmap.rows;
		}

		uint8_t* data = new uint8_t[width * height * 4];

		std::vector<uint8_t> Buffer(48);
		int x = 0;
		int offset = 0;
		for (unsigned char c = 0; c < 128; c++)
		{
			if (FT_Load_Char(face, c, FT_LOAD_RENDER))
			{
				assert(false && "Unable to load Char");
			}

			int TempOffset = offset;
			for (unsigned int x = 0; x < face->glyph->bitmap.rows; x++)
			{
				std::copy(face->glyph->bitmap.buffer + x * face->glyph->bitmap.width, face->glyph->bitmap.buffer + (x + 1) * face->glyph->bitmap.width, Buffer.data());

				for (unsigned int y = 0; y < face->glyph->bitmap.width; y++)
				{
					const uint8_t& letter = Buffer[y];
					int Index = TempOffset + y * 4;
					for (int z = 0; z < 4; z++)
						data[Index + z] = letter;
				}
				TempOffset += width * 4;
			}
			offset += face->glyph->bitmap.width * 4;

			float xpos = static_cast<float>(x) / width ;
			float w = static_cast<float>(face->glyph->bitmap.width) / static_cast<float>(width);
			float ypos = static_cast<float>(face->glyph->bitmap.rows) / static_cast<float>(height);
			
			Character character = 
			{
				glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
				glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
				{{xpos, ypos}, { xpos + w,  ypos }, { xpos + w,  0.f }, { xpos,  0.f }},
				face->glyph->advance.x
			};

			m_Characters.insert(std::pair<char, Character>(c, character));

			x += static_cast<int>(face->glyph->bitmap.width);
		}

		m_FontTexture = std::make_shared<VulkanTexture>(data, width * height * 4, width, height);
		m_FontMaterial->SetTexture("FontTexture", m_FontTexture);

		delete[] data;
		FT_Done_Face(face);
		FT_Done_FreeType(m_FTLibrary);
	}

	void FontRenderer::RenderFont(VkFramebuffer TargetFramebuffer, const std::shared_ptr<CommandBuffer>& CommandBuffer)
	{
		auto SC = Engine::GetInstance().GetWindow()->GetSwapChain();

		glm::mat4 TranslateToMid = glm::translate(glm::identity<glm::mat4>(), glm::vec3(static_cast<float>(SC->GetWidth()) / 2.f, static_cast<float>(SC->GetHeight()) / 2.f, 0.f)); //Translate by viewport width or height / 2
		glm::mat4 TempProj = glm::ortho(0.f, static_cast<float>(SC->GetWidth()), 0.f, static_cast<float>(SC->GetHeight())) * TranslateToMid;

		auto Index = Engine::GetInstance().GetWindow()->GetSwapChain()->GetCurrentBufferIndex();

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
		m_FontMaterial->UpdateForRendering(nullptr, Index);
		for (auto Entity : ECSManager::Instance().GetEntities<TextComponent>())
		{
			auto& TextComp = Entity->GetComponent<TextComponent>();
			if (!TextComp.m_IsVisible)
				continue;

			float offset = 0.f;
			for (auto Letter : TextComp.m_TextContent)
			{
				float textwidth = (m_Characters[Letter].Advance >> 6) / 48.f;
				glm::vec2 fontscale = glm::vec2(m_Characters[Letter].Size.x / 48.f, m_Characters[Letter].Size.y / 48.f);
				offset += textwidth;

				Font_PushConstant pc{};
				auto TransformComp = Entity->GetComponent<Transform>();
				pc.Proj = TempProj * TransformComp.m_WorldXform * glm::translate(glm::mat4(1.f), glm::vec3(offset, 0.f, 0.f)) * glm::scale(glm::mat4(1.f), glm::vec3(fontscale.x, fontscale.y, 1.f));
				pc.Color = TextComp.m_Color;

				
				vkCmdPushConstants(CommandBuffer->GetInUseCommandBuffer(), m_FontPipeline->GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(Font_PushConstant), &pc);
				vkCmdBindDescriptorSets(CommandBuffer->GetInUseCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_FontPipeline->GetPipelineLayout(), 0, 1, &m_FontMaterial->GetDescriptor(Index), 0, NULL);

				VkDeviceSize offsets[] = { 0 };
				auto VB = m_FontVertexBuffer->GetBuffer();
				vkCmdBindVertexBuffers(CommandBuffer->GetInUseCommandBuffer(), 0, 1, &VB, offsets);
				vkCmdBindIndexBuffer(CommandBuffer->GetInUseCommandBuffer(), m_FontIndexBuffer->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);

				vkCmdDrawIndexed(CommandBuffer->GetInUseCommandBuffer(), m_FontIndexBuffer->GetIndexCount(), 1, 0, 0, 0);

				offset += textwidth;
			}
		}

		Renderer::EndRenderPass(CommandBuffer);
	}

	std::string FontRenderer::GetFontType(std::string Filepath)
	{
		size_t Start = Filepath.find_last_of('/') + 1;
		size_t End1 = Filepath.find_last_of('.');
		size_t End2 = Filepath.find_last_of('-');
		size_t Final = (End1 < End2) ? End1 : End2;
		std::string FontType = Filepath.substr(Start, Final - Start);
		TRE_CORE_INFO("Font Type Loaded: {0}", FontType);

		return FontType;
	}
}