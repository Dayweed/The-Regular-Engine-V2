#include "pch.h"
#include "Engine.h"
#include "Scripting/ScriptEngine.h"

#define HAHA_PHYSX_TEST 1
#if HAHA_PHYSX_TEST
#include "PhysX/PxPhysicsAPI.h"
#define HAHA_PHYSX_PVD 1
#endif

#if HAHA_PHYSX_TEST
physx::PxDefaultAllocator		gAllocator;
physx::PxDefaultErrorCallback	gErrorCallback;
physx::PxFoundation* gFoundation = nullptr;
physx::PxPhysics* gPhysics = nullptr;
physx::PxDefaultCpuDispatcher* gDispatcher = nullptr;
physx::PxScene* gScene = nullptr;
physx::PxMaterial* gMaterial = nullptr;
physx::PxPvd* gPvd = nullptr;
physx::PxReal 					stackZ = 10.0f;

void createStack(const physx::PxTransform& t, physx::PxU32 size, physx::PxReal halfExtent)
{
	physx::PxShape* shape = gPhysics->createShape(physx::PxBoxGeometry(halfExtent, halfExtent, halfExtent), *gMaterial);
	for (physx::PxU32 i = 0; i < size; i++)
	{
		for (physx::PxU32 j = 0; j < size - i; j++)
		{
			physx::PxTransform localTm(physx::PxVec3(physx::PxReal(j * 2) - physx::PxReal(size - i), physx::PxReal(i * 2 + 1), 0) * halfExtent);
			physx::PxRigidDynamic* body = gPhysics->createRigidDynamic(t.transform(localTm));
			body->attachShape(*shape);
			physx::PxRigidBodyExt::updateMassAndInertia(*body, 10.0f);
			gScene->addActor(*body);
		}
	}
	shape->release();
}

void HAHAPhysxInit()
{
	gFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, gAllocator, gErrorCallback);
	printf("%d\n", PX_PHYSICS_VERSION);
	printf("%d\n", PX_PHYSICS_VERSION_MAJOR);
	printf("%d\n", PX_PHYSICS_VERSION_MINOR);
	printf("%d\n", PX_PHYSICS_VERSION_BUGFIX);

#if HAHA_PHYSX_PVD
	gPvd = PxCreatePvd(*gFoundation);
	physx::PxPvdTransport* transport = physx::PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
	gPvd->connect(*transport, physx::PxPvdInstrumentationFlag::eALL);
#endif

	gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation, physx::PxTolerancesScale(), true, gPvd);

	physx::PxSceneDesc sceneDesc(gPhysics->getTolerancesScale());
	sceneDesc.gravity = physx::PxVec3(0.0f, -9.81f, 0.0f);
	gDispatcher = physx::PxDefaultCpuDispatcherCreate(2);
	sceneDesc.cpuDispatcher = gDispatcher;
	sceneDesc.filterShader = physx::PxDefaultSimulationFilterShader;
	gScene = gPhysics->createScene(sceneDesc);

#if HAHA_PHYSX_PVD
	physx::PxPvdSceneClient* pvdClient = gScene->getScenePvdClient();
	if (pvdClient)
	{
		pvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
		pvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
		pvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
	}
#endif

	gMaterial = gPhysics->createMaterial(0.5f, 0.5f, 0.6f);

	physx::PxRigidStatic* groundPlane = PxCreatePlane(*gPhysics, physx::PxPlane(0, 1, 0, 0), *gMaterial);
	gScene->addActor(*groundPlane);

	for (physx::PxU32 i = 0; i < 5; i++)
		createStack(physx::PxTransform(physx::PxVec3(0, 0, stackZ -= 10.0f)), 10, 2.0f);
}

void HAHAPhysxUpdate()
{
	gScene->simulate(1.0f / 60.0f);
	gScene->fetchResults(true);
}

#endif

namespace TRE
{
	Engine* Engine::s_Instance = nullptr;

	Engine::Engine()
	{
		s_Instance = this;
		m_Window = std::make_unique<Window>();
		ScriptEngine::InitMono();

#if HAHA_PHYSX_TEST
		HAHAPhysxInit();
#endif
	}

	Engine::~Engine()
	{

	}

	void Engine::Update()
	{
		while (!m_Window->ShouldWindowClose())
		{
			m_Window->PollEvents();

#if HAHA_PHYSX_TEST
			HAHAPhysxUpdate();
#endif
		}
	}

	void Engine::Shutdown()
	{

	}
}