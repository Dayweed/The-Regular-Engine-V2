#include "pch.h"
#include "Shader.h"
#include "RendererContext.h"
#include "Core/Logger.h"

namespace TRE
{
	std::vector<PushConstants>& Shader::GetPushConstants()
	{
		return m_ReflectionData.PushConstants;
	}

	std::vector<VkDescriptorSetLayoutBinding>& Shader::GetDescriptorBindings()
	{
		return m_DescriptorBindings;
	}

	VkPipelineShaderStageCreateInfo Shader::GetPipelineShaderInfo()
	{
		return m_PipelineShaderCreateInfo;
	}

	const std::unordered_map<std::string, VkWriteDescriptorSet>& Shader::GetWriteDescriptorSets()
	{
		if (m_ReflectionData.DescriptorSets.size())
			return m_ReflectionData.DescriptorSets[0].WriteDescriptorSets;
		else
			return std::unordered_map<std::string, VkWriteDescriptorSet>();
	}

	Shader::Shader(const std::filesystem::path& ShaderPath) : m_ShaderPath(ShaderPath)
	{
		m_Type = AssetType::Shader;
	}

	Shader::~Shader()
	{
		VkDevice Device = RendererContext::GetDevice()->GetLogicalDevice();
		vkDestroyShaderModule(Device, m_PipelineShaderCreateInfo.module, nullptr);
	}

	void Shader::LoadAndCreateShader(const std::vector<uint32_t>& ShaderBinary, VkShaderStageFlagBits ShaderStage)
	{
		m_ShaderBinary = ShaderBinary;

		VkDevice Device = RendererContext::GetDevice()->GetLogicalDevice();

		VkShaderModuleCreateInfo ShaderModCreateInfo{};
		ShaderModCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		ShaderModCreateInfo.codeSize = ShaderBinary.size() * sizeof(uint32_t);
		ShaderModCreateInfo.pCode = ShaderBinary.data();

		VkShaderModule ShaderMod;
		if (auto Result = vkCreateShaderModule(Device, &ShaderModCreateInfo, nullptr, &ShaderMod); Result != VK_SUCCESS)
		{
			TRE_CORE_WARN("Shader Module failed to create");
			assert(Result == VK_SUCCESS);
		}

		m_PipelineShaderCreateInfo = {};
		m_PipelineShaderCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		m_PipelineShaderCreateInfo.stage = ShaderStage;
		m_PipelineShaderCreateInfo.module = ShaderMod;
		m_PipelineShaderCreateInfo.pName = "main";
	}

	void Shader::CreateDescriptors()
	{
		VkDevice Device = RendererContext::GetDevice()->GetLogicalDevice();
		m_Types.clear();

		for (uint32_t x = 0; x < m_ReflectionData.DescriptorSets.size(); x++)
		{
			auto& ShaderDescriptorSet = m_ReflectionData.DescriptorSets[x];
			if (ShaderDescriptorSet.UniformBuffers.size())
			{
				VkDescriptorPoolSize& PoolSize = m_Types[x].emplace_back();
				PoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				PoolSize.descriptorCount = (uint32_t)ShaderDescriptorSet.UniformBuffers.size();
			}

			if (ShaderDescriptorSet.ImageSamplers.size())
			{
				VkDescriptorPoolSize& PoolSize = m_Types[x].emplace_back();
				PoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				PoolSize.descriptorCount = (uint32_t)ShaderDescriptorSet.ImageSamplers.size();
			}

			std::vector<VkDescriptorSetLayoutBinding> LayoutBindings;
			for (auto& [binding, uniformBuffer] : ShaderDescriptorSet.UniformBuffers)
			{
				VkDescriptorSetLayoutBinding& layout = m_DescriptorBindings.emplace_back();
				layout.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				layout.descriptorCount = 1;
				layout.stageFlags = uniformBuffer.ShaderStageFlag;
				layout.pImmutableSamplers = nullptr;
				layout.binding = binding;

				VkWriteDescriptorSet& Set = ShaderDescriptorSet.WriteDescriptorSets[uniformBuffer.Name];
				Set = {};
				Set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET; 
				Set.descriptorCount = 1;
				Set.descriptorType = layout.descriptorType;
				Set.dstBinding = layout.binding;
			}

			for (auto& [binding, ImageSampler] : ShaderDescriptorSet.ImageSamplers)
			{
				VkDescriptorSetLayoutBinding& layout = m_DescriptorBindings.emplace_back();
				layout.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				layout.descriptorCount = ImageSampler.ArraySize;
				layout.stageFlags = ImageSampler.ShaderStage;
				layout.pImmutableSamplers = nullptr;
				layout.binding = binding;

				VkWriteDescriptorSet& Set = ShaderDescriptorSet.WriteDescriptorSets[ImageSampler.Name];
				Set = {};
				Set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				Set.descriptorCount = ImageSampler.ArraySize;
				Set.descriptorType = layout.descriptorType;
				Set.dstBinding = layout.binding;
			}
		}
	}

	void Shader::SetReflectionData(const ShaderReflectionData& ReflectionData)
	{
		m_ReflectionData = ReflectionData;
	}

	void Shader::Serialize()
	{
		
	}
}