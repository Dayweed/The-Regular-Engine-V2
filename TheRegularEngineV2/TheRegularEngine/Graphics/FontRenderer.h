#pragma once
#include "freetype/freetype.h"
#include "Pipeline.h"
#include "CommandBuffer.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"

namespace TRE
{
	class Device;

	struct Character
	{
		glm::ivec2	 Size;
		glm::ivec2	 Bearing;
		uint32_t	 Offset;
		uint32_t	 Advance;
	};

	struct FontVertex 
	{
		glm::vec3 Pos;
		glm::vec4 Color;
		glm::vec2 UV;
	};

	struct Font_PushConstant
	{
		glm::mat4 Proj;
	};

	class FontRenderer
	{
		public:
			FontRenderer(const std::shared_ptr<Device>& Device);
			~FontRenderer();

			void CreateNewFontFace(std::string Filepath, std::string FontType);
			std::string GetFontType(std::string Filepath);

			void RenderFont(VkFramebuffer TargetFramebuffer, const std::shared_ptr<CommandBuffer>& CommandBuffer);

		private:
			std::shared_ptr<Device> m_Device;

		private:
			FT_Library m_FTLibrary;
			std::unordered_map<std::string, FT_Face> m_Faces{};
			std::vector<std::string> m_AvailableFonts;
			std::unordered_map<char, Character> m_Characters{};

			std::string m_DefaultFontFilepath = "../Resources/Fonts/Inter-Black.ttf";

			float invBmpWidth;
			uint32_t bmpHeight;

			std::shared_ptr<RenderPass> m_FontRenderPass;
			std::shared_ptr<Pipeline> m_FontPipeline;
	};
}