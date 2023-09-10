#include "pch.h"
#include "Shader.h"
#include "RendererContext.h"
#include "Core/Logger.h"

namespace TRE
{
	Shader::Shader(const std::filesystem::path& ShaderPath) : m_ShaderPath(ShaderPath)
	{

	}

	Shader::~Shader()
	{
		VkDevice Device = RendererContext::GetDevice()->GetLogicalDevice();
		vkDestroyShaderModule(Device, m_PipelineShaderCreateInfo.module, nullptr);
	}

	void Shader::LoadAndCreateShader(const std::vector<uint32_t>& ShaderBinary)
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

		m_PipelineShaderCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		m_PipelineShaderCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		m_PipelineShaderCreateInfo.module = ShaderMod;
		m_PipelineShaderCreateInfo.pName = "main";
	}

	void Shader::CreateDescriptors()
	{

	}

	void Shader::SetReflectionData(const ShaderReflectionData& ReflectionData)
	{
		m_ReflectionData = ReflectionData;
	}
}