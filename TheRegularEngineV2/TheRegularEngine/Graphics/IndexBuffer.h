#pragma once

namespace TRE
{
	class IndexBuffer
	{
		public:
			IndexBuffer(void* Indices, uint32_t Size = 0, uint32_t IndicesCount = 0);
			~IndexBuffer();

			VkBuffer GetBuffer() const;
			uint32_t GetIndexCount() const; 

		private:
			VkBuffer m_Buffer;
			VkDeviceMemory m_BufferMemory;
			uint32_t m_BufferSize;
			uint32_t m_IndexCount;
	};
}