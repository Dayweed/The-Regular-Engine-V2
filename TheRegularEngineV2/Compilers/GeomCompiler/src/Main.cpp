#include "geomcompiler.h"
#include "geom.h"
#include "assimp/Importer.hpp"
#include <filesystem>
#include <iostream>

int main(int argc, char** argv)
{
	std::cout << "===Geom Compiler: Run===" << std::endl;
	TRE::GeomDescriptorFile descriptorFile;

	// ./GeomCompiler.exe "descriptor path"
	if (argc != 2)
	{
		std::cout << "Usage: ./GeomCompiler.exe descriptor_path" << std::endl;
		return 0;
	}
	if(std::filesystem::exists(argv[1]) == false)
	{
		std::cout << "Error: descriptor file does not exist: " << argv[1] << std::endl;
		return 0;
	}
	descriptorFile.ReadDescriptorFile(argv[1]);
	TRE::GeomCompiler::Instance().Compile(descriptorFile.GetAssetPath());
	TRE::Geom::Serialize(descriptorFile.GetGeomPath(), TRE::GeomCompiler::Instance().GetGeom());
	std::cout << "===Geom Compiler: Success===" << std::endl;

	return 0;
}