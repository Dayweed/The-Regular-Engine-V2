#include "pch.h"
#include "Descriptor.h"
#include "RendererContext.h"

namespace TRE
{
	//Descriptor Set Layout Builder
#pragma region Descriptor set layout builder
	DescriptorSetLayout::Builder& DescriptorSetLayout::Builder::AddBinding(uint32_t binding, VkDescriptorType descriptorType,
		VkShaderStageFlags stageFlags, uint32_t count)
	{
		assert(m_Bindings.count(binding) == 0 && "Binding already in use");
		VkDescriptorSetLayoutBinding layoutBinding{};
		layoutBinding.binding = binding;
		layoutBinding.descriptorType = descriptorType;
		layoutBinding.stageFlags = stageFlags;
		layoutBinding.descriptorCount = count;

		m_Bindings[binding] = layoutBinding;
		return *this;
	}

	std::unique_ptr<DescriptorSetLayout> DescriptorSetLayout::Builder::Build() const
	{
		return std::make_unique<DescriptorSetLayout>(m_Bindings);
	}
#pragma endregion Descriptor set layout builder

	//Descriptor Set Layout
#pragma region Descriptor Set Layout 
	DescriptorSetLayout::DescriptorSetLayout(std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings) : m_Bindings(bindings)
	{
		std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings{};
		for (const auto& kv : bindings)
		{
			setLayoutBindings.push_back(kv.second);
		}

		VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo{};
		descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		descriptorSetLayoutInfo.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
		descriptorSetLayoutInfo.pBindings = setLayoutBindings.data();

		if (vkCreateDescriptorSetLayout(RendererContext::GetDevice()->GetLogicalDevice(), &descriptorSetLayoutInfo, nullptr, &m_DescriptorSetLayout) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create descriptor set layout");
		}
	}

	DescriptorSetLayout::~DescriptorSetLayout()
	{
		vkDestroyDescriptorSetLayout(RendererContext::GetDevice()->GetLogicalDevice(), m_DescriptorSetLayout, nullptr);
	}
#pragma endregion Descriptor Set Layout

	//Descriptor Pool Builder
#pragma region Descriptor Pool Builder
	DescriptorPool::Builder& DescriptorPool::Builder::AddPoolSize(VkDescriptorType descriptorType, uint32_t count)
	{
		m_PoolSizes.push_back({ descriptorType, count });
		return *this;
	}

	DescriptorPool::Builder& DescriptorPool::Builder::SetPoolFlags(VkDescriptorPoolCreateFlags flags)
	{
		m_PoolFlags = flags;
		return *this;
	}

	DescriptorPool::Builder& DescriptorPool::Builder::SetMaxSets(uint32_t count)
	{
		m_MaxSets = count;
		return *this;
	}

	std::unique_ptr<DescriptorPool> DescriptorPool::Builder::Build() const
	{
		return std::make_unique<DescriptorPool>(m_MaxSets, m_PoolFlags, m_PoolSizes);
	}
#pragma endregion Descriptor Pool Builder

	//Descriptor Pool
#pragma region Descriptor Pool
	DescriptorPool::DescriptorPool(uint32_t maxSets, VkDescriptorPoolCreateFlags poolFlags,
		const std::vector <VkDescriptorPoolSize>& poolSizes)
	{
		VkDescriptorPoolCreateInfo descriptorPoolInfo{};
		descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		descriptorPoolInfo.pPoolSizes = poolSizes.data();
		descriptorPoolInfo.maxSets = maxSets;
		descriptorPoolInfo.flags = poolFlags;

		if (vkCreateDescriptorPool(RendererContext::GetDevice()->GetLogicalDevice(), &descriptorPoolInfo, nullptr, &m_DescriptorPool) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create descriptor pool");
		}
	}

	DescriptorPool::~DescriptorPool()
	{
		vkDestroyDescriptorPool(RendererContext::GetDevice()->GetLogicalDevice(), m_DescriptorPool, nullptr);
	}

	bool DescriptorPool::AllocateDescriptorSet(const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptor) const
	{
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = m_DescriptorPool;
		allocInfo.pSetLayouts = &descriptorSetLayout;
		allocInfo.descriptorSetCount = 1;

		//if pool full
		if (vkAllocateDescriptorSets(RendererContext::GetDevice()->GetLogicalDevice(), &allocInfo, &descriptor) != VK_SUCCESS)
		{
			return false;
		}

		return true;
	}

	void DescriptorPool::FreeDescriptorSet(std::vector<VkDescriptorSet>& descriptors) const
	{
		vkFreeDescriptorSets(RendererContext::GetDevice()->GetLogicalDevice(), m_DescriptorPool, static_cast<uint32_t>(descriptors.size()), descriptors.data());
	}

	void DescriptorPool::ResetPool()
	{
		vkResetDescriptorPool(RendererContext::GetDevice()->GetLogicalDevice(), m_DescriptorPool, 0);
	}
#pragma endregion Descriptor Pool

	//Descriptor Writer
#pragma region Descriptor Writer
	DescriptorWriter::DescriptorWriter(DescriptorSetLayout& setLayout, DescriptorPool& pool) : m_SetLayout(setLayout), m_Pool(pool) {}

	DescriptorWriter& DescriptorWriter::WriteBuffer(uint32_t binding, VkDescriptorBufferInfo* bufferInfo)
	{
		assert(m_SetLayout.m_Bindings.count(binding) == 1 && "Layout does not contain specified binding");

		auto& bindingDescription = m_SetLayout.m_Bindings[binding];

		assert(bindingDescription.descriptorCount == 1 && "Binding single descriptor info but binding expects multiple");

		VkWriteDescriptorSet write{};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.descriptorType = bindingDescription.descriptorType;
		write.dstBinding = binding;
		write.pBufferInfo = bufferInfo;
		write.descriptorCount = 1;

		m_Writes.push_back(write);
		return *this;
	}

	DescriptorWriter& DescriptorWriter::WriteImage(uint32_t binding, VkDescriptorImageInfo* imageInfo)
	{
		assert(m_SetLayout.m_Bindings.count(binding) == 1 && "Layout does not contain specified binding");

		auto& bindingDescription = m_SetLayout.m_Bindings[binding];

		assert(bindingDescription.descriptorCount == 1 && "Binding single descriptor info but binding expects multiple");

		VkWriteDescriptorSet write{};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.descriptorType = bindingDescription.descriptorType;
		write.dstBinding = binding;
		write.pImageInfo = imageInfo;
		write.descriptorCount = 1;

		m_Writes.push_back(write);
		return *this;
	}

	bool DescriptorWriter::Build(VkDescriptorSet& set)
	{
		bool success = m_Pool.AllocateDescriptorSet(m_SetLayout.GetDescriptorSetLayout(), set);
		if (success == false)
		{
			return false;
		}

		Overwrite(set);
		return true;
	}

	void DescriptorWriter::Overwrite(VkDescriptorSet& set)
	{
		for (auto& write : m_Writes)
		{
			write.dstSet = set;
		}
		vkUpdateDescriptorSets(RendererContext::GetDevice()->GetLogicalDevice(), static_cast<uint32_t>(m_Writes.size()), m_Writes.data(), 0, nullptr);
	}
#pragma endregion Descriptor Writer
}