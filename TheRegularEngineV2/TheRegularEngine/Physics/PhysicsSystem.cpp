/*!
	@file      PhysicsSystem.cpp
	@author    Prashanth Subrahmanyam Sharma (Code Contribution 100%)
	@email     p.sharma@digipen.edu
	@date      03/09/2023
	@brief     This file contains the definition of the PhysicsSystem
			   member functions, which is an interface for PhysX functions.

	Copyright (C) 2023 DigiPen Institute of Technology.
	Reproduction or disclosure of this file or its contents without the
	prior written consent of DigiPen Institute of Technology is prohibited.
************************************************************************/
#include "pch.h"
#include "TREIncludes.h"
#include "PhysicsSystem.h"

// USE_PHYSX_PVD is not defined in Release
#ifdef _DEBUG
#define USE_PHYSX_PVD 0
#endif

#pragma region Macros


#define UNUSED_PARAM(param) (void)param

// not the best name... :/
#define UpdateAllEntitiesWithComponent(Type)\
	for (const Entity & entity : ECSManager::Instance().GetEntities<Type>()) Update##Type(entity)

// this name is even worse! :_(
#define OnEntityMismatchDestroyComponent(entity, attachedComponents, Type)\
	if ((attachedComponents) & PhysicsComponentTypes::##Type && !(entity)->HasComponent<Type>()) Destruct##Type(entity)

#pragma endregion

using namespace physx;
// to save my dwindling sanity

#if 0
namespace Text
{
	const std::string reset{ "\033[0m" };
	const std::string red{ "\033[0;31m" };
	const std::string green{ "\033[0;32m" };
	const std::string blue{ "\033[0;36m" };
	const std::string crit{ "\033[37;41m" };
}
#else
namespace Text
{
	const std::string reset{ "" };
	const std::string red{ "" };
	const std::string green{ "" };
	const std::string blue{ "" };
	const std::string crit{ "" };
}
#endif

namespace TRE
{
	PhysicsSystem::PhysicsSystem()
	{
		TRE_CORE_INFO("Physics System Constructor called");
		TRE_CORE_INFO("Initializing Physics/PhysX systems...");

		//Create foundation is similar to initializing the scene
		m_Foundation = PxCreateFoundation(PX_PHYSICS_VERSION, m_Allocator, m_ErrorCallback);
		assert(m_Foundation);

		// doesn't pass in name parameter when allocating stuff on its own now
		// one less thing passed in, the better I guess.
		m_Foundation->setReportAllocationNames(_DEBUG);

		TRE_CORE_INFO("PhysX Version: {0}.{1}.{2}",
			PX_PHYSICS_VERSION_MAJOR, PX_PHYSICS_VERSION_MINOR, PX_PHYSICS_VERSION_BUGFIX);

#if USE_PHYSX_PVD
		//PVD is like a debugger for the physics (leave off for submission)
		m_Pvd = PxCreatePvd(*m_Foundation);
		assert(m_Pvd);

		m_Transport = PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
		assert(m_Transport);

		m_Pvd->connect(*m_Transport, PxPvdInstrumentationFlag::eALL);
		/* ^ this line gives the following PhysX error when PVD is enabled AND connected... for some reason... :(
		code was		: |32|
		message was		: |Failed to load PhysXGpu_64.dll!|
		from file		: |C:\Users\jerel\Downloads\PhysX-release-104.2\PhysX-release-104.2\physx\source\physx\src\gpu\PxPhysXGpuModuleLoader.cpp|
		at line			: |148|
		*/
#endif

		// set trackOutstandingAllocations to `true` for tracking memory allocations
		m_Physics = PxCreatePhysics(PX_PHYSICS_VERSION, *m_Foundation, PxTolerancesScale(), false, m_Pvd);
		assert(m_Physics);

		PxInitExtensions(*m_Physics, m_Pvd);

		PxSceneDesc sceneDesc(m_Physics->getTolerancesScale());
		sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);

		//A cpu thread for the scene
		m_Dispatcher = PxDefaultCpuDispatcherCreate(2);
		assert(m_Dispatcher);
		sceneDesc.cpuDispatcher = m_Dispatcher;

		// SimulationEventCallback must inherit PxSimulationEventCallback
		// PUBLICLY in order to work, otherwise...
		// C2243: 'type cast': conversion from 'TRE::SimulationEventCallback *'
		// to 'physx::PxSimulationEventCallback *' exists, but is inaccessible
		sceneDesc.simulationEventCallback = &m_SimulationEventCallback;

		//A thread that will do collision management

		// sceneDesc.filterShader = PxDefaultSimulationFilterShader;
		sceneDesc.filterShader = SimulationFilterShader;

		m_Scene = m_Physics->createScene(sceneDesc);
		assert(m_Scene);

#if USE_PHYSX_PVD
		if (PxPvdSceneClient* pvdClient = m_Scene->getScenePvdClient())
		{
			PxPvdSceneFlags sceneFlags = pvdClient->getScenePvdFlags();
			sceneFlags.raise(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS);
			sceneFlags.raise(PxPvdSceneFlag::eTRANSMIT_CONTACTS);
			sceneFlags.raise(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES);
			pvdClient->setScenePvdFlags(sceneFlags);
		}
		else
		{
			TRE_CORE_WARN("PhysicsSystem::PhysicsSystem() - Unable to obain scene's PVD client.");
		}
#endif

		//Create material gives the object a static, dynamic and restitution.
		m_DefaultMaterial = m_Physics->createMaterial(0.5f, 0.5f, 0.6f);

		//Static object creation
		m_GroundPlane = PxCreatePlane(*m_Physics, PxPlane(0, 1, 0, 0.5), *m_DefaultMaterial);
#ifdef _DEBUG
		m_GroundPlane->setName("THE PLANE");
#endif
		m_Scene->addActor(*m_GroundPlane);

		TRE_CORE_INFO("Physics/PhysX systems initialization complete! :D");
	}

	// largely identical to PhysX's SnippetTriggers implementation
	PxFilterFlags SimulationFilterShader(PxFilterObjectAttributes attributes0, PxFilterData filterData0,
		PxFilterObjectAttributes attributes1, PxFilterData filterData1,
		PxPairFlags& pairFlags, const void* constantBlock, PxU32 constantBlockSize)
	{
		UNUSED_PARAM(filterData0);
		UNUSED_PARAM(filterData1);
		UNUSED_PARAM(constantBlock);
		UNUSED_PARAM(constantBlockSize);

		// let triggers through
		if (PxFilterObjectIsTrigger(attributes0) || PxFilterObjectIsTrigger(attributes1))
		{
			pairFlags = PxPairFlag::eTRIGGER_DEFAULT;
			return PxFilterFlags();
		}

		pairFlags = PxPairFlag::eCONTACT_DEFAULT
			| PxPairFlag::eDETECT_CCD_CONTACT
			| PxPairFlag::eCONTACT_EVENT_POSE
			| PxPairFlag::eNOTIFY_TOUCH_CCD
			// | PxPairFlag::eNOTIFY_CONTACT_POINTS
			| PxPairFlag::eNOTIFY_TOUCH_FOUND // for OnCollisionEnter
			| PxPairFlag::eNOTIFY_TOUCH_PERSISTS // added for OnCollisionStay
			| PxPairFlag::eNOTIFY_TOUCH_LOST // added for OnCollisionExit
		;

		return {};
	}

	static bool isReadyForUpdate = false;

	bool PhysicsSystem::TESTUpdate()
	{
#if 0
		const float stackInitialZ = 10.0f, stackSeparation = 10.0f, shapeHalfExtent = 2.0f;
		const unsigned stackSize = 3, numOfStacks = 1;

		for (PxU32 i = 0; i < numOfStacks; i++)
			CreateStack({ 0, 0, stackInitialZ - (stackSeparation * i) }, stackSize, shapeHalfExtent);
#endif

#if 0
		const Entity e1 = ECSManager::Instance().CreateEntity("box 1");
		e1->GetComponent<Transform>().m_Position = { 0,5,0 };
		e1->AddComponent<BoxCollider>();
		ConstructBoxCollider(e1, { 7, 7, 7 });
		ColliderToTrigger(e1);
#endif

#if 0
		const Entity e2 = ECSManager::Instance().CreateEntity("ball 1");
		e2->GetComponent<Transform>().m_Position = { 0, 15, 0 };
		e2->AddComponent<SphereCollider>();
		e2->AddComponent<Rigidbody>();
		// ColliderToTrigger(e2);			TriggerToCollider(e2);
#endif

		return isReadyForUpdate = true;
	}

	void PhysicsSystem::Update()
	{
		//if (!isReadyForUpdate) TESTUpdate();
		// makes a non-void function only run once
		// without any if branches, using short-circuiting! :D
		isReadyForUpdate || TESTUpdate();

		// How do I tell if a component has been removed from an entity???
		for (auto& x : m_Actors)
		{
			// NEVER ERASE ELEMENTS WHILE ITERATING THROUGH THEM.
			// THE ITERATOR WILL ++ AND READ INVALID DATA!
			// BEGIN & END WILL BECOME THE SAME AFTER THE ERASE (maybe)
			// BUT THE LOOP WILL STILL ITERATE BECAUSE THE END OF THE MAP CHANGED!! (probably)
			if (m_Actors.empty())
			{
				break;
				// int x = 1; (void)x;
				// THIS WOULD'VE WORKED IN VS2019!!!
				// THANKS YOU VS 2022 (: (: (:
			}
			Entity entity = ECSManager::Instance().FindEntity(x.first);
			if (!entity) continue; // I sure hope this doesn't happen!

			// if the attached comps say yes, but the entity says no...
			// there is a mismatch. Thus, destroy that component.
			const auto& attachedComponents = x.second.m_AttachedComponents;

			OnEntityMismatchDestroyComponent(entity, attachedComponents, Rigidbody);
			OnEntityMismatchDestroyComponent(entity, attachedComponents, SphereCollider);
			OnEntityMismatchDestroyComponent(entity, attachedComponents, BoxCollider);
			OnEntityMismatchDestroyComponent(entity, attachedComponents, CapsuleCollider);
		}

		UpdateAllEntitiesWithComponent(Rigidbody);
		UpdateAllEntitiesWithComponent(SphereCollider);
		UpdateAllEntitiesWithComponent(BoxCollider);
		UpdateAllEntitiesWithComponent(CapsuleCollider);

#if 0
		static std::time_t start_timer = std::time(nullptr);
		const long long result = std::time(nullptr) - start_timer;

		if (result >= 5)
		{
			// do a test thingy here
			auto e1 = ECSManager::Instance().GetEntities<SphereCollider>().front();

			AddForce(e1, { -100, 0, 0 });
			printf("====================================================\n");

			// reset timer
			std::time(&start_timer);
		}
#endif

		// Accumulator?
		// https://nvidia-omniverse.github.io/PhysX/physx/5.1.3/docs/Simulation.html#the-simulation-loop
		m_Scene->simulate(1.0f / 60.0f);
		m_Scene->fetchResults(true);

		for (const auto& pair : m_Actors)
		{
			const Entity entity = ECSManager::Instance().FindEntity(pair.first);
			if (!entity) continue; // HERE WE ARE FOR SOME REASON??
			const SharedData& sharedData = pair.second;

			const PxVec3 pos = sharedData.m_RigidDynamic->getGlobalPose().p;
			entity->GetComponent<Transform>().m_Position = VEC3_CAST(glm::vec3, pos);

			const PxQuat rotQuat = sharedData.m_RigidDynamic->getGlobalPose().q;
			const glm::vec3 eulerAnglesInRad = glm::eulerAngles(glm::quat{ rotQuat.w, rotQuat.x, rotQuat.y, rotQuat.z });
			entity->GetComponent<Transform>().m_Rotation = eulerAnglesInRad / PI * 180.0f;

			entity->GetComponent<Transform>().m_IsDirty = true;
#if 0
			printf("%s has\n", entity->GetComponent<Properties>().m_Name.c_str());
			const auto& pos = entity->GetComponent<Transform>().m_Position;
			const auto& rot = entity->GetComponent<Transform>().m_Rotation;
			printf("pos: %f %f %f\n", pos.x, pos.y, pos.z);
			printf("rot: %f %f %f\n\n", rot.x, rot.y, rot.z);
#endif
		}
	}

	void PhysicsSystem::OnReset()
	{
		m_Actors.clear(); // ???
	}

	void PhysicsSystem::OnDestroyEntities()
	{
#if 0
		TRE_CORE_WARN("AM I EVEN HERE?!?!?!?!?");

		// hopefully this'll be the parameter of this function in the future!!
		Entity entity = ECSManager::Instance().CreateEntity("PhysicsSystem::OnDestroyGO");

		if (entity->HasComponent<Rigidbody>())		DestructRigidbody(entity);
		if (entity->HasComponent<SphereCollider>())	DestructSphereCollider(entity);
		if (entity->HasComponent<BoxCollider>())	DestructBoxCollider(entity);

		m_Actors.erase(entity->GetGUID());
		ECSManager::Instance().DestroyEntity(entity);
#endif
	}

	void PhysicsSystem::Shutdown()
	{
		TRE_CORE_INFO("Physics System Shutdown");
		// HOW THE HECK DID THIS MAGICALLY WORK ?!?
		// WAIT I FOUND OUT.
		// NEVER CLOSE THE PVD BEFORE THE APPLICATION AAAAAAAAAAAAA

		m_Actors.clear();
		PX_RELEASE(m_GroundPlane);
		PX_RELEASE(m_DefaultMaterial);
		PX_RELEASE(m_Scene);
		PxCloseExtensions();
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

	//This function creates a stack of shapes
	void PhysicsSystem::CreateStack(const PxTransform& t, unsigned size, float halfExtent) const
	{
		for (unsigned i = 0; i < size; i++)
		{
			for (unsigned j = 0; j < size - i; j++)
			{
				Entity entity = ECSManager::Instance().CreateEntity();
				const PxVec3 stackPos{ (2.0f * j) - (size - i) , 2.0f * i + 1 , 0 };
				const PxVec3 newPos = t.transform(halfExtent * stackPos);
				entity->GetComponent<Transform>().m_Position = VEC3_CAST(glm::vec3, newPos);
				entity->AddComponent<Rigidbody>();		ConstructRigidbody(entity);
				entity->AddComponent<SphereCollider>();	ConstructSphereCollider(entity, halfExtent);
				entity->AddComponent<BoxCollider>();	ConstructBoxCollider(entity, glm::vec3{ halfExtent });
			}
		}
	}

	// if one shape on an entity is a trigger, they're all triggers now :)
	void PhysicsSystem::ColliderToTrigger(const Entity& entity) const
	{
		assert(m_Actors.contains(entity->GetGUID()));

		// assert that there is at least 1 collider component on this entity
		assert(m_Actors[entity->GetGUID()].m_AttachedComponents >> 1);

		PxRigidDynamic*& rigidDynamic = m_Actors[entity->GetGUID()].m_RigidDynamic;

		// because Rigidbody is represented by the 1st bit in m_AttachedComponents
		const bool hasRigidbody = m_Actors[entity->GetGUID()].m_AttachedComponents & PhysicsComponentTypes::Rigidbody;

		// if there's a rigidbody attached, enable gravity
		rigidDynamic->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, !hasRigidbody);

		unsigned nbShapes = rigidDynamic->getNbShapes();
		const std::unique_ptr<PxShape* []> shapes(new PxShape * [nbShapes]); // I hate that I have to do this...
		nbShapes = rigidDynamic->getShapes(shapes.get(), nbShapes);

		for (unsigned i = 0; i < nbShapes; ++i)
		{
			PxShapeFlags shapeFlags = shapes[i]->getFlags();
			shapeFlags.clear(PxShapeFlag::eSIMULATION_SHAPE);
			shapeFlags.raise(PxShapeFlag::eTRIGGER_SHAPE);
			shapes[i]->setFlags(shapeFlags);
		}

		auto MarkAsTrigger = []<typename Collider>(const Entity & e)
		{
			if (e->HasComponent<Collider>())
				e->GetComponent<Collider>().m_IsTrigger = true;
		};

		// because I can't do MarkAsTrigger<Collider>() with lambdas...
		MarkAsTrigger.operator() < SphereCollider > (entity);
		MarkAsTrigger.operator() < BoxCollider > (entity);
		MarkAsTrigger.operator() < CapsuleCollider > (entity);
	}

	// if one shape on an entity is a collider, they're all colliders now :)
	void PhysicsSystem::TriggerToCollider(const Entity& entity) const
	{
		assert(m_Actors.contains(entity->GetGUID()));

		// assert that there is at least 1 collider component on this entity
		assert(m_Actors[entity->GetGUID()].m_AttachedComponents >> 1);

		PxRigidDynamic*& rigidDynamic = m_Actors[entity->GetGUID()].m_RigidDynamic;

		// because Rigidbody is represented by the 1st bit in m_AttachedComponents
		const bool hasRigidbody = m_Actors[entity->GetGUID()].m_AttachedComponents & PhysicsComponentTypes::Rigidbody;

		// if there's a rigidbody attached, enable gravity
		rigidDynamic->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, !hasRigidbody);

		unsigned nbShapes = rigidDynamic->getNbShapes();
		const std::unique_ptr<PxShape* []> shapes(new PxShape * [nbShapes]); // I hate that I have to do this...
		nbShapes = rigidDynamic->getShapes(shapes.get(), nbShapes);

		for (unsigned i = 0; i < nbShapes; ++i)
		{
			PxShapeFlags shapeFlags = shapes[i]->getFlags();
			shapeFlags.clear(PxShapeFlag::eTRIGGER_SHAPE);
			shapeFlags.raise(PxShapeFlag::eSIMULATION_SHAPE);
			shapes[i]->setFlags(shapeFlags);
		}

		auto MarkAsCollider = []<typename Collider>(const Entity & e)
		{
			if (e->HasComponent<Collider>())
				e->GetComponent<Collider>().m_IsTrigger = false;
		};

		// because I can't do MarkAsCollider<Collider>() with lambdas...
		MarkAsCollider.operator() < SphereCollider > (entity);
		MarkAsCollider.operator() < BoxCollider > (entity);
		MarkAsCollider.operator() < CapsuleCollider > (entity);
	}

	void SimulationEventCallback::onAdvance(const PxRigidBody* const* bodyBuffer, const PxTransform* poseBuffer, const PxU32 count)
	{
		UNUSED_PARAM(bodyBuffer);
		UNUSED_PARAM(poseBuffer);
		UNUSED_PARAM(count);
		printf("|%s|\n", __FUNCTION__);
	}

	void SimulationEventCallback::onConstraintBreak(PxConstraintInfo* constraints, PxU32 count)
	{
		UNUSED_PARAM(constraints);
		UNUSED_PARAM(count);
		printf("|%s|\n", __FUNCTION__);
	}

	void SimulationEventCallback::onContact(const PxContactPairHeader& pairHeader, const PxContactPair* pairs, PxU32 nbPairs)
	{
		UNUSED_PARAM(pairHeader);
		UNUSED_PARAM(pairs);
		UNUSED_PARAM(nbPairs);
		printf("|%s|\n", __FUNCTION__);

		for (unsigned i = 0; i < nbPairs; ++i)
		{
			printf("the contact are |%s| and |%s|\n", pairHeader.actors[0]->getName(), pairHeader.actors[1]->getName());

			if (pairs->flags & PxContactPairFlag::eACTOR_PAIR_HAS_FIRST_TOUCH)
				printf("%sfirst kiss%s\n", Text::red.c_str(), Text::reset.c_str());
			if (pairs->flags & PxContactPairFlag::eACTOR_PAIR_LOST_TOUCH)
				printf("%sbreakup%s\n", Text::blue.c_str(), Text::reset.c_str());
			if (!(pairs->flags & (PxContactPairFlag::eACTOR_PAIR_HAS_FIRST_TOUCH | PxContactPairFlag::eACTOR_PAIR_LOST_TOUCH)))
				printf("%ssustained??%s\n", Text::green.c_str(), Text::reset.c_str());
		}
		printf("\n");
	}

	void SimulationEventCallback::onSleep(PxActor** actors, PxU32 count)
	{
		UNUSED_PARAM(actors);
		UNUSED_PARAM(count);
		printf("|%s|\n", __FUNCTION__);
		printf("oh, %s is going to sleep...\n", actors[0]->getName());
	}

	void SimulationEventCallback::onTrigger(PxTriggerPair* pairs, PxU32 count)
	{
		UNUSED_PARAM(pairs);
		UNUSED_PARAM(count);
		printf("|%s|\n", __FUNCTION__);

		if (!count) return;
		// auto& pair = pairs[count - 1];
		//printf("YOOOOOOOOOOOOOOOOOOOOOOO\n");
		for (unsigned i = 0; i < count; ++i)
		{
			printf("the trigger are |%s| & |%s|\n", pairs->triggerActor->getName(), pairs->otherActor->getName());
			const auto& yeah = pairs[i];
			if (yeah.status & PxPairFlag::eNOTIFY_TOUCH_FOUND)
				printf("Shape is entering trigger volume\n");
			if (yeah.status & PxPairFlag::eNOTIFY_TOUCH_LOST)
				printf("Shape is leaving trigger volume\n");
			// on trigger stay needs to use the results of eNOTIFY_TOUCH_FOUND and eNOTIFY_TOUCH_LOST ...
		}
		printf("\n");
	}

	void SimulationEventCallback::onWake(PxActor** actors, PxU32 count)
	{
		UNUSED_PARAM(actors);
		UNUSED_PARAM(count);
		printf("|%s|\n", __FUNCTION__);
		printf("oh, %s woke up!\n", actors[0]->getName());
	}
}


// collision layering!! -> PxSetGroupCollisionFlag()

// DISCO RGB FONT FOR EDITOR COMPONENTS?????

// none			- just don't have anything, please
// collider		- make shape with 
// rigidbody	- 
// both			- create stuff like prior

// WHAT IF A THING DIDN'T HAVE TO HAVE A SHAPE ATTACHED???
// or what if I shape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, false); ?
// or maybe rb.m_RigidDynamic->setActorFlags(PxActorFlag::eDISABLE_GRAVITY);

// WAIT I NEED THE RIGIDACTOR AN ATTACHED COMPONENTS TO BE SHARED **AT ALL TIMES**
