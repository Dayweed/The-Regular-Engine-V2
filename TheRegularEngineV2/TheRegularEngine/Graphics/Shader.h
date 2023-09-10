#pragma once
#include "ShaderResource.h"

namespace TRE
{
	class Shader
	{
		public:
			Shader() = default;
			Shader(const std::filesystem::path& ShaderPath);
			~Shader();
			
			void LoadAndCreateShader(const std::vector<uint32_t>& ShaderBinary);
			void SetReflectionData(const ShaderReflectionData& ReflectionData);
			void CreateDescriptors();

		private:
			VkPipelineShaderStageCreateInfo m_PipelineShaderCreateInfo;
			std::vector<uint32_t> m_ShaderBinary;
			ShaderReflectionData m_ReflectionData;

			std::vector<VkDescriptorSetLayout> m_DescriptorSetLayouts;
			VkDescriptorSet m_DescriptorSet;

			std::filesystem::path m_ShaderPath;
			std::string m_ShaderName;

			friend class ShaderCompiler;
	};
}