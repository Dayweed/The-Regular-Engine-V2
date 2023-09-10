#include "pch.h"
#include "ShaderCompiler.h"
#include "shaderc/shaderc.hpp"
#include "Core/Logger.h"
#include "Shader.h"
#include "spirv_cross/spirv_glsl.hpp"

namespace TRE
{
	ShaderCompiler::ShaderCompiler(const std::filesystem::path& ShaderPath, bool EnableOptimization) : m_ShaderPath(ShaderPath), m_EnableOptimization(EnableOptimization)
	{
		m_ShaderLanguage = ShaderLanguage::GLSL; //For the sake of allowing it to be modular in future
	}

	std::shared_ptr<Shader> ShaderCompiler::CompileShader(const std::filesystem::path& ShaderPath, bool EnableOptimization)
	{
		std::string path = ShaderPath.string();
		size_t found = path.find_last_of("/\\");
		std::string name = found != std::string::npos ? path.substr(found + 1) : path;
		found = name.find_last_of('.');
		name = found != std::string::npos ? name.substr(0, found) : name;
		TRE_CORE_INFO("Shader Name: {0}", name);

		std::shared_ptr<ShaderCompiler> Compiler = std::make_shared<ShaderCompiler>(ShaderPath, true);
		Compiler->Compile();

		std::shared_ptr<Shader> GeneratedShader = std::make_shared<Shader>(ShaderPath);
		GeneratedShader->m_ShaderName = name;
		GeneratedShader->LoadAndCreateShader(Compiler->m_SPIRVData);
		GeneratedShader->SetReflectionData(Compiler->m_ReflectionData);
		GeneratedShader->CreateDescriptors();

		return GeneratedShader;
	}

	void ShaderCompiler::Compile()
	{
		m_SPIRVData.clear();

		std::string RawCode = ReadGLSLToString(m_ShaderPath.string());
		std::string PreProcessCode = PreProcess(RawCode);
		
		if (auto Compiled = CompileGLSLToBinary(m_SPIRVData, PreProcessCode); Compiled == false)
		{
			TRE_CORE_ERROR("Failed to compile shader");
		}
		else
		{
			TRE_CORE_INFO("Shader Compiled");
		}

		ReflectShaderData(VK_SHADER_STAGE_VERTEX_BIT, m_SPIRVData);

	}

	std::string ShaderCompiler::PreProcess(const std::string& Source)
	{
		shaderc::Compiler ShaderCompiler;
		shaderc::CompileOptions options;
		auto PPResult = ShaderCompiler.PreprocessGlsl(Source, shaderc_glsl_default_vertex_shader, m_ShaderPath.string().c_str(), options);
		if (PPResult.GetCompilationStatus() != shaderc_compilation_status_success)
		{
			TRE_CORE_INFO("Shader Preprocess Error: {0}", PPResult.GetErrorMessage());
		}
		else
		{
			TRE_CORE_INFO("Success Precompiled");
		}

		std::string NewCode = { PPResult.cbegin(), PPResult.cend() };

		return NewCode;
	}

	bool ShaderCompiler::CompileGLSLToBinary(std::vector<uint32_t>& OutputBinary, const std::string& SourceCode)
	{
		shaderc::Compiler ShaderCompiler;
		auto CompilationResult = ShaderCompiler.CompileGlslToSpv(SourceCode, shaderc_glsl_default_vertex_shader, m_ShaderPath.string().c_str());
		auto status = CompilationResult.GetCompilationStatus();
		if (status == shaderc_compilation_status_success)
		{
			TRE_CORE_INFO("Shader Compiled");
			OutputBinary.clear();
			OutputBinary = { CompilationResult.begin(), CompilationResult.end() };
			return true;
		}
		else
		{
			TRE_CORE_INFO("Shader Compile Error: {0}", CompilationResult.GetErrorMessage());
			return false;
		}

		return true;
	}

	void ShaderCompiler::ReflectShaderData(VkShaderStageFlagBits ShaderStage, const std::vector<uint32_t>& ShaderBinary)
	{
		spirv_cross::Compiler Compiler(ShaderBinary);
		auto Resources = Compiler.get_shader_resources();

		TRE_CORE_INFO("Reflecting Uniform Buffers, Size: {0}", Resources.uniform_buffers.size());
		for (const auto& resource : Resources.uniform_buffers)
		{

		}

		TRE_CORE_INFO("Reflecting PushConstants, Size: {0}", Resources.push_constant_buffers.size());
		for (const auto& resource : Resources.push_constant_buffers)
		{

		}
	}

	void ShaderCompiler::ClearReflectionData()
	{
		m_ReflectionData.DescriptorSets.clear();
		m_ReflectionData.PushConstants.clear();
	}

	std::string ShaderCompiler::ReadGLSLToString(const std::string& filename)
	{
		std::ifstream file(filename, std::ios::in | std::ios::binary);
		std::string Result;

		if (!file.is_open()) {
			throw std::runtime_error("failed to open file!");
		}

		file.seekg(0, std::ios::end);
		auto FileSize = file.tellg();
		const int Skipped = SkipBOM(file);
		FileSize -= Skipped - 1;

		Result.resize(FileSize);

		file.read(Result.data() + 1, FileSize);
		Result[0] = '\t';
		file.close();

		return Result;
	}
	
	int ShaderCompiler::SkipBOM(std::istream& in)
	{
		char test[4] = { 0 };
		in.seekg(0, std::ios::beg);
		in.read(test, 3);
		if (strcmp(test, "\xEF\xBB\xBF") == 0)
		{
			in.seekg(3, std::ios::beg);
			return 3;
		}
		in.seekg(0, std::ios::beg);
		return 0;
	}
}