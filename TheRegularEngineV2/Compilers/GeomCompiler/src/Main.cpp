#include "geomcompiler.h"
#include "geom.h"
#include "assimp/Importer.hpp"
#include <iostream>

int main()
{
	std::cout << "Hello World!\n";
	TRE::_geom_compiler->Compile("../../Assets/smooth_vase.obj");
	TRE::_geom_compiler->Serialize("../../Assets/smooth_vase.geom");
	return 0;
}