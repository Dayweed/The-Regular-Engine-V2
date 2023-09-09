#include "pch.h"
#include "Shader.h"
#include "shaderc/shaderc.hpp"
#include "Utilities/spirv_reflect.h"

namespace TRE
{
	int Shader::SkipBOM(std::istream& in)
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

	std::string Shader::ReadGLSLToString(const std::string& filename)
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

	void Shader::LoadShader()
	{
		std::string Code = ReadGLSLToString("Resources/Shaders/Template.vert");

		shaderc::Compiler ShaderCompiler;
		shaderc::CompileOptions options;
		options.AddMacroDefinition("DEFINE", "1");
		auto PPResult = ShaderCompiler.PreprocessGlsl(Code, shaderc_glsl_default_vertex_shader, "Template.vert", options);

		if (PPResult.GetCompilationStatus() != shaderc_compilation_status_success)
		{
			std::cout << PPResult.GetErrorMessage() << std::endl;
		}
		else
		{
			std::cout << "Success Precompile" << std::endl;
		}

		std::string NewCode = { PPResult.cbegin(), PPResult.cend() };

		auto CompilationResult = ShaderCompiler.CompileGlslToSpv(NewCode, shaderc_glsl_default_vertex_shader, "Template.vert");
		auto status = CompilationResult.GetCompilationStatus();
		if (status == shaderc_compilation_status_success)
		{
			std::cout << "Shader Compile Success" << std::endl;
		}
		else
		{
			std::cout << CompilationResult.GetErrorMessage() << std::endl;
		}

		std::vector<uint32_t> Binary(CompilationResult.begin(), CompilationResult.end());

		SpvReflectShaderModule Mod;
		SpvReflectResult ReflectResult = spvReflectCreateShaderModule(Binary.size() * sizeof(uint32_t), Binary.data(), &Mod);
		if (ReflectResult == SPV_REFLECT_RESULT_SUCCESS)
		{
			std::cout << "Reflect Success" << std::endl;
		}
		else
		{
			std::cout << "Reflect Failed" << std::endl;
		}
		

		uint32_t Count = 0;
		auto Result = spvReflectEnumerateDescriptorSets(&Mod, &Count, nullptr);
		if (Result == SPV_REFLECT_RESULT_SUCCESS)
			std::cout << "Got Count" << std::endl;

		std::vector<SpvReflectDescriptorSet*> Dset(Count);
		auto ReflectDataResult = spvReflectEnumerateDescriptorSets(&Mod, &Count, Dset.data());
		if (ReflectDataResult == SPV_REFLECT_RESULT_SUCCESS)
			std::cout << "Got Dset" << std::endl;

		spvReflectDestroyShaderModule(&Mod);
	}
}