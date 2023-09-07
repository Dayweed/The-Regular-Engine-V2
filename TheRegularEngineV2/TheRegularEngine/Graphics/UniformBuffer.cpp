#include "pch.h"
#include "UniformBuffer.h"
#include "Renderer.h"
#include "RendererContext.h"

namespace TRE
{
	//std::shared_ptr<Buffer> UniformBuffer::GetBuffer()
	//{
	//	return m_Buffer;
	//}

	//UniformBuffer::UniformBuffer(uint32_t Size, uint32_t Binding)
	//{
	//	m_Buffer = std::make_shared<Buffer>(sizeof(UBO), 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
	//}

	//UniformBuffer::~UniformBuffer()
	//{

	//}
	//
	//void UniformBuffer::Map(VkDeviceSize size, VkDeviceSize offset)
	//{
	//	m_Buffer->Map();
	//}

	std::shared_ptr<Buffer> UniformBuffer::GetBuffer()
	{
		return m_Buffer;
	}

	UniformBuffer::UniformBuffer()
	{
		m_Buffer = std::make_shared<Buffer>(sizeof(UBO), 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
	}

	UniformBuffer::~UniformBuffer()
	{

	}

	void UniformBuffer::Map(VkDeviceSize size, VkDeviceSize offset)
	{
		m_Buffer->Map();
	}
}