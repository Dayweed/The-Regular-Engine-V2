#include "GeomCompiler.h"
#include "Geom.h"
#include "Geomdescriptorfile.h"
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
	//std::cout << "Current path: " << std::filesystem::current_path() << std::endl;
	//descriptorFile.ReadDescriptorFile("../../Assets/ee3e77d6dd0374c7.geom.desc");
	TRE::GeomCompiler::Instance().Compile(descriptorFile);
	TRE::Geom::Serialize(descriptorFile.GetResourcePath(), 
	TRE::GeomCompiler::Instance().GetGeom(), 
	TRE::GeomCompiler::Instance().GetSkeleton(), 
	TRE::GeomCompiler::Instance().GetAnimation().m_Animations);
	std::cout << "===Geom Compiler: Success===" << std::endl;

	return 0;
}