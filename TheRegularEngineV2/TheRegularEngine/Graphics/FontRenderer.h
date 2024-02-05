#pragma once
#include "Pipeline.h"
#include "CommandBuffer.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "Pipeline.h"
#include "RenderPass.h"
#include "VulkanTexture.h"
#include "Material.h"
#include "RendererContext.h"

namespace TRE
{
	class Device;

	struct Character
	{
		glm::vec2	 Size;
		glm::vec2	 Bearing;
		glm::vec2	 UV[4];
		unsigned int Advance;
		unsigned int HeightAdvance; //To be remove, just making it easy now
	};

	struct FontVertex 
	{
		glm::vec3 Pos;
		glm::vec2 UV;
	};

	struct Font_PushConstant
	{
		glm::mat4 Proj;
		glm::vec4 Color;
	};

	class FontRenderer
	{
		public:
			FontRenderer(const std::shared_ptr<Device>& Device);
			~FontRenderer();

			static void CreateNewFontFace(std::string Filepath, std::string FontType);
			static std::string GetFontType(std::string Filepath);

			void RenderFont(VkFramebuffer TargetFramebuffer, const std::shared_ptr<CommandBuffer>& CommandBuffer);


			static void LoadFont(std::string FilePath);
			static std::vector<std::string>& GetLoadedFonts();

		private:
			static void CreateFontData(std::string FontType);

		private:
			std::shared_ptr<Device> m_Device;

		private:
			static std::vector<std::string> m_AvailableFonts;
			static std::unordered_map<std::string, std::unordered_map<char, Character>> m_Characters; //Per Font Type

			std::string m_DefaultFontFilepath = "../Assets/Font/arial.ttf";

			std::shared_ptr<RenderPass> m_FontRenderPass;
			std::shared_ptr<Pipeline> m_FontPipeline;

			static std::unordered_map<std::string, std::map<char, std::shared_ptr<VertexBuffer>>> m_VertexData; //Key = Font Type //Each FontType has a map characters, each characters have a vector of quad
			std::shared_ptr<IndexBuffer> m_FontIndexBuffer;

		private:
			static std::unordered_map<std::string, std::shared_ptr<VulkanTexture>> m_FontTexture;
			static std::unordered_map<std::string, std::shared_ptr<Material>> m_FontMaterial;
	};
}