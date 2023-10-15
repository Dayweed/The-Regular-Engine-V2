#include <iostream>
#include <memory>
#include <fstream>
#include "ShaderCompiler.h"

int main(int argc, char** argv)
{
	std::cout << "===Shader Compiler: Run===" << std::endl;
	if (argc != 2)
	{
		std::cout << "Usage: ./ShaderCompiler.exe ShaderPath" << std::endl;
		return 0;
	}

	std::string path = argv[1];
	std::unique_ptr<ShaderCompiler> Compiler = std::make_unique<ShaderCompiler>(path);
	Compiler->Compile();

	//Serializing shader into binary
	std::string OutputFilePath = "../Resources/" + Compiler->m_NameOfShader + ".TREshader";
	std::ofstream file(OutputFilePath, std::ios::binary | std::ios::trunc);
	
	int NumberofIterations = static_cast<int>(Compiler->m_SPIRVData.size());
	file.write(reinterpret_cast<const char*>(&NumberofIterations), sizeof(int));

	for (auto& [Stage, Binary] : Compiler->m_SPIRVData)
	{
		int SizeofBinary = Binary.size();
		int ShaderStage = static_cast<int>(Stage);
		file.write(reinterpret_cast<const char*>(&SizeofBinary), sizeof(int));
		file.write(reinterpret_cast<const char*>(&ShaderStage), sizeof(int));
	}

	for (auto& [Stage, Binary] : Compiler->m_SPIRVData)
	{
		file.write(reinterpret_cast<const char*>(&Binary[0]), sizeof(uint32_t) * Binary.size());
	}
	
	file.close();
}