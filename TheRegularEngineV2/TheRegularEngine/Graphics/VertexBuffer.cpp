#include "pch.h"
#include "VertexBuffer.h"
#include "RendererContext.h"
#include "VulkanUtilities.h"

namespace TRE
{
	VkBuffer VertexBuffer::GetBuffer() const
	{
		return m_Buffer;
	}

	uint32_t VertexBuffer::GetSize() const
	{
		return m_BufferSize;
	}

	VertexBuffer::VertexBuffer(void* Vertices, uint32_t Size) : m_BufferSize(Size)
	{
		auto Device = RendererContext::GetDevice()->GetLogicalDevice();

		VkBufferCreateInfo BufferCreateInfo{};
		BufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		BufferCreateInfo.size = m_BufferSize;
		BufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		BufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		
		VkBuffer StagingBuffer;
		VkDeviceMemory StageBufferMemory;
		if (auto Result = vkCreateBuffer(Device, &BufferCreateInfo, nullptr, &StagingBuffer); Result != VK_SUCCESS)
		{
			assert(Result == VK_SUCCESS && "Unable to allocate staging bufffer for vertex buffer");
		}

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(Device, StagingBuffer, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = vkUtils::BufferFindMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		if (auto Result = vkAllocateMemory(Device, &allocInfo, nullptr, &StageBufferMemory); Result != VK_SUCCESS)
		{
			assert(Result == VK_SUCCESS && "Unable to allocate memory for staging buffer");
		}

		uint8_t* StagingData;
		vkBindBufferMemory(Device, StagingBuffer, StageBufferMemory, 0);
		vkMapMemory(Device, StageBufferMemory, 0, VK_WHOLE_SIZE, 0, (void**)&StagingData);
		memcpy(StagingData, Vertices, static_cast<size_t>(m_BufferSize));
		vkUnmapMemory(Device, StageBufferMemory);
		StagingData = nullptr;

		VkBufferCreateInfo VertexBufferCreateInfo{};
		VertexBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		VertexBufferCreateInfo.size = m_BufferSize;
		VertexBufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;;
		VertexBufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		
		if (auto Result = vkCreateBuffer(Device, &VertexBufferCreateInfo, nullptr, &m_Buffer); Result != VK_SUCCESS)
		{
			assert(Result == VK_SUCCESS && "Unable to bufffer for vertex buffer");
		}

		VkMemoryRequirements VertexBufferMemRequirements;
		vkGetBufferMemoryRequirements(Device, m_Buffer, &VertexBufferMemRequirements);

		VkMemoryAllocateInfo VertexBufferAllocInfo{};
		VertexBufferAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		VertexBufferAllocInfo.allocationSize = VertexBufferMemRequirements.size;
		VertexBufferAllocInfo.memoryTypeIndex = vkUtils::BufferFindMemoryType(VertexBufferMemRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		if (auto Result = vkAllocateMemory(Device, &VertexBufferAllocInfo, nullptr, &m_BufferMemory); Result != VK_SUCCESS)
		{
			assert(Result == VK_SUCCESS && "Unable to allocate memory for vertex buffer");
		}

		vkBindBufferMemory(Device, m_Buffer, m_BufferMemory, 0);

		vkUtils::CopyBuffer(StagingBuffer, m_Buffer, m_BufferSize);

		vkDestroyBuffer(Device, StagingBuffer, nullptr);
		vkFreeMemory(Device, StageBufferMemory, nullptr);
	}

	VertexBuffer::~VertexBuffer()
	{
		auto Device = RendererContext::GetDevice()->GetLogicalDevice();

		vkDestroyBuffer(Device, m_Buffer, nullptr);
		vkFreeMemory(Device, m_BufferMemory, nullptr);
	}
}