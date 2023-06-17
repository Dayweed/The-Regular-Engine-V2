#include "EditorLayer.h"

namespace TRE
{
	EditorLayer::EditorLayer()
	{
		std::cout << "Editor Init" << std::endl;
	}
	
	EditorLayer::~EditorLayer()
	{

	}

	void EditorLayer::Update()
	{

	}

	void EditorLayer::Shutdown()
	{
		std::cout << "Editor Shutdown" << std::endl;
	}
}