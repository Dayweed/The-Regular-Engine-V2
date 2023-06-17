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
		//std::cout << "Physics System Update" << std::endl;
	}
	
	void PhysicsSystem::Shutdown()
	{
		std::cout << "Physics System Shutdown" << std::endl;
	}
}