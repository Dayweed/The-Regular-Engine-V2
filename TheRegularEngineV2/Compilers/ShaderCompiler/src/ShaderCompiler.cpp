#include "ShaderCompiler.h"
#include <fstream>
#include <iostream>
#include <assert.h>
#include <regex>
#include "shaderc.hpp"

static VkShaderStageFlagBits ShaderTypeFromString(const std::string_view type)
{
	if (type == "vert")	return VK_SHADER_STAGE_VERTEX_BIT;
	if (type == "frag")	return VK_SHADER_STAGE_FRAGMENT_BIT;
	if (type == "comp")	return VK_SHADER_STAGE_COMPUTE_BIT;

	return VK_SHADER_STAGE_ALL;
}

static std::vector<std::string> SplitStringAndKeepDelims(std::string str)
{
	const static std::regex re(R"((^\W|^\w+)|(\w+)|[:()])", std::regex_constants::optimize);

	std::regex_iterator<std::string::iterator> rit(str.begin(), str.end(), re);
	std::regex_iterator<std::string::iterator> rend;
	std::vector<std::string> result;

	while (rit != rend)
	{
		result.emplace_back(rit->str());
		++rit;
	}
	return result;
}

static shaderc_shader_kind VulkanStageToShaderC(const VkShaderStageFlagBits stage)
{
	switch (stage)
	{
		case VK_SHADER_STAGE_VERTEX_BIT:    return shaderc_vertex_shader;
		case VK_SHADER_STAGE_FRAGMENT_BIT:  return shaderc_fragment_shader;
		case VK_SHADER_STAGE_COMPUTE_BIT:   return shaderc_compute_shader;
	}
	
	return {};
}

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

ShaderCompiler::ShaderCompiler(std::string Path) : m_FilePath(Path)
{
}

void ShaderCompiler::Compile()
{
	std::string path = m_FilePath;
	size_t found = path.find_last_of("/\\");
	std::string name = found != std::string::npos ? path.substr(found + 1) : path;
	found = name.find_last_of('.');
	name = found != std::string::npos ? name.substr(0, found) : name;
	std::cout << "Shader Name: " << name << std::endl;
	m_NameOfShader = name;

	std::string RawCode = ReadGLSLToString(m_FilePath);
	m_ShaderSourceCode = PreProcessGLSL(RawCode);

	//Compile shaders
	for (auto& [Stage, Source] : m_ShaderSourceCode)
	{
		if (auto Compiled = CompileGLSLToBinary(m_SPIRVData[Stage], Stage); Compiled == false)
		{
			std::cout << "Failed to compile shader" << std::endl;
		}
		else
		{
			std::string StageString;
			if (Stage == VK_SHADER_STAGE_VERTEX_BIT)
			{
				StageString = "Vertex";
			}
			else if (Stage == VK_SHADER_STAGE_FRAGMENT_BIT)
			{
				StageString = "Fragment";
			}
			std::cout << m_FilePath << " " << StageString << " stage compiled" << std::endl;
		}
	}
}

std::map<VkShaderStageFlagBits, std::string> ShaderCompiler::PreProcessGLSL(const std::string& Source)
{
	std::map<VkShaderStageFlagBits, std::string> ShaderSources = PreProcessCustom(Source);

	static shaderc::Compiler ShaderCompiler;

	for (auto& [Stage, Source] : ShaderSources)
	{
		shaderc::CompileOptions options;

		const auto PreProcessResult = ShaderCompiler.PreprocessGlsl(Source, VulkanStageToShaderC(Stage), m_FilePath.c_str(), options);
		if (PreProcessResult.GetCompilationStatus() != shaderc_compilation_status_success)
		{
			std::cout << "Failed to preprocess " << m_FilePath << " shader" << std::endl;
		}

		Source = std::string(PreProcessResult.begin(), PreProcessResult.end());
	}

	return ShaderSources;
}

std::map<VkShaderStageFlagBits, std::string> ShaderCompiler::PreProcessCustom(const std::string& Source)
{
	std::string NewSource = Source;
	std::map<VkShaderStageFlagBits, std::string> ShaderSources;
	std::vector<std::pair<VkShaderStageFlagBits, size_t>> Positions;

	size_t Start = 0;
	size_t Pos = NewSource.find('#');

	Pos = NewSource.find('#', Pos + 1);

	while (Pos != std::string::npos)
	{
		const size_t EndOfLine = NewSource.find_first_of("\r\n", Pos) + 1;
		size_t Index = 1;
		std::vector<std::string> tokens = SplitStringAndKeepDelims(NewSource.substr(Pos, EndOfLine - Pos));

		if (tokens[Index] == "pragma")
		{
			++Index;
			if (tokens[Index] == "stage")
			{
				++Index;
				assert(tokens[Index] == ":" && "Shader stage not declared properly");
				++Index; //Ignores ':'

				const std::string_view stage = tokens[Index];
				assert((stage == "vert" || stage == "frag" || stage == "comp") && "Shader stage not supported"); //comp = compute shader if we goes into it
				auto ShaderStage = ShaderTypeFromString(stage);

				Positions.emplace_back(ShaderStage, Start);
			}
		}
		else if constexpr (true)
		{
			if (tokens[Index] == "version")
			{
				++Index;
				Start = Pos;
			}
		}

		Pos = NewSource.find('#', Pos + 1);
	}

	assert(Positions.size() && "Could not load shader as no stages are found");

	auto& [FirstStage, FirstPosition] = Positions[0];
	if (Positions.size() > 1)
	{
		//Get first stage
		const std::string firstStageStr = NewSource.substr(0, Positions[1].second);
		size_t lineCount = std::count(firstStageStr.begin(), firstStageStr.end(), '\n') + 1;
		ShaderSources[FirstStage] = firstStageStr;


		//Get stages in the middle
		for (size_t i = 1; i < Positions.size() - 1; ++i)
		{
			auto& [stage, stagePos] = Positions[i];
			std::string stageStr = NewSource.substr(stagePos, Positions[i + 1].second - stagePos);
			const size_t secondLinePos = stageStr.find_first_of('\n', 1) + 1;
			//stageStr.insert(secondLinePos, fmt::format("#line {}\n", lineCount));
			ShaderSources[stage] = stageStr;
			lineCount += std::count(stageStr.begin(), stageStr.end(), '\n') + 1;
		}

		//Get last stage
		auto& [stage, stagePos] = Positions[Positions.size() - 1];
		std::string lastStageStr = NewSource.substr(stagePos);
		const size_t secondLinePos = lastStageStr.find_first_of('\n', 1) + 1;
		//lastStageStr.insert(secondLinePos, fmt::format("#line {}\n", lineCount + 1));
		ShaderSources[stage] = lastStageStr;
	}
	else
	{
		ShaderSources[FirstStage] = Source;
	}

	return ShaderSources;
}

bool ShaderCompiler::CompileGLSLToBinary(std::vector<uint32_t>& OutputBinary, VkShaderStageFlagBits ShaderStage)
{
	static shaderc::Compiler ShaderCompiler;
	std::string SourceCode = m_ShaderSourceCode[ShaderStage];

	auto CompilationResult = ShaderCompiler.CompileGlslToSpv(SourceCode, VulkanStageToShaderC(ShaderStage), m_FilePath.c_str());
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