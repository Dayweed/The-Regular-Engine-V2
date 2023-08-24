#include "pch.h"
#include "TREIncludes.h"
#include "PhysicsSystem.h"

#define GLMVec3ToPxVec3(glmVec) (physx::PxVec3{(glmVec).x, (glmVec).y, (glmVec).z})
#define PxVec3ToGLMVec3(pxVec) (glm::vec3{(pxVec).x, (pxVec).y, (pxVec).z})

namespace TRE
{
	PhysicsSystem::PhysicsSystem()
	{
		printf("Physics System Constructor called\n");
		printf("Initializing Physics/PhysX systems...\n");

		//Create foundation is similar to initializing the scene
		m_Foundation = PxCreateFoundation(PX_PHYSICS_VERSION, m_Allocator, m_ErrorCallback);
		assert(m_Foundation);

		// doesn't pass in name parameter when allocating stuff on its own now
		// one less thing passed in, the better I guess.
		m_Foundation->setReportAllocationNames(false);

		printf("PhysX Version: %d.%d.%d\n",
			PX_PHYSICS_VERSION_MAJOR, PX_PHYSICS_VERSION_MINOR, PX_PHYSICS_VERSION_BUGFIX);

#if USE_PHYSX_PVD
		//PVD is like a debugger for the physics (leave off for submission)
		m_Pvd = PxCreatePvd(*m_Foundation);
		assert(m_Pvd);

		m_Transport = physx::PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
		assert(m_Transport);

		m_Pvd->connect(*m_Transport, physx::PxPvdInstrumentationFlag::eALL);
		/* ^ this line gives the following PhysX error when PVD is enabled AND connected... for some reason... :(
		code was		: |32|
		message was		: |Failed to load PhysXGpu_64.dll!|
		from file		: |C:\Users\jerel\Downloads\PhysX-release-104.2\PhysX-release-104.2\physx\source\physx\src\gpu\PxPhysXGpuModuleLoader.cpp|
		at line			: |148|
		*/
#endif

		// set trackOutstandingAllocations to `true` for tracking memory allocations
		m_Physics = PxCreatePhysics(PX_PHYSICS_VERSION, *m_Foundation, physx::PxTolerancesScale(), false, m_Pvd);
		assert(m_Physics);

		physx::PxSceneDesc sceneDesc(m_Physics->getTolerancesScale());
		sceneDesc.gravity = physx::PxVec3(0.0f, -9.81f, 0.0f);

		//A cpu thread for the scene
		m_Dispatcher = physx::PxDefaultCpuDispatcherCreate(2);
		assert(m_Dispatcher);
		sceneDesc.cpuDispatcher = m_Dispatcher;

		//A thread that will do collision management
		sceneDesc.filterShader = physx::PxDefaultSimulationFilterShader;
		m_Scene = m_Physics->createScene(sceneDesc);
		assert(m_Scene);

#if USE_PHYSX_PVD
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

		printf("Physics/PhysX systems initialization complete! :D\n");
	}

	PhysicsSystem::~PhysicsSystem()
	{
		printf("Physics System Destructor called\n");
	}

	bool PhysicsSystem::TESTUpdate()
	{
		const float stackInitialZ = 10.0f, stackSeparation = 10.0f, shapeHalfExtent = 2.0f;
		const unsigned stackSize = 3, numOfStacks = 1;

		for (physx::PxU32 i = 0; i < numOfStacks; i++)
			CreateStack(physx::PxTransform({ 0, 0, stackInitialZ - (stackSeparation * i) }), stackSize, shapeHalfExtent);

		return m_IsReadyForUpdate = true;
	}

	void PhysicsSystem::Update()
	{
		//if (!m_IsReadyForUpdate) TESTUpdate();
		// makes a non-void function only run once
		// without any if branches, using short-circuiting! :D
		m_IsReadyForUpdate || TESTUpdate();

		// deletes a SphereCollider every second, if found
		static std::time_t start_timer = std::time(nullptr);
		auto const result = std::time(nullptr) - start_timer;
		if (result >= 1)
		{
			auto vec = _ecs_manager->GetGO<SphereCollider>();
			if (!vec.empty())
				DestructSphereCollider(vec.front());
			std::time(&start_timer);
		}

		m_Scene->simulate(1.0f / 60.0f);
		m_Scene->fetchResults(true);
	}

	void OLDSTUFF_Update()
	{
		/*std::cout << "PhysicsUpdate: Printing useless data m_PosX...---------------------\n";
		for (GO obj : _ecs_manager->GetGO<Transform>())
		{
			std::cout << obj->GetComponent<Transform>().m_PosX << "|";
		}
		std::cout << "\n-------------------------------------------------------------------\n";*/
	}

	void OLDSTUFF_OnDestroyGO()
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
		printf("Physics System Shutdown\n");
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
		/* ^ this line gives the following PhysX error when PVD is enabled AND connected... for some reason... :(
		code was		: |32|
		message was		: |Failed to load PhysXGpu_64.dll!|
		from file		: |C:\Users\jerel\Downloads\PhysX-release-104.2\PhysX-release-104.2\physx\source\physx\src\gpu\PxPhysXGpuModuleLoader.cpp|
		at line			: |148|
		*/
		PX_RELEASE(m_Foundation);
	}

	void PhysicsSystem::ConstructSphereCollider(const GO& go, const float radius, const glm::vec3& offset) const
	{
		auto& sphereCollider = go->AddComponent<SphereCollider>();
		// add component if missing, otherwise get existing component

		const auto objPos = go->GetComponent<Transform>().m_Position;
		const physx::PxVec3 colliderPos = GLMVec3ToPxVec3(objPos) + GLMVec3ToPxVec3(offset);

		// create a 'container', that being the PxActor (specifically a PxRigidDynamic in this case)
		physx::PxRigidDynamic* body = m_Physics->createRigidDynamic(physx::PxTransform(colliderPos));

		// fill the 'container' with a shape, that being a sphere
		physx::PxShape* shape = m_Physics->createShape(physx::PxSphereGeometry(radius), *m_Material);
		body->attachShape(*shape);

		physx::PxRigidBodyExt::updateMassAndInertia(*body, 10.0f);

		m_Scene->addActor(*body);
		sphereCollider.m_RigidActor = body;
		sphereCollider.m_Radius = radius;

		shape->release();

		/*
		Obtaining the shape of an actor (e.g physx::PxGeometryType::Enum::eSPHERE)

		const int nbShapes = sphereCollider.m_RigidActor->getNbShapes();
		physx::PxShape** buffer = new physx::PxShape * [nbShapes + 1];
		const int actualNbShapes = sphereCollider.m_RigidActor->getShapes(buffer, nbShapes + 1);

		for (int i = 0; i < actualNbShapes; ++i)
			printf("%d\n", buffer[i]->getGeometry().getType());
		*/
	}

	void PhysicsSystem::DestructSphereCollider(const GO& go) const
	{
		go->GetComponent<SphereCollider>().m_RigidActor->release();
		go->RemoveComponent<SphereCollider>();
	}

	void PhysicsSystem::ConstructBoxCollider(const GO& go, const glm::vec3& halfExtents, const glm::vec3& offset) const
	{
		auto& boxCollider = go->AddComponent<BoxCollider>();
		// add component if missing, otherwise get existing component

		const auto objPos = go->GetComponent<Transform>().m_Position;
		const physx::PxVec3 colliderPos = GLMVec3ToPxVec3(objPos) + GLMVec3ToPxVec3(offset);

		// create a 'container', that being the PxActor (specifically a PxRigidDynamic in this case)
		physx::PxRigidDynamic* body = m_Physics->createRigidDynamic(physx::PxTransform(colliderPos));

		// fill the 'container' with a shape, that being a box
		physx::PxShape* shape = m_Physics->createShape(physx::PxBoxGeometry(GLMVec3ToPxVec3(halfExtents)), *m_Material);
		body->attachShape(*shape);

		physx::PxRigidBodyExt::updateMassAndInertia(*body, 10.0f);

		m_Scene->addActor(*body);
		boxCollider.m_RigidActor = body;
		boxCollider.m_HalfExtents = halfExtents;

		shape->release();
	}

	void PhysicsSystem::DestructBoxCollider(const GO& go) const
	{
		go->GetComponent<BoxCollider>().m_RigidActor->release();
		go->RemoveComponent<BoxCollider>();
	}

	//This function creates a stack of shapes
	void PhysicsSystem::CreateStack(const physx::PxTransform& t, unsigned size, float halfExtent) const
	{
		for (unsigned i = 0; i < size; i++)
		{
			for (unsigned j = 0; j < size - i; j++)
			{
				GO go = _ecs_manager->CreateGO();
				const physx::PxVec3 stackPos{ (2.0f * j) - (size - i) , 2.0f * i + 1 , 0 };
				const physx::PxVec3 newPos = t.transform(halfExtent * stackPos);
				go->GetComponent<Transform>().m_Position = PxVec3ToGLMVec3(newPos);
				// ConstructSphereCollider(go, halfExtent);
				ConstructBoxCollider(go, glm::vec3{ halfExtent });
			}
		}
	}
}
