#include "pch.h"
#include "UniformBuffer.h"
#include "RendererContext.h"
#include "Core/Logger.h"

namespace TRE
{
	static uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
	{
		VkPhysicalDeviceMemoryProperties memProperties;
		auto physicalDevice = RendererContext::GetDevice()->GetPhysicalDevice()->GetPhysicalDevice();
		vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);
		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
		{
			if ((typeFilter & (1 << i)) &&
				(memProperties.memoryTypes[i].propertyFlags & properties) == properties)
			{
				return i;
			}
		}

		TRE_CORE_ERROR("Unable to find memory type");
		assert(false);
		return 0;
	}

	uint32_t UniformBuffer::GetBinding() const
	{
		return m_Binding;
	}

	const VkDescriptorBufferInfo& UniformBuffer::GetDescriptorBufferInfo() const
	{
		return m_BufferInfo;
	}

	UniformBuffer::UniformBuffer(uint32_t Size, uint32_t Binding) : m_Size(Size), m_Binding(Binding)
	{
		m_Storage = new uint8_t[Size];
		Invalidate();
	}

	UniformBuffer::~UniformBuffer()
	{
		Release();
	}

	void UniformBuffer::SetData(const void* data, uint32_t size, uint32_t offset)
	{
		memcpy(m_Storage, data, size);
		VkDevice Device = RendererContext::GetDevice()->GetLogicalDevice();
		uint8_t* pData;
		vkMapMemory(Device, m_BufferMemory, offset, size, 0, (void**)&pData);
		memcpy(pData, (const uint8_t*)m_Storage + offset, size);
		vkUnmapMemory(Device, m_BufferMemory);
	}

	void UniformBuffer::Release()
	{
		if (m_Buffer == VK_NULL_HANDLE)
			return;

		VkDevice Device = RendererContext::GetDevice()->GetLogicalDevice();
		vkFreeMemory(Device, m_BufferMemory, nullptr);
		vkDestroyBuffer(Device, m_Buffer, nullptr);
		delete[] reinterpret_cast<uint8_t*>(m_Storage);
		m_Buffer = nullptr;
		m_Storage = nullptr;
	}

	void UniformBuffer::Invalidate()
	{
		Release();

		VkDevice Device = RendererContext::GetDevice()->GetLogicalDevice();

		VkBufferCreateInfo BufferCreateInfo{};
		BufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		BufferCreateInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		BufferCreateInfo.size = m_Size;
		BufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (auto Result = vkCreateBuffer(Device, &BufferCreateInfo, nullptr, &m_Buffer); Result != VK_SUCCESS)
		{
			TRE_CORE_ERROR("Error creating uniform buffer");
			assert(Result == VK_SUCCESS);
		}

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(Device, m_Buffer, &memRequirements);

		VkMemoryAllocateInfo MemoryAllocateInfo{};
		MemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		MemoryAllocateInfo.allocationSize = memRequirements.size;
		MemoryAllocateInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

		if (auto Result = vkAllocateMemory(Device, &MemoryAllocateInfo, nullptr, &m_BufferMemory); Result != VK_SUCCESS)
		{
			TRE_CORE_ERROR("Failed to allocate memory");
			assert(Result == VK_SUCCESS);
		}

		m_BufferInfo.buffer = m_Buffer;
		m_BufferInfo.offset = 0;
		m_BufferInfo.range = m_Size;

		vkBindBufferMemory(Device, m_Buffer, m_BufferMemory, 0);
	}
}