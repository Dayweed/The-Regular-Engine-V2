#pragma once
#include <string>
#include <map>
#include <vector>
#include "vulkan/vulkan.h"

class ShaderCompiler
{
	public:
		ShaderCompiler(std::string Path);
		void Compile();

		std::string m_NameOfShader;
		std::map<VkShaderStageFlagBits, std::vector<uint32_t>> m_SPIRVData;

	private:
		std::map<VkShaderStageFlagBits, std::string> PreProcessCustom(const std::string& Source);
		std::map<VkShaderStageFlagBits, std::string> PreProcessGLSL(const std::string& Source);
		bool CompileGLSLToBinary(std::vector<uint32_t>& OutputBinary, VkShaderStageFlagBits ShaderStage);

	private:
		std::string m_FilePath;
		std::map<VkShaderStageFlagBits, std::string> m_ShaderSourceCode;
};