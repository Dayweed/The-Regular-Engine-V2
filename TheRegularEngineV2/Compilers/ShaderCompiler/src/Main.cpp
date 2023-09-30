#include <fstream>
#include <iostream>
#include <string>
#include "shaderc.hpp"

static int SkipBOM(std::istream& in)
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

static std::string ReadGLSLToString(const std::string& filename)
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

static std::string PreProcess(const std::string& Source, shaderc_shader_kind ShaderStage, std::string Filepath)
{
	shaderc::Compiler ShaderCompiler;
	shaderc::CompileOptions options;
	auto PPResult = ShaderCompiler.PreprocessGlsl(Source, ShaderStage, Filepath.c_str(), options);
	if (PPResult.GetCompilationStatus() != shaderc_compilation_status_success)
	{
		std::cout << "Shader Preprocess Error: " << PPResult.GetErrorMessage() << std::endl;;
	}
	else
	{
		std::cout << "Success Precompiled" << std::endl;
	}

	std::string NewCode = { PPResult.cbegin(), PPResult.cend() };

	return NewCode;
}

static bool CompileGLSLToBinary(std::vector<uint32_t>& OutputBinary, const std::string& SourceCode, shaderc_shader_kind ShaderStage, std::string Filepath)
{
	shaderc::Compiler ShaderCompiler;
	auto CompilationResult = ShaderCompiler.CompileGlslToSpv(SourceCode, ShaderStage, Filepath.c_str());
	auto status = CompilationResult.GetCompilationStatus();
	if (status == shaderc_compilation_status_success)
	{
		std::cout << "Shader Compiled" << std::endl;
		OutputBinary.clear();
		OutputBinary = { CompilationResult.begin(), CompilationResult.end() };
		return true;
	}
	else
	{
		std::cout << "Shader Compile Error: " << CompilationResult.GetErrorMessage() << std::endl;
		return false;
	}

	return true;
}

int main(int argc, char** argv)
{
	std::cout << "===Shader Compiler: Run===" << std::endl;
	if (argc != 2)
	{
		std::cout << "Usage: ./ShaderCompiler.exe ShaderPath" << std::endl;
		return 0;
	}

	std::string path = argv[1];
	std::size_t found = path.find_last_of("/\\");
	std::string name = found != std::string::npos ? path.substr(found + 1) : path;
	found = name.find_last_of('.');
	name = found != std::string::npos ? name.substr(0, found) : name;
	std::string shaderStage = path.substr(path.find_last_of('.') + 1);
	shaderc_shader_kind ShaderStage{};
	
	if (shaderStage == "vert")
	{
		ShaderStage = shaderc_vertex_shader;
	}
	else if (shaderStage == "frag")
	{
		ShaderStage = shaderc_fragment_shader;
	}
	else
		std::cout << "Shader stage not supported" << std::endl;
	
	std::string Source = ReadGLSLToString(path);
	std::string PreprocessedCode = PreProcess(Source, ShaderStage, path);

	std::vector<uint32_t> ShaderBinary;
	if (auto Compiled = CompileGLSLToBinary(ShaderBinary, PreprocessedCode, ShaderStage, path); Compiled == false)
	{
		std::cout << "Failed to compile shader" << std::endl;
	}
	else
	{
		std::cout << "Shader Compiled Successfully" << std::endl;
	}

	//Serializing shader into binary
	std::string OutputFilePath = "../Resources/" + name + ".TREshader";
	std::ofstream file(OutputFilePath, std::ios::binary | std::ios::trunc);

	file.write(reinterpret_cast<const char*>(&ShaderBinary[0]), ShaderBinary.size() * sizeof(uint32_t));
	file.close();
}