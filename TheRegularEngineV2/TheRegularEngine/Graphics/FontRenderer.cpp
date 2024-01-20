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
	FontRenderer::FontRenderer(const std::shared_ptr<Device>& Device) : m_Device(Device)
	{
		if (FT_Error Error = FT_Init_FreeType(&m_FTLibrary); !Error)
		{
			TRE_CORE_INFO("Free Type Init");
		}

		//Initialize a default font to render with
		std::string FontType = GetFontType(m_DefaultFontFilepath);
		CreateNewFontFace(m_DefaultFontFilepath, FontType);

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
		FontPipeConfig.EnableBlending = true;
		FontPipeConfig.Shader = ResourceManager::Instance().GetResource<Shader>(11);
		//FontPipeConfig.CullMode = VK_CULL_MODE_FRONT_BIT;
		m_FontPipeline = std::make_shared<Pipeline>(FontPipeConfig, m_FontRenderPass);
	}

	FontRenderer::~FontRenderer()
	{

	}

	void FontRenderer::CreateNewFontFace(std::string Filepath, std::string FontType)
	{
		FT_Face NewFace;
		if (FT_Error Error = FT_New_Face(m_FTLibrary, Filepath.c_str(), 0, &NewFace); Error)
		{
			assert(false && "Unable to create new FT Face");
		}

		m_Faces[FontType] = NewFace;
		m_AvailableFonts.push_back(FontType);

		FT_Set_Pixel_Sizes(m_Faces[FontType], 0, 48);

		uint32_t bmpWidth = 0;
		std::vector<uint8_t> pixels;
		std::unordered_map<char, std::vector<uint8_t>> data;

		for (unsigned char c = 0; c < 128; c++)
		{
			FT_Error Error = FT_Load_Char(NewFace, c, FT_LOAD_RENDER);
			assert(Error == 0);

			uint32_t bmpHeight = std::max(bmpHeight, NewFace->glyph->bitmap.rows);

			unsigned int pitch = NewFace->glyph->bitmap.pitch;

			Character character =
			{
				glm::ivec2(NewFace->glyph->bitmap.width, NewFace->glyph->bitmap.rows),
				glm::ivec2(NewFace->glyph->bitmap_left, NewFace->glyph->bitmap_top),
				bmpWidth,
				static_cast<unsigned int>(NewFace->glyph->advance.x)
			};

			m_Characters.insert(std::pair<char, Character>(c, character));
			if (NewFace->glyph->bitmap.width > 0)
			{
				void* ptr = NewFace->glyph->bitmap.buffer;

				std::vector<uint8_t> Data(NewFace->glyph->bitmap.width * NewFace->glyph->bitmap.rows);

				int rows = NewFace->glyph->bitmap.rows;
				int width = NewFace->glyph->bitmap.width;
				for (int x = 0; x < rows; x++)
				{
					for (int y = 0; y < width; y++)
					{
						uint8_t byte = NewFace->glyph->bitmap.buffer[x * pitch + y];
						Data[x * pitch + y] = byte;
					}
				}

				data.insert(std::pair<char, std::vector<uint8_t>>(c, Data));
			}
			bmpWidth += NewFace->glyph->bitmap.width;
		}

		FT_Error Error = FT_Done_Face(NewFace);
		assert(Error == 0 && "Error calling Done Face on font");

		invBmpWidth = 1 / (float)bmpWidth;

		uint8_t* buffer = new uint8_t[bmpHeight * bmpWidth];
		memset(buffer, 0, bmpHeight * bmpWidth);

		uint32_t xpos = 0;
		for (unsigned char c = 0; c < 128; c++)
		{
			Character& character = m_Characters[c];

			std::vector<uint8_t>& charData = data[c];
			uint32_t width = character.Size.x;
			uint32_t height = character.Size.y;
			for (uint32_t i = 0; i < height; i++) 
			{
				for (uint32_t j = 0; j < width; j++) 
				{
					uint8_t byte = charData[i * width + j];
					buffer[i * bmpWidth + xpos + j] = byte;
				}
			}
			xpos += width;
		}

		VkDeviceSize pixelsize = 1;
		bool enablelod = false;

		VkDeviceSize ImageSize = (uint64_t)bmpWidth * (uint64_t)bmpHeight * pixelsize;
		
		
	}

	void FontRenderer::RenderFont(VkFramebuffer TargetFramebuffer, const std::shared_ptr<CommandBuffer>& CommandBuffer)
	{
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
		for (auto Entity : ECSManager::Instance().GetEntities<TextComponent>())
		{
			auto& TextComp = Entity->GetComponent<TextComponent>();
			if (!TextComp.m_IsVisible)
				continue;

			Font_PushConstant pc{};
			auto TransformComp = Entity->GetComponent<Transform>();
			pc.Proj = TransformComp.m_WorldXform;

			vkCmdPushConstants(CommandBuffer->GetInUseCommandBuffer(), m_FontPipeline->GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(Font_PushConstant), &pc);

			//UIComp.m_Material->UpdateForRendering(m_UIUBO, Index);
			//vkCmdBindDescriptorSets(CommandBuffer->GetInUseCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_FontPipeline->GetPipelineLayout(), 0, 1, &UIComp.m_Material->GetDescriptor(Index), 0, NULL);

			VkDeviceSize offsets[] = { 0 };
			//auto VB = m_TestVertexBuffer->GetBuffer();
			//vkCmdBindVertexBuffers(CommandBuffer->GetInUseCommandBuffer(), 0, 1, &VB, offsets);
			//vkCmdBindIndexBuffer(CommandBuffer->GetInUseCommandBuffer(), m_TestIndexBuffer->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);

			//vkCmdDrawIndexed(CommandBuffer->GetInUseCommandBuffer(), m_TestIndexBuffer->GetIndexCount(), 1, 0, 0, 0);
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