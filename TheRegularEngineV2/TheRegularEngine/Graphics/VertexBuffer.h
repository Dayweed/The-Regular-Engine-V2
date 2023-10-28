#pragma once

namespace TRE
{
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