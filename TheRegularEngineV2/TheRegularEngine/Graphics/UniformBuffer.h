#pragma once
#include "Buffer.h"

namespace TRE
{
	class UniformBuffer
	{
		public:
			UniformBuffer(uint32_t Size, uint32_t Binding);
			~UniformBuffer();
			void SetData(const void* data, uint32_t size, uint32_t offset = 0);


		public:
			uint32_t GetBinding() const;
			const VkDescriptorBufferInfo& GetDescriptorBufferInfo() const;

		private:
			void Release();
			void Invalidate();

		private:
			VkBuffer m_Buffer = VK_NULL_HANDLE;
			VkDeviceMemory m_BufferMemory;
			VkDescriptorBufferInfo m_BufferInfo{};
			uint32_t m_Size = 0;
			uint32_t m_Binding = 0;
			VkShaderStageFlagBits m_ShaderStage = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
			uint8_t* m_Storage = nullptr;
			std::string DebugName;
	};
}