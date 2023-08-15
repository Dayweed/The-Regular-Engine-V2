#include "pch.h"
#include "TREIncludes.h"
#include "PhysicsSystem.h"

namespace TRE
{
	PhysicsSystem::PhysicsSystem()
	{
		std::cout << "Physics System Init" << std::endl;
	}

	PhysicsSystem::~PhysicsSystem()
	{

	}

	void PhysicsSystem::Update()
	{
		std::cout << "PhysicsUpdate: Printing useless data m_PosX...---------------------\n";
		for (GO obj : _ecs_manager->GetGO<Transform>())
		{
			std::cout << obj->GetComponent<Transform>().m_PosX << "|";
		}
		std::cout << "\n-------------------------------------------------------------------\n";
	}

	void PhysicsSystem::RenderImgui()
	{

	}
	
	void PhysicsSystem::Shutdown()
	{
		std::cout << "Physics System Shutdown" << std::endl;
	}
}