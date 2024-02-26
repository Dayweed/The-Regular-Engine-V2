#include "pch.h"
#include "Font.h"
#include "Core/Logger.h"
#include "Resource/ResourceManager.h"

namespace TRE
{
	void Font::CreateFontData() //To be remove after batching
	{
		float x = -1.f; float y = -1.f;
		float width = 2, height = 2;
		std::map<char, std::vector<FontVertex>> CharactersContainer;
		std::vector<FontVertex> NewVertex(4);
		for (unsigned char c = 0; c < 128; c++)
		{
			NewVertex[0].Pos = glm::vec3(x, y, 0.0f);
			NewVertex[0].UV = m_Characters[c].UV[0];

			NewVertex[1].Pos = glm::vec3(x + width, y, 0.0f);
			NewVertex[1].UV = m_Characters[c].UV[1];

			NewVertex[2].Pos = glm::vec3(x + width, y + height, 0.0f);
			NewVertex[2].UV = m_Characters[c].UV[2];

			NewVertex[3].Pos = glm::vec3(x, y + height, 0.0f);
			NewVertex[3].UV = m_Characters[c].UV[3];

			m_VertexData[c] = std::make_shared<VertexBuffer>(static_cast<void*>(NewVertex.data()), NewVertex.size() * sizeof(FontVertex));
		}
	}

	Font::Font(const std::string& resourcepath)
	{
		m_Type = ResourceType::Font;
		std::filesystem::path path = resourcepath;
		if (std::filesystem::exists(resourcepath))
		{
			std::ifstream file(resourcepath, std::ios::binary);
			if (file.is_open())
			{				
				file.seekg(0, std::ios::end);
				std::size_t size = file.tellg();
				file.seekg(0, std::ios::beg);

				char* buffer = new char[size];
				file.read(buffer, size);
				file.close();

				std::size_t offset = 0;

				//Width
				m_Width = *reinterpret_cast<std::uint32_t*>(buffer + offset);
				offset += sizeof(std::uint32_t);

				//Height
				m_Height = *reinterpret_cast<std::uint32_t*>(buffer + offset);
				offset += sizeof(std::uint32_t);

				//Data Size
				uint32_t TotalDataSize = *reinterpret_cast<int*>(buffer + offset);
				offset += sizeof(std::uint32_t);

				//Texture Data
				uint8_t* TextureData = new uint8_t[TotalDataSize];
				memcpy(TextureData, buffer + offset, TotalDataSize);
				offset += TotalDataSize;
				
				//Characters Info
				for (unsigned char c = 0; c < 128; c++)
				{
					m_Characters[c].Size = *reinterpret_cast<glm::vec2*>(buffer + offset);
					offset += sizeof(glm::vec2);

					m_Characters[c].Bearing = *reinterpret_cast<glm::vec2*>(buffer + offset);
					offset += sizeof(glm::vec2);

					m_Characters[c].UV[0] = *reinterpret_cast<glm::vec2*>(buffer + offset);
					offset += sizeof(glm::vec2);

					m_Characters[c].UV[1] = *reinterpret_cast<glm::vec2*>(buffer + offset);
					offset += sizeof(glm::vec2);

					m_Characters[c].UV[2] = *reinterpret_cast<glm::vec2*>(buffer + offset);
					offset += sizeof(glm::vec2);

					m_Characters[c].UV[3] = *reinterpret_cast<glm::vec2*>(buffer + offset);
					offset += sizeof(glm::vec2);

					m_Characters[c].Advance = *reinterpret_cast<unsigned int*>(buffer + offset);
					offset += sizeof(unsigned int);

					m_Characters[c].HeightAdvance = *reinterpret_cast<unsigned int*>(buffer + offset);
					offset += sizeof(unsigned int);
				}

				m_TextureAtlas = std::make_shared<VulkanTexture>(TextureData, TotalDataSize, m_Width, m_Height);
				m_Material = std::make_shared<Material>(ResourceManager::Instance().GetResource<Shader>(11));
				m_Material->Invalidate();
				m_Material->SetTexture("FontTexture", m_TextureAtlas);

				CreateFontData();

				delete[] buffer;
			}
			else
			{
				TRE_CORE_ERROR("File could not be opened: {0}", resourcepath);
			}
		}
		else
		{
			TRE_CORE_ERROR("File does not exist: {0}", resourcepath);
		}
	}
}