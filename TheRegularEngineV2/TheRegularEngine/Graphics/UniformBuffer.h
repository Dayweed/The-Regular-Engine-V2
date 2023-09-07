#pragma once
#include "Buffer.h"

namespace TRE
{
	//class UniformBuffer
	//{
	//	public:
	//		UniformBuffer(uint32_t Size, uint32_t Binding);
	//		~UniformBuffer();

	//		void Map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

	//		std::shared_ptr<Buffer> GetBuffer();

	//	private:
	//		//std::shared_ptr<Buffer> m_Buffer;

	//		VkBuffer m_Buffer;
	//		VkDescriptorBufferInfo m_BufferInfo;
	//		uint32_t m_Size;
	//		uint32_t m_Binding;
	//		VkShaderStageFlagBits m_ShaderStage = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
	//		uint8_t* m_Storage;
	//};


	class UniformBuffer
	{
	public:
		UniformBuffer();
		~UniformBuffer();

		void Map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

		std::shared_ptr<Buffer> GetBuffer();

	private:
		std::shared_ptr<Buffer> m_Buffer;
	};
}