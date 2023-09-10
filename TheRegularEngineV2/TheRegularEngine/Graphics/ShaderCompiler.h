#pragma once
#include "ShaderResource.h"

namespace TRE
{
	class Shader;

	enum class ShaderLanguage
	{
		NONE, 
		GLSL,
		HLSL
	};

	class ShaderCompiler
	{
		public:
			ShaderCompiler(const std::filesystem::path& ShaderPath, bool EnableOptimization = true);
			~ShaderCompiler() = default;

			static std::shared_ptr<Shader> CompileShader(const std::filesystem::path& ShaderPath, VkShaderStageFlagBits ShaderStage, bool EnableOptimization = true);
		
		private:
			int SkipBOM(std::istream& in); //Can create utilities header file if too much in future
			std::string ReadGLSLToString(const std::string& filename);

			void Compile(VkShaderStageFlagBits ShaderStage);
			std::string PreProcess(const std::string& Source, VkShaderStageFlagBits ShaderStage);
			bool CompileGLSLToBinary(std::vector<uint32_t>& OutputBinary, const std::string& SourceCode, VkShaderStageFlagBits ShaderStage);

			void ReflectShaderData(VkShaderStageFlagBits ShaderStage, const std::vector<uint32_t>& ShaderBinary);
			void ClearReflectionData();

		private:
			std::filesystem::path m_ShaderPath;
			bool m_EnableOptimization = true;
			ShaderLanguage m_ShaderLanguage;
			ShaderReflectionData m_ReflectionData;
			std::vector<uint32_t> m_SPIRVData;
	};
}