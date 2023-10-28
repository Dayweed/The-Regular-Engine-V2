#include "pch.h"
#include "Shader.h"
#include "RendererContext.h"
#include "Core/Logger.h"
#include "ShaderTypes/PBRShader.h"
#include "ShaderTypes/LineShader.h"

namespace TRE
{
	std::vector<VkDescriptorSetLayout> Shader::GetAllDescriptorLayout()
	{
		std::vector<VkDescriptorSetLayout> result;
		result.reserve(m_DescriptorSetLayouts.size());
		for (auto& layout : m_DescriptorSetLayouts)
			result.emplace_back(layout);

		return result;
	}

	std::map<std::string, VkWriteDescriptorSet> Shader::GetWriteDescriptors()
	{
		return m_ReflectionData.DescriptorSets[0].WriteDescriptorSets;
	}

	uint32_t Shader::GetVertexStrides()
	{
		return m_ReflectionData.VertexStride;
	}

	const std::vector<VkVertexInputAttributeDescription>& Shader::GetVertexAttributes()
	{
		return m_ReflectionData.VertexInputAttributeDescriptions;
	}

	std::vector<PushConstants>& Shader::GetPushConstants()
	{
		return m_ReflectionData.PushConstants;
	}

	const std::vector<VkPipelineShaderStageCreateInfo>& Shader::GetPipelineShaderInfo()
	{
		return m_PipelineShaderCreateInfos;
	}

	Shader::Shader(const std::filesystem::path& ShaderPath) : m_ShaderPath(ShaderPath)
	{
		m_Type = ResourceType::Shader;
	}

	Shader::~Shader()
	{
		VkDevice Device = RendererContext::GetDevice()->GetLogicalDevice();
		for (auto& ShaderInfo : m_PipelineShaderCreateInfos)
			vkDestroyShaderModule(Device, ShaderInfo.module, nullptr);

		for (auto& DescriptorLayout : m_DescriptorSetLayouts)
		{
			vkDestroyDescriptorSetLayout(Device, DescriptorLayout, nullptr);
		}
	}

	void Shader::LoadAndCreateShader(const std::map<VkShaderStageFlagBits, std::vector<uint32_t>>& ShaderBinary)
	{
		m_ShaderBinary = ShaderBinary;

		VkDevice Device = RendererContext::GetDevice()->GetLogicalDevice();
		m_PipelineShaderCreateInfos.clear();

		for (auto [Stage, Binary] : m_ShaderBinary)
		{
			VkShaderModuleCreateInfo ShaderModCreateInfo{};
			ShaderModCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			ShaderModCreateInfo.codeSize = Binary.size() * sizeof(uint32_t);
			ShaderModCreateInfo.pCode = Binary.data();

			VkShaderModule ShaderMod;
			if (auto Result = vkCreateShaderModule(Device, &ShaderModCreateInfo, nullptr, &ShaderMod); Result != VK_SUCCESS)
			{
				TRE_CORE_WARN("Shader Module failed to create");
				assert(Result == VK_SUCCESS);
			}

			VkPipelineShaderStageCreateInfo& ShaderStageCreateInfo = m_PipelineShaderCreateInfos.emplace_back();
			ShaderStageCreateInfo = {};
			ShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			ShaderStageCreateInfo.stage = Stage;
			ShaderStageCreateInfo.module = ShaderMod;
			ShaderStageCreateInfo.pName = "main";
		}
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
				VkDescriptorSetLayoutBinding& layout = LayoutBindings.emplace_back();
				layout.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				layout.descriptorCount = 1;
				layout.stageFlags = uniformBuffer.ShaderStageFlag;
				layout.pImmutableSamplers = nullptr;
				layout.binding = binding;

				VkWriteDescriptorSet& WriteDescriptorSet = ShaderDescriptorSet.WriteDescriptorSets[uniformBuffer.Name];
				WriteDescriptorSet = {};
				WriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				WriteDescriptorSet.descriptorCount = 1;
				WriteDescriptorSet.descriptorType = layout.descriptorType;
				WriteDescriptorSet.dstBinding = layout.binding;
			}

			for (auto& [binding, ImageSampler] : ShaderDescriptorSet.ImageSamplers)
			{
				VkDescriptorSetLayoutBinding& layout = LayoutBindings.emplace_back();
				layout.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				layout.descriptorCount = ImageSampler.ArraySize;
				layout.stageFlags = ImageSampler.ShaderStage;
				layout.pImmutableSamplers = nullptr;
				layout.binding = binding;

				VkWriteDescriptorSet& WriteDescriptorSet = ShaderDescriptorSet.WriteDescriptorSets[ImageSampler.Name];
				WriteDescriptorSet = {};
				WriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				WriteDescriptorSet.descriptorCount = layout.descriptorCount;
				WriteDescriptorSet.descriptorType = layout.descriptorType;
				WriteDescriptorSet.dstBinding = layout.binding;
			}

			VkDescriptorSetLayoutCreateInfo DescriptorLayoutCreateInfo{};
			DescriptorLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			DescriptorLayoutCreateInfo.bindingCount = static_cast<uint32_t>(LayoutBindings.size());
			DescriptorLayoutCreateInfo.pBindings = LayoutBindings.data();

			if (x >= m_DescriptorSetLayouts.size())
				m_DescriptorSetLayouts.resize(x + 1);

			if (auto Result = vkCreateDescriptorSetLayout(Device, &DescriptorLayoutCreateInfo, nullptr, &m_DescriptorSetLayouts[x]); Result != VK_SUCCESS)
			{
				TRE_CORE_ERROR("Unable to create descriptor set layout");
				assert(Result == VK_SUCCESS);
			}
		}

		TRE_CORE_INFO("Number of Descriptor Set Layout for {0}: {1}", m_ShaderName, m_DescriptorSetLayouts.size());
	}

	void Shader::SetReflectionData(const ShaderReflectionData& ReflectionData)
	{
		m_ReflectionData = ReflectionData;
	}

	void Shader::SetupShaders()
	{
		std::unique_ptr<PBR> pbr = std::make_unique<PBR>("51e8150cd09be553");
		std::unique_ptr<Line> line = std::make_unique<Line>("7");
	}

	void ShaderDescriptorFile::Load(const std::string& shaderName, const std::string& hexHandle)
	{
		const std::string assetFolderPath = "../Assets/";
		const std::string resourceFolderPath = "../Resources/";
		const std::string resource = hexHandle + ".TREShader";
		const std::string descPath = assetFolderPath + resource + ".desc";
		const std::string resourcePath = resourceFolderPath + resource;
		SetAssetPath(shaderName + ".TREShader");
		SetDescriptorPath(descPath);
		GenerateDescriptorFile();

		//Rename the shader file from english to hex handle
		std::filesystem::rename(resourceFolderPath + shaderName + ".TREShader", resourcePath);
	}
}