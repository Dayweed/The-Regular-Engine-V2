#include "pch.h"
#include "TREIncludes.h"
#include "PhysicsSystem.h"

namespace TRE
{
	PhysicsSystem::PhysicsSystem()
	{
		std::cout << "Physics System Constructed\n";
	}

	PhysicsSystem::~PhysicsSystem()
	{
		std::cout << "Physics System Destroyed\n";
	}

	bool PhysicsSystem::TESTInit()
	{
		std::cout << "Physics System TESTInit\n";


#if HAHA_PHYSX_TEST
		//Create foundation is similar to initializing the scene
		m_Foundation = PxCreateFoundation(PX_PHYSICS_VERSION, m_Allocator, m_ErrorCallback);

		// doesn't pass in name parameter when allocating stuff on its own now
		// one less thing passed in, the better I guess.
		m_Foundation->setReportAllocationNames(false);

		printf("PhysX Version: %d.%d.%d\n",
			PX_PHYSICS_VERSION_MAJOR, PX_PHYSICS_VERSION_MINOR, PX_PHYSICS_VERSION_BUGFIX);

#if HAHA_PHYSX_PVD
		//PVD is like a debugger for the physics (leave off for submission)
		m_Pvd = PxCreatePvd(*m_Foundation);
		m_Transport = physx::PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
		m_Pvd->connect(*m_Transport, physx::PxPvdInstrumentationFlag::eALL);
#endif

		m_Physics = PxCreatePhysics(PX_PHYSICS_VERSION, *m_Foundation, physx::PxTolerancesScale(), true, m_Pvd);

		physx::PxSceneDesc sceneDesc(m_Physics->getTolerancesScale());
		sceneDesc.gravity = physx::PxVec3(0.0f, -9.81f, 0.0f);
		m_Dispatcher = physx::PxDefaultCpuDispatcherCreate(2);

		//A cpu thread for the scene
		sceneDesc.cpuDispatcher = m_Dispatcher;

		//A thread that will do collision management
		sceneDesc.filterShader = physx::PxDefaultSimulationFilterShader;
		m_Scene = m_Physics->createScene(sceneDesc);

#if HAHA_PHYSX_PVD
		if (physx::PxPvdSceneClient* pvdClient = m_Scene->getScenePvdClient())
		{
			pvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
			pvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
			pvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
		}
		else
		{
			printf("oh no, no flags for my pvd :(\n");
		}
#endif

		//Create material gives the object a static, dynamic and restitution.
		m_Material = m_Physics->createMaterial(0.5f, 0.5f, 0.6f);

		//Static object creation
		m_GroundPlane = PxCreatePlane(*m_Physics, physx::PxPlane(0, 1, 0, 0), *m_Material);
		m_Scene->addActor(*m_GroundPlane);

		//Create 5 stacks of boxes
		for (physx::PxU32 i = 0; i < 5; i++)
			CreateStack(physx::PxTransform(physx::PxVec3(0, 0, m_stackZ -= 10.0f)), 10, 2.0f);
#endif
		return m_IsInitialized = true;
	}

	void PhysicsSystem::Update()
	{
		//if (!m_IsInitialized) TESTInit();
		// makes a non-void function only run once
		// without any if branches, using short-circuiting! :D
		m_IsInitialized || TESTInit();

#if HAHA_PHYSX_TEST
		m_Scene->simulate(1.0f / 60.0f);
		m_Scene->fetchResults(true);
#endif

		/*std::cout << "PhysicsUpdate: Printing useless data m_PosX...---------------------\n";
		for (GO obj : _ecs_manager->GetGO<Transform>())
		{
			std::cout << obj->GetComponent<Transform>().m_PosX << "|";
		}
		std::cout << "\n-------------------------------------------------------------------\n";*/
	}

	void PhysicsSystem::RenderImgui()
	{

	}

	void PhysicsSystem::OnDestroyGO()
	{
		/*std::cout << "Destroy GOs that have transform is to be removed\n";
		for (GO obj : _ecs_manager->GetGO<Transform, Removal>())
		{
			std::cout << "Found object " << obj->GetComponent<Properties>().m_Name << "\n";
		}
		std::cout << "================\n";*/
	}

	void PhysicsSystem::Shutdown()
	{
		std::cout << "Physics System Shutdown\n";
#if HAHA_PHYSX_TEST
		// HOW THE HECK DID THIS MAGICALLY WORK ?!?
		// WAIT I FOUND OUT.
		// NEVER CLOSE THE PVD BEFORE THE APPLICATION AAAAAAAAAAAAA

		PX_RELEASE(m_GroundPlane);
		PX_RELEASE(m_Material);
		PX_RELEASE(m_Scene);
		PX_RELEASE(m_Dispatcher);
		PX_RELEASE(m_Physics);
		PX_RELEASE(m_Transport);
		PX_RELEASE(m_Pvd);
		PX_RELEASE(m_Foundation);
#endif
	}

#if HAHA_PHYSX_TEST
	//This function creates a stack of boxes
	void PhysicsSystem::CreateStack(const physx::PxTransform& t, physx::PxU32 size, physx::PxReal halfExtent)
	{
		//Create the shape of the box
		// physx::PxShape* shape = m_Physics->createShape(physx::PxBoxGeometry(halfExtent, halfExtent, halfExtent), *m_Material);
		physx::PxShape* shape = m_Physics->createShape(physx::PxSphereGeometry(halfExtent), *m_Material);
		//Create the boxes stacked 
		for (physx::PxU32 i = 0; i < size; i++)
		{
			for (physx::PxU32 j = 0; j < size - i; j++)
			{
				//PxTransform is the position represented in a vector multiplied by its halfExtents
				physx::PxTransform localTm(physx::PxVec3(physx::PxReal(j * 2) - physx::PxReal(size - i), physx::PxReal(i * 2 + 1), 0) * halfExtent);
				physx::PxRigidDynamic* body = m_Physics->createRigidDynamic(t.transform(localTm));
				body->attachShape(*shape);
				physx::PxRigidBodyExt::updateMassAndInertia(*body, 10.0f);
				m_Scene->addActor(*body);
			}
		}
		//Release shape
		shape->release();
	}
#endif
}