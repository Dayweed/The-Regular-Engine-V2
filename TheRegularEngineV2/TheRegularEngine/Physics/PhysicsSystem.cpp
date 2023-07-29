#include "pch.h"
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
	}

	void PhysicsSystem::RenderImgui()
	{

	}
	
	void PhysicsSystem::Shutdown()
	{
		std::cout << "Physics System Shutdown" << std::endl;
	}
}