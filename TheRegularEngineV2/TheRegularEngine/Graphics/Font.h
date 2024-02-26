#pragma once
#include "pch.h"
#include "Resource/Resource.h"
#include "VulkanTexture.h"
#include "Material.h"
#include "VertexBuffer.h"

namespace TRE
{
	struct FontVertex
	{
		glm::vec3 Pos;
		glm::vec2 UV;
	};

	struct Character
	{
		glm::vec2	 Size;
		glm::vec2	 Bearing;
		glm::vec2	 UV[4];
		unsigned int Advance;
		unsigned int HeightAdvance;
	};

	class Font : public Resource
	{
		public:
			Font(const std::string& resourcepath);
			std::shared_ptr<Material> GetMaterial()
			{
				return m_Material;
			}

			void CreateFontData();
			static ResourceType GetType() { return ResourceType::Font; }
			//~Font();

		//private:
			std::string m_FontType;
			std::unordered_map<char, Character> m_Characters;
			std::shared_ptr<VulkanTexture> m_TextureAtlas;
			std::shared_ptr<Material> m_Material;
			std::map<char, std::shared_ptr<VertexBuffer>> m_VertexData;

			uint32_t m_Width;
			uint32_t m_Height;
	};
}