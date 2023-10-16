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

#define VEC3_CAST(type, vec) (##type{(vec).x, (vec).y, (vec).z})
#define UNUSED_PARAM(param) (void)param
#define PI 3.14159265358979323846f

#pragma region PhysicsComponentAssertions
#define PhysicsComponentConstructorAssertion(Type)														\
	if (!entity->HasComponent<Type>())																	\
	{																									\
		TRE_CORE_ERROR("[" __FUNCTION__ "] "															\
			"Entity \"" + entity->GetName() + "\" has no "+  #Type + " to construct.");					\
		assert(entity->HasComponent<Type>());															\
	}

// this should never have to trip, but you never know...
#define PhysicsComponentDestructorAssertion(Type)														\
	if (!entity->HasComponent<Type>())																	\
	{																									\
		TRE_CORE_ERROR("[" __FUNCTION__ "] "															\
			"Entity \"" + entity->GetName() + "\" has no "+  #Type + " to destroy.");					\
		assert(entity->HasComponent<Type>());															\
	}

#define PhysicsComponentAssertion(Type) 																\
	if (!entity->HasComponent<Type>())																	\
	{																									\
		TRE_CORE_ERROR("[" __FUNCTION__ "] "															\
			"Entity \"" + entity->GetName() + "\" has no "+  #Type + " to perform this operation.");	\
		assert(entity->HasComponent<Type>());															\
	}
#pragma endregion

// not the best name... :/
#define UpdateAllEntitiesWithComponent(Type)\
	for (const Entity & entity : ECSManager::Instance().GetEntities<Type>()) Update##Type(entity)

// this name is even worse! :_(
#define OnEntityMismatchDestroyComponent(entity, attachedComponents, Type)\
	if ((attachedComponents) & PhysicsComponentTypes::##Type && !(entity)->HasComponent<Type>()) Destruct##Type(entity)

#pragma endregion

using namespace physx;
// to save my dwindling sanity

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
		m_Foundation->setReportAllocationNames(false);

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

		//A thread that will do collision management
		sceneDesc.filterShader = PxDefaultSimulationFilterShader;

		// SimulationEventCallback must inherit PxSimulationEventCallback
		// PUBLICLY in order to work, otherwise...
		// C2243: 'type cast': conversion from 'TRE::SimulationEventCallback *'
		// to 'physx::PxSimulationEventCallback *' exists, but is inaccessible
		sceneDesc.simulationEventCallback = &m_SimulationEventCallback;

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
		m_Scene->addActor(*m_GroundPlane);

		TRE_CORE_INFO("Physics/PhysX systems initialization complete! :D");
	}

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
		e1->GetComponent<Transform>().m_Position = { 0, 10, 0 };
		e1->AddComponent<BoxCollider>();
		e1->AddComponent<Rigidbody>();
#endif

#if 0
		const Entity e2 = ECSManager::Instance().CreateEntity("ball 1");
		e2->GetComponent<Transform>().m_Position = { 1,2,0 };
		e2->AddComponent<SphereCollider>();
		// e2->AddComponent<Rigidbody>();
		// ColliderToTrigger(e2);			TriggerToCollider(e2);
#endif

		return m_IsReadyForUpdate = true;
	}

	void PhysicsSystem::Update()
	{
		//if (!m_IsReadyForUpdate) TESTUpdate();
		// makes a non-void function only run once
		// without any if branches, using short-circuiting! :D
		m_IsReadyForUpdate || TESTUpdate();

#if 0
		static std::time_t start_timer = std::time(nullptr);
		const long long result = std::time(nullptr) - start_timer;
		if (result >= 1)
		{
			// do a test thingy here

			// reset timer
			std::time(&start_timer);
		}
#endif

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

			// TODO: Remove this, use PhysicsComponent destructors I BEG OF YOU
			OnEntityMismatchDestroyComponent(entity, attachedComponents, Rigidbody);
			OnEntityMismatchDestroyComponent(entity, attachedComponents, SphereCollider);
			OnEntityMismatchDestroyComponent(entity, attachedComponents, BoxCollider);
		}

		UpdateAllEntitiesWithComponent(Rigidbody);
		UpdateAllEntitiesWithComponent(SphereCollider);
		UpdateAllEntitiesWithComponent(BoxCollider);

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

	void PhysicsSystem::OnDestroyGO()
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

	bool PhysicsSystem::ConstructSphereCollider(const Entity& entity, const float radius, const glm::vec3& offset) const
	{
		PhysicsComponentConstructorAssertion(SphereCollider);

		// if there are no existing physics components on the entity
		if (!m_Actors.contains(entity->GetGUID()))
		{
			SharedData tempSharedData;

			const glm::vec3 pos = entity->GetComponent<Transform>().m_Position + offset;
			const PxVec3 colliderPos = VEC3_CAST(PxVec3, pos);

			const glm::vec3 eulerAnglesInRad = entity->GetComponent<Transform>().m_Rotation * PI / 180.0f;
			const glm::quat rotQuat{ eulerAnglesInRad };

			const PxTransform transform(colliderPos, PxQuat{ rotQuat.x, rotQuat.y, rotQuat.z, rotQuat.w });

			tempSharedData.m_RigidDynamic = m_Physics->createRigidDynamic(transform);
			m_Scene->addActor(*tempSharedData.m_RigidDynamic);

			tempSharedData.m_GUID = entity->GetGUID();

			m_Actors[entity->GetGUID()] = tempSharedData;
		}

		SharedData& sharedData = m_Actors[entity->GetGUID()];

		PxRigidActorExt::createExclusiveShape(*sharedData.m_RigidDynamic, PxSphereGeometry(radius), *m_DefaultMaterial);

		// if no rigidbody, turn the gravity off so that these colliders won't 'fall'
		if (!(sharedData.m_AttachedComponents & PhysicsComponentTypes::Rigidbody))
		{
			sharedData.m_RigidDynamic->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);

			// so that colliders without rigidbodies will stay put when hit
			sharedData.m_RigidDynamic->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, true);
		}
		else
		{
			// if there is a rigidbody, we gotta recalculate stuff because we just added a shape (?)
			// WAIT YES THAT'S ACTUALLY IT YATTA!!!
			PxRigidBodyExt::updateMassAndInertia(*sharedData.m_RigidDynamic, 1.0f);
		}

		sharedData.m_AttachedComponents |= PhysicsComponentTypes::SphereCollider;
		// assert(entity->GetGUID() == sharedData.m_GUID);

		SphereCollider& sphereCollider = entity->GetComponent<SphereCollider>();
		// TODO: assign more data here
		// sphereCollider.m_IsTrigger = ...
		sphereCollider.m_Offset = offset;
		sphereCollider.m_Radius = radius;

		return sphereCollider.m_IsInitialized = true;
	}

	void PhysicsSystem::ResizeSphereCollider(const Entity& entity, const float newRadius) const
	{
		PhysicsComponentAssertion(SphereCollider);

		PxRigidDynamic*& rigidDynamic = m_Actors[entity->GetGUID()].m_RigidDynamic;

		unsigned nbShapes = rigidDynamic->getNbShapes();
		const std::unique_ptr<PxShape* []> shapes(new PxShape * [nbShapes]); // I hate that I have to do this...
		nbShapes = rigidDynamic->getShapes(shapes.get(), nbShapes);

		for (unsigned i = 0; i < nbShapes; ++i)
		{
			if (shapes[i]->getGeometryType() != PxGeometryType::eSPHERE) continue;

			shapes[i]->setGeometry(PxSphereGeometry(fabs(newRadius))); break;
		}
	}

	void PhysicsSystem::UpdateSphereCollider(const Entity& entity) const
	{
		PhysicsComponentAssertion(SphereCollider);

		SphereCollider& sphereCollider = entity->GetComponent<SphereCollider>();

		sphereCollider.m_IsInitialized || ConstructSphereCollider(entity);

		PxRigidDynamic*& rigidDynamic = m_Actors[entity->GetGUID()].m_RigidDynamic;

		// sphereCollider.m_IsTrigger = rigidDynamic->getSomeFlags().isSet(/*whatever the heck is used for triggers*/)

		sphereCollider.m_Offset = VEC3_CAST(glm::vec3, rigidDynamic->getGlobalPose().p) - entity->GetComponent<Transform>().m_Position;

		unsigned nbShapes = rigidDynamic->getNbShapes();
		const std::unique_ptr<PxShape* []> shapes(new PxShape * [nbShapes]); // I hate that I have to do this...
		nbShapes = rigidDynamic->getShapes(shapes.get(), nbShapes);

		// obtain the index of the sphere shape
		unsigned i = 0;
		for (; i < nbShapes; ++i)
		{
			if (shapes[i]->getGeometryType() != PxGeometryType::eSPHERE) continue;

			break;
		}

		PxSphereGeometry sphereGeometry;
		shapes[i]->getSphereGeometry(sphereGeometry);
		sphereCollider.m_Radius = sphereGeometry.radius;
	}

	void PhysicsSystem::DestructSphereCollider(const Entity& entity) const
	{
		// PhysicsComponentDestructorAssertion(SphereCollider);

		SharedData& sharedData = m_Actors[entity->GetGUID()];
		// PxRigidDynamic*& rigidDynamic = sharedData.m_RigidDynamic;

		// reset bit for this component
		sharedData.m_AttachedComponents &= ~PhysicsComponentTypes::SphereCollider;

		if (!sharedData.m_AttachedComponents)
		{
			m_Scene->removeActor(*sharedData.m_RigidDynamic);
			sharedData.m_RigidDynamic->release();
			m_Actors.erase(entity->GetGUID());
		}
		else // there's still more attached physics components
		{
			unsigned nbShapes = sharedData.m_RigidDynamic->getNbShapes();
			const std::unique_ptr<PxShape* []> shapes(new PxShape * [nbShapes]); // I hate that I have to do this...
			nbShapes = sharedData.m_RigidDynamic->getShapes(shapes.get(), nbShapes);

			for (unsigned i = 0; i < nbShapes; ++i)
			{
				if (shapes[i]->getGeometryType() != PxGeometryType::eSPHERE) continue;

				// there should only be ONE of each physics component, so it's safe to stop looping here
				sharedData.m_RigidDynamic->detachShape(*shapes[i]); break;
			}

			PxRigidBodyExt::updateMassAndInertia(*sharedData.m_RigidDynamic, 1.0);
		}

		// entity->RemoveComponent<SphereCollider>();
	}

	bool PhysicsSystem::ConstructBoxCollider(const Entity& entity, const glm::vec3& halfExtents, const glm::vec3& offset) const
	{
		PhysicsComponentConstructorAssertion(BoxCollider);

		// if there are no existing physics components on the entity
		if (!m_Actors.contains(entity->GetGUID()))
		{
			SharedData tempSharedData;

			const glm::vec3 pos = entity->GetComponent<Transform>().m_Position + offset;
			const PxVec3 colliderPos = VEC3_CAST(PxVec3, pos);

			const glm::vec3 eulerAnglesInRad = entity->GetComponent<Transform>().m_Rotation * PI / 180.0f;
			const glm::quat rotQuat{ eulerAnglesInRad };

			const PxTransform transform(colliderPos, PxQuat{ rotQuat.x, rotQuat.y, rotQuat.z, rotQuat.w });

			tempSharedData.m_RigidDynamic = m_Physics->createRigidDynamic(transform);
			m_Scene->addActor(*tempSharedData.m_RigidDynamic);

			tempSharedData.m_GUID = entity->GetGUID();

			m_Actors[entity->GetGUID()] = tempSharedData;
		}

		SharedData& sharedData = m_Actors[entity->GetGUID()];

		PxRigidActorExt::createExclusiveShape(*sharedData.m_RigidDynamic, PxBoxGeometry(VEC3_CAST(PxVec3, halfExtents)), *m_DefaultMaterial);
		// (above))->setFlag(PxShapeFlag::eSIMULATION_SHAPE, false);

		// if no rigidbody, turn the gravity off so that these colliders won't 'fall'
		if (!(sharedData.m_AttachedComponents & PhysicsComponentTypes::Rigidbody))
		{
			sharedData.m_RigidDynamic->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);

			// so that colliders without rigidbodies will stay put when hit
			sharedData.m_RigidDynamic->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, true);
		}
		else
		{
			// if there is a rigidbody, we gotta recalculate stuff because we just added a shape (?)
			// WAIT YES THAT'S ACTUALLY IT YATTA!!!
			PxRigidBodyExt::updateMassAndInertia(*sharedData.m_RigidDynamic, 1.0f);
		}

		sharedData.m_AttachedComponents |= PhysicsComponentTypes::BoxCollider;
		// assert(entity->GetGUID() == sharedData.m_GUID);

		BoxCollider& boxCollider = entity->GetComponent<BoxCollider>();
		// TODO: assign more data here
		// boxCollider.m_IsTrigger = ...
		boxCollider.m_Offset = offset;
		boxCollider.m_HalfExtents = halfExtents;

		return boxCollider.m_IsInitialized = true;
	}

	void PhysicsSystem::ResizeBoxCollider(const Entity& entity, const glm::vec3& newHalfExtents) const
	{
		PhysicsComponentAssertion(BoxCollider);

		PxRigidDynamic*& rigidDynamic = m_Actors[entity->GetGUID()].m_RigidDynamic;

		unsigned nbShapes = rigidDynamic->getNbShapes();
		const std::unique_ptr<PxShape* []> shapes(new PxShape * [nbShapes]); // I hate that I have to do this...
		nbShapes = rigidDynamic->getShapes(shapes.get(), nbShapes);

		for (unsigned i = 0; i < nbShapes; ++i)
		{
			if (shapes[i]->getGeometryType() != PxGeometryType::eBOX) continue;

			shapes[i]->setGeometry(PxBoxGeometry(
				fabs(newHalfExtents.x),
				fabs(newHalfExtents.y),
				fabs(newHalfExtents.z)));
			break;
		}
	}

	void PhysicsSystem::UpdateBoxCollider(const Entity& entity) const
	{
		PhysicsComponentAssertion(BoxCollider);

		BoxCollider& boxCollider = entity->GetComponent<BoxCollider>();

		boxCollider.m_IsInitialized || ConstructBoxCollider(entity);

		PxRigidDynamic*& rigidDynamic = m_Actors[entity->GetGUID()].m_RigidDynamic;

		// boxCollider.m_IsTrigger = rigidDynamic->getSomeFlags().isSet(/*whatever the heck is used for triggers*/)

		boxCollider.m_Offset = VEC3_CAST(glm::vec3, rigidDynamic->getGlobalPose().p) - entity->GetComponent<Transform>().m_Position;

		unsigned nbShapes = rigidDynamic->getNbShapes();
		const std::unique_ptr<PxShape* []> shapes(new PxShape * [nbShapes]); // I hate that I have to do this...
		nbShapes = rigidDynamic->getShapes(shapes.get(), nbShapes);

		// obtain the index of the box shape
		unsigned i = 0;
		for (; i < nbShapes; ++i)
		{
			if (shapes[i]->getGeometryType() != PxGeometryType::eBOX) continue;

			break;
		}

		PxBoxGeometry boxGeometry;
		shapes[i]->getBoxGeometry(boxGeometry);
		boxCollider.m_HalfExtents = VEC3_CAST(glm::vec3, boxGeometry.halfExtents);
	}

	void PhysicsSystem::DestructBoxCollider(const Entity& entity) const
	{
		// PhysicsComponentDestructorAssertion(BoxCollider);

		SharedData& sharedData = m_Actors[entity->GetGUID()];
		// PxRigidDynamic* rigidDynamic = sharedComponent.m_RigidDynamic;

		// reset bit for this component
		sharedData.m_AttachedComponents &= ~PhysicsComponentTypes::BoxCollider;

		if (!sharedData.m_AttachedComponents)
		{
			m_Scene->removeActor(*sharedData.m_RigidDynamic);
			sharedData.m_RigidDynamic->release();
			m_Actors.erase(entity->GetGUID());
		}
		else // there's still more attached physics components
		{
			unsigned nbShapes = sharedData.m_RigidDynamic->getNbShapes();
			const std::unique_ptr<PxShape* []> shapes(new PxShape * [nbShapes]); // I hate that I have to do this...
			nbShapes = sharedData.m_RigidDynamic->getShapes(shapes.get(), nbShapes);

			for (unsigned i = 0; i < nbShapes; ++i)
			{
				if (shapes[i]->getGeometryType() != PxGeometryType::eBOX) continue;

				// there should only be ONE of each physics component, so it's safe to stop looping here
				sharedData.m_RigidDynamic->detachShape(*shapes[i]); break;
			}

			PxRigidBodyExt::updateMassAndInertia(*sharedData.m_RigidDynamic, 1.0);
		}

		// entity->RemoveComponent<BoxCollider>();
	}

	bool PhysicsSystem::ConstructRigidbody(const Entity& entity) const
	{
		PhysicsComponentConstructorAssertion(Rigidbody);

		// if there are no existing physics components on the entity
		if (!m_Actors.contains(entity->GetGUID()))
		{
			SharedData tempSharedData;
			const glm::vec3 pos = entity->GetComponent<Transform>().m_Position;
			const PxVec3 rigidbodyPos = VEC3_CAST(PxVec3, pos);

			const glm::vec3 eulerAnglesInRad = entity->GetComponent<Transform>().m_Rotation * PI / 180.0f;
			const glm::quat rotQuat{ eulerAnglesInRad };

			const PxTransform transform(rigidbodyPos, PxQuat{ rotQuat.x, rotQuat.y, rotQuat.z, rotQuat.w });

			tempSharedData.m_RigidDynamic = m_Physics->createRigidDynamic(transform);
			m_Scene->addActor(*tempSharedData.m_RigidDynamic);

			tempSharedData.m_GUID = entity->GetGUID();

			m_Actors[entity->GetGUID()] = tempSharedData;
		}

		SharedData& sharedData = m_Actors[entity->GetGUID()];
		PxRigidBodyExt::updateMassAndInertia(*sharedData.m_RigidDynamic, 1.0f);

		// activate gravity by default
		bool useGravity = true; // TODO: disabling gravity
		sharedData.m_RigidDynamic->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, !useGravity);

		// this is necessary to allow the actor to freakin move by physics and forces and such
		sharedData.m_RigidDynamic->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, false);
		
		// wake up the sleeping beauty
		sharedData.m_RigidDynamic->wakeUp();
		// UNBELIEVABLE, THIS IS WHAT I WAS MISSING AFTER ALL THIS TIME.
		// AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA

		// also, very lucky that I called this AFTER setting kinematic to false,
		// wouldn't work otherwise!

		// no kinematic rigidbodies for now pls thanks
		// sharedData.m_RigidDynamic->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, false);

		sharedData.m_AttachedComponents |= PhysicsComponentTypes::Rigidbody;
		// assert(entity->GetGUID() == sharedData.m_GUID);

		Rigidbody& rigidbody = entity->GetComponent<Rigidbody>();
		return rigidbody.m_IsInitialized = true;
	}

	void PhysicsSystem::AddForce(const Entity& entity, glm::vec3 force) const
	{
		PhysicsComponentAssertion(Rigidbody);
		m_Actors[entity->GetGUID()].m_RigidDynamic->addForce(VEC3_CAST(PxVec3, force));
	}

	void PhysicsSystem::UpdateRigidbody(const Entity& entity) const
	{
		PhysicsComponentAssertion(Rigidbody);

		Rigidbody& rigidbody = entity->GetComponent<Rigidbody>();

		rigidbody.m_IsInitialized || ConstructRigidbody(entity);

		PxRigidDynamic*& rigidDynamic = m_Actors[entity->GetGUID()].m_RigidDynamic;

		rigidbody.m_Mass = rigidDynamic->getMass();
		// rigidbody.m_Drag = ;
		// rigidbody.m_AngularDrag = ;
		rigidbody.m_UseGravity = !rigidDynamic->getActorFlags().isSet(PxActorFlag::eDISABLE_GRAVITY);
		rigidbody.m_IsKinematic = rigidDynamic->getRigidBodyFlags().isSet(PxRigidBodyFlag::eKINEMATIC);
	}

	void PhysicsSystem::DestructRigidbody(const Entity& entity) const
	{
		// PhysicsComponentDestructorAssertion(Rigidbody);

		SharedData& sharedData = m_Actors[entity->GetGUID()];
		sharedData.m_AttachedComponents &= ~PhysicsComponentTypes::Rigidbody;

		// if colliders are *still* present on this entity
		if (sharedData.m_AttachedComponents)
		{
			// turn the gravity off so that these colliders won't 'fall'
			sharedData.m_RigidDynamic->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);
		}
		else
		{
			// get rid of all things related to the actor
			m_Scene->removeActor(*sharedData.m_RigidDynamic);
			sharedData.m_RigidDynamic->release();
			m_Actors.erase(entity->GetGUID());
		}

		// entity->RemoveComponent<Rigidbody>();
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

	void PhysicsSystem::RigidbodyConstraintsStuff(const Entity& entity) const
	{
		assert(m_Actors.contains(entity->GetGUID()));
		PxRigidDynamic*& rigidDynamic = m_Actors[entity->GetGUID()].m_RigidDynamic;

		auto x = rigidDynamic->getRigidDynamicLockFlags();
		x.raise(PxRigidDynamicLockFlag::eLOCK_ANGULAR_X);
		rigidDynamic->setRigidDynamicLockFlags(x);
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
	}

	void SimulationEventCallback::onSleep(PxActor** actors, PxU32 count)
	{
		UNUSED_PARAM(actors);
		UNUSED_PARAM(count);
		printf("|%s|\n", __FUNCTION__);
	}

	void SimulationEventCallback::onTrigger(PxTriggerPair* pairs, PxU32 count)
	{
		UNUSED_PARAM(pairs);
		UNUSED_PARAM(count);
		printf("|%s|\n", __FUNCTION__);

		if (!count) return;
		// auto& pair = pairs[count - 1];
		printf("YOOOOOOOOOOOOOOOOOOOOOOO\n");
	}

	void SimulationEventCallback::onWake(PxActor** actors, PxU32 count)
	{
		UNUSED_PARAM(actors);
		UNUSED_PARAM(count);
		printf("|%s|\n", __FUNCTION__);
	}
}
