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

			static std::unique_ptr<Shader> CompileShader(const std::filesystem::path& ShaderPath, bool EnableOptimization = true);
			//static std::unique_ptr<Shader> DeserializeReflectShader(const std::filesystem::path& ShaderPath, bool EnableOptimization = true); //Temp
			//void DeserializeShaderBinary(std::string FilePath);
		
		private:
			int SkipBOM(std::istream& in); //Can create utilities header file if too much in future
			std::string ReadGLSLToString(const std::string& filename);

			void Compile();
			std::map<VkShaderStageFlagBits, std::string> PreProcessCustom(const std::string& Source);
			std::map<VkShaderStageFlagBits, std::string> PreProcessGLSL(const std::string& Source);
			bool CompileGLSLToBinary(std::vector<uint32_t>& OutputBinary, VkShaderStageFlagBits ShaderStage);

			void ReflectShaderData(const std::map<VkShaderStageFlagBits, std::vector<uint32_t>>& ShaderBinary);
			void Reflect(VkShaderStageFlagBits ShaderStage, const std::vector<uint32_t>& ShaderBinary);
			void ClearReflectionData();

		private:
			std::filesystem::path m_ShaderPath;
			bool m_EnableOptimization = true;
			ShaderLanguage m_ShaderLanguage;
			ShaderReflectionData m_ReflectionData;
			std::map<VkShaderStageFlagBits, std::vector<uint32_t>> m_SPIRVData;

			std::map<VkShaderStageFlagBits, std::string> m_ShaderSourceCode;
	};
}