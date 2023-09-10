#include "TextureCompiler.h"
#include <iostream>
#include <filesystem>

int main(int argc, char** argv)
{
	std::cout << "===Texture Compiler: Run===" << std::endl;
	TRE::TextureDescriptorFile descriptorFile;
	if (argc != 2)
	{
		std::cout << "Usage: ./TextureCompiler.exe descriptor_path" << std::endl;
		return 0;
	}
	if (std::filesystem::exists(argv[1]) == false)
	{
		std::cout << "Error: descriptor file does not exist: " << argv[1] << std::endl;
		return 0;
	}
	descriptorFile.ReadDescriptorFile(argv[1]);
	TRE::TextureCompiler::Instance().Compile(descriptorFile);
	TRE::Texture::Serialize(descriptorFile.GetTexturePath(), TRE::TextureCompiler::Instance().GetTexture());
	std::cout << "===Texture Compiler: Success===" << std::endl;
	return 0;
}
