#include "pch.h"
#include "Buffer.h"
#include "RendererContext.h"
#include "Device.h"

namespace TRE
{
	uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) 
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

		throw std::runtime_error("failed to find suitable memory type!");
	}
}

namespace TRE
{
	Buffer::Buffer(VkDeviceSize instanceSize, uint32_t instanceCount, VkBufferUsageFlags usageFlags,
		VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize minOffsetAlignment)
		: m_InstanceSize{ instanceSize }, m_InstanceCount{ instanceCount }, m_UsageFlags{ usageFlags }, m_MemoryPropertyFlags{ memoryPropertyFlags }
	{
		m_AlignmentSize = GetAlignment(instanceSize, minOffsetAlignment);
		m_BufferSize = m_InstanceCount * m_AlignmentSize;
		CreateBuffer();
	}

	Buffer::~Buffer()
	{
		Unmap();
		VkDevice logicalDevice = RendererContext::GetDevice()->GetLogicalDevice();
		vkDestroyBuffer(logicalDevice, m_Buffer, nullptr);
		vkFreeMemory(logicalDevice, m_Memory, nullptr);
	}

	VkResult Buffer::Map(VkDeviceSize size, VkDeviceSize offset)
	{
		assert(m_Buffer && "called map before buffer created");
		return vkMapMemory(RendererContext::GetDevice()->GetLogicalDevice(), m_Memory, offset, size, 0, &m_Mapped);
	}

	void Buffer::Unmap()
	{
		if (m_Mapped)
		{
			vkUnmapMemory(RendererContext::GetDevice()->GetLogicalDevice(), m_Memory);
			m_Mapped = nullptr;
		}
	}

	void Buffer::WriteToBuffer(void* data, VkDeviceSize size, VkDeviceSize offset)
	{
		assert(m_Mapped && "Cannot copy to unmapped buffer");

		if (size == VK_WHOLE_SIZE)
			memcpy(m_Mapped, data, static_cast<size_t>(m_BufferSize));
		else
		{
			char* memOffset = static_cast<char*>(m_Mapped);
			memOffset += offset;
			memcpy(memOffset, data, static_cast<size_t>(size));
		}
	}

	VkResult Buffer::Flush(VkDeviceSize size, VkDeviceSize offset)
	{
		VkMappedMemoryRange mappedRange = {};
		mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		mappedRange.memory = m_Memory;
		mappedRange.offset = offset;
		mappedRange.size = size;
		return vkFlushMappedMemoryRanges(RendererContext::GetDevice()->GetLogicalDevice(), 1, &mappedRange);
	}

	VkResult Buffer::Invalidate(VkDeviceSize size, VkDeviceSize offset)
	{
		VkMappedMemoryRange mappedRange = {};
		mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		mappedRange.memory = m_Memory;
		mappedRange.offset = offset;
		mappedRange.size = size;
		return vkInvalidateMappedMemoryRanges(RendererContext::GetDevice()->GetLogicalDevice(), 1, &mappedRange);
	}

	VkDescriptorBufferInfo Buffer::DescriptorInfo(VkDeviceSize size, VkDeviceSize offset)
	{
		VkDescriptorBufferInfo bufferInfo = {};
		bufferInfo.buffer = m_Buffer;
		bufferInfo.offset = offset;
		bufferInfo.range = size;
		return bufferInfo;
	}

	void Buffer::WriteToIndex(void* data, int index)
	{
		WriteToBuffer(data, m_InstanceSize, index * m_AlignmentSize);
	}

	VkResult Buffer::FlushIndex(int index)
	{
		return Flush(m_AlignmentSize, index * m_AlignmentSize);
	}

	VkDescriptorBufferInfo Buffer::DescriptorInfoIndex(int index)
	{
		return DescriptorInfo(m_AlignmentSize, index * m_AlignmentSize);
	}

	VkResult Buffer::InvalidateIndex(int index)
	{
		return Invalidate(m_AlignmentSize, index * m_AlignmentSize);
	}

	VkDeviceSize Buffer::GetAlignment(VkDeviceSize instanceSize, VkDeviceSize minOffsetAlignment)
	{
		if (minOffsetAlignment > 0)
			return (instanceSize + minOffsetAlignment - 1) & ~(minOffsetAlignment - 1);
		else
			return instanceSize;
	}

	void Buffer::CreateBuffer()
	{
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = m_BufferSize;
		bufferInfo.usage = m_UsageFlags;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VkDevice logicalDevice = RendererContext::GetDevice()->GetLogicalDevice();

		if (vkCreateBuffer(logicalDevice, &bufferInfo, nullptr, &m_Buffer) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create vertex buffer!");
		}

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(logicalDevice, m_Buffer, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, m_MemoryPropertyFlags);

		if (vkAllocateMemory(logicalDevice, &allocInfo, nullptr, &m_Memory) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to allocate vertex buffer memory!");
		}

		vkBindBufferMemory(logicalDevice, m_Buffer, m_Memory, 0);
	}
}