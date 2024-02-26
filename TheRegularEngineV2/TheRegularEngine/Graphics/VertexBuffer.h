#pragma once

namespace TRE
{
	struct QuadVertex
	{
		glm::vec3 Position;
		glm::vec2 TexCoord;
	};

	class VertexBuffer
	{
		public:
			VertexBuffer(void* Vertices, uint32_t Size);
			~VertexBuffer();

			VkBuffer GetBuffer() const;
			uint32_t GetSize() const;

		private:
			VkBuffer m_Buffer;
			VkDeviceMemory m_BufferMemory;
			uint32_t m_BufferSize;
	};
}