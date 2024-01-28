#include "pch.h"
#include "Slideshow.h"
#include "SwapChain.h"
#include "Core/Engine.h"
#include "RendererContext.h"

namespace TRE
{
	SlideshowComponent::SlideshowComponent()
	{

	}

	void SlideshowComponent::GenerateVertexBuffer()
	{
		m_VertexBuffers.clear();
		for (int i = 0; i < m_Slices; i++)
		{
			float x = -1.f; float y = -1.f;
			float width = 2, height = 2;
			std::vector<QuadVertex> data(4);

			data[0].Position = glm::vec3(x, y, 0.0f);
			data[0].TexCoord = glm::vec2(0.f / m_Slices, 0.f);

			data[1].Position = glm::vec3(x + width, y, 0.0f);
			data[1].TexCoord = glm::vec2(1.f / m_Slices, 0.f);

			data[2].Position = glm::vec3(x + width, y + height, 0.0f);
			data[2].TexCoord = glm::vec2(1.f / m_Slices, 1.f);

			data[3].Position = glm::vec3(x, y + height, 0.0f);
			data[3].TexCoord = glm::vec2(0.f / m_Slices, 1.f);

			auto vertexBuffer = std::make_shared<VertexBuffer>(static_cast<void*>(data.data()),
				UINT32_T_CAST(data.size() * sizeof(QuadVertex)));

			m_VertexBuffers.push_back(vertexBuffer);
		}
	}
}