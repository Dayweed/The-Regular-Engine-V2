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

#define VEC3_CAST(type, vec) (##type{(vec).x, (vec).y, (vec).z})

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

using namespace physx;
// to save my dwindling sanity

namespace TRE
{
	// thank you https://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles#Source_code_2
	PxVec3 QuatToEulerAngles(const PxQuat& q)
	{
		PxVec3 angles;

		// roll (x-axis rotation)
		float sinr_cosp = 2 * (q.w * q.x + q.y * q.z);
		float cosr_cosp = 1 - 2 * (q.x * q.x + q.y * q.y);
		angles.x = std::atan2(sinr_cosp, cosr_cosp);

		// pitch (y-axis rotation)
		float sinp = std::sqrt(1 + 2 * (q.w * q.y - q.x * q.z));
		float cosp = std::sqrt(1 - 2 * (q.w * q.y - q.x * q.z));
		angles.y = 2 * std::atan2(sinp, cosp) - 3.141592653f / 2;

		// yaw (z-axis rotation)
		float siny_cosp = 2 * (q.w * q.z + q.x * q.y);
		float cosy_cosp = 1 - 2 * (q.y * q.y + q.z * q.z);
		angles.z = std::atan2(siny_cosp, cosy_cosp);

		return angles;
	}

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
		e1->AddComponent<BoxCollider>();	ConstructBoxCollider(e1);
		e1->AddComponent<Rigidbody>();		ConstructRigidBody(e1);
#endif

#if 0
		const Entity e2 = ECSManager::Instance().CreateEntity("ball 1");
		e2->GetComponent<Transform>().m_Position = { 1,5,0 };
		e2->AddComponent<SphereCollider>();	ConstructSphereCollider(e2);
		e2->AddComponent<Rigidbody>();		ConstructRigidBody(e2);
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

			//const auto& vec = ECSManager::Instance().GetEntities<BoxCollider>();
			//auto& rb = m_Actors[vec.front()->GetGUID()].m_RigidDynamic;

			//std::unique_ptr<PxShape* []> arr(new PxShape * [rb->getNbShapes()]);
			//const unsigned n = rb->getShapes(arr.get(), rb->getNbShapes());
			//arr[0]->setFlag(PxShapeFlag::eSIMULATION_SHAPE, false);
			//arr[0]->setFlag(PxShapeFlag::eTRIGGER_SHAPE, true);

			// reset timer
			std::time(&start_timer);
		}
#endif

		m_Scene->simulate(1.0f / 60.0f);
		m_Scene->fetchResults(true);

		// auto UpdateTransform = []<typename Collider>{};
		// because I can't do UpdateTransform< Type >() :(
		// UpdateTransform.operator() < BoxCollider > ();
		// UpdateTransform.operator() < SphereCollider > ();
		// UpdateTransform.operator() < Rigidbody > ();

		for (const auto& pair : m_Actors)
		{
			const Entity entity = ECSManager::Instance().FindEntity(pair.first);
			const SharedData& sharedData = pair.second;
			entity->GetComponent<Transform>().m_Position = VEC3_CAST(glm::vec3, sharedData.m_RigidDynamic->getGlobalPose().p);

			// I hope this is right XO
			const PxVec3 eulerAngles = QuatToEulerAngles(sharedData.m_RigidDynamic->getGlobalPose().q);
			entity->GetComponent<Transform>().m_Rotation = VEC3_CAST(glm::vec3, eulerAngles) / 3.141592654f * 180.0f;

			printf("%s has\n", entity->GetComponent<Properties>().m_Name.c_str());
			const auto& pos = entity->GetComponent<Transform>().m_Position;
			const auto& rot = entity->GetComponent<Transform>().m_Rotation;
			printf("pos: %f %f %f\n", pos.x, pos.y, pos.z);
			printf("rot: %f %f %f\n\n", rot.x, rot.y, rot.z);
		}
	}

	void PhysicsSystem::OnDestroyGO()
	{
		// hopefully this'll be the parameter of this function in the future!!
		Entity entity = ECSManager::Instance().CreateEntity("PhysicsSystem::OnDestroyGO");

		if (entity->HasComponent<Rigidbody>())		DestructRigidBody(entity);
		if (entity->HasComponent<SphereCollider>())	DestructSphereCollider(entity);
		if (entity->HasComponent<BoxCollider>())	DestructBoxCollider(entity);

		m_Actors.erase(entity->GetGUID());
		ECSManager::Instance().DestroyEntity(entity);
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

	void PhysicsSystem::ConstructSphereCollider(const Entity& entity, const float radius, const Vector3& offset) const
	{
		PhysicsComponentConstructorAssertion(SphereCollider);

		// if there are no existing physics components on the entity
		if (!m_Actors.contains(entity->GetGUID()))
		{
			SharedData tempSharedData;

			const Vector3 objPos = entity->GetComponent<Transform>().m_Position;
			const PxVec3 colliderPos = VEC3_CAST(PxVec3, objPos) + VEC3_CAST(PxVec3, offset);

			tempSharedData.m_RigidDynamic = m_Physics->createRigidDynamic(PxTransform{ colliderPos });
			m_Scene->addActor(*tempSharedData.m_RigidDynamic);

			m_Actors[entity->GetGUID()] = tempSharedData;
		}

		SharedData& sharedData = m_Actors[entity->GetGUID()];

		PxRigidActorExt::createExclusiveShape(*sharedData.m_RigidDynamic, PxSphereGeometry(radius), *m_DefaultMaterial);

		// if no rigidbody, turn the gravity off so that these colliders won't 'fall'
		if (!(sharedData.m_AttachedComponents & PhysicsComponentTypes::Rigidbody))
		{
			sharedData.m_RigidDynamic->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);
		}
		else
		{
			// if there is a rigidbody, we gotta recalculate stuff because we just added a shape (?)
			// WAIT YES THAT'S ACTUALLY IT YATTA!!!
			PxRigidBodyExt::updateMassAndInertia(*sharedData.m_RigidDynamic, 1.0f);
		}

		sharedData.m_AttachedComponents |= PhysicsComponentTypes::SphereCollider;

		SphereCollider& sphereCollider = entity->GetComponent<SphereCollider>();
		sphereCollider.m_Radius = radius;
		// TODO: assign more data here
		// sphereCollider.m_IsTrigger = ...
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

	void PhysicsSystem::DestructSphereCollider(const Entity& entity) const
	{
		PhysicsComponentDestructorAssertion(SphereCollider);

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

		entity->RemoveComponent<SphereCollider>();
	}

	void PhysicsSystem::ConstructBoxCollider(const Entity& entity, const Vector3& halfExtents, const Vector3& offset) const
	{
		PhysicsComponentConstructorAssertion(BoxCollider);

		// if there are no existing physics components on the entity
		if (!m_Actors.contains(entity->GetGUID()))
		{
			SharedData tempSharedData;

			const Vector3 objPos = entity->GetComponent<Transform>().m_Position;
			const PxVec3 colliderPos = VEC3_CAST(PxVec3, objPos) + VEC3_CAST(PxVec3, offset);

			tempSharedData.m_RigidDynamic = m_Physics->createRigidDynamic(PxTransform{ colliderPos });
			m_Scene->addActor(*tempSharedData.m_RigidDynamic);

			m_Actors[entity->GetGUID()] = tempSharedData;
		}

		SharedData& sharedData = m_Actors[entity->GetGUID()];

		PxRigidActorExt::createExclusiveShape(*sharedData.m_RigidDynamic, PxBoxGeometry(VEC3_CAST(PxVec3, halfExtents)), *m_DefaultMaterial);
		// (above))->setFlag(PxShapeFlag::eSIMULATION_SHAPE, false);

		// if no rigidbody, turn the gravity off so that these colliders won't 'fall'
		if (!(sharedData.m_AttachedComponents & PhysicsComponentTypes::Rigidbody))
		{
			sharedData.m_RigidDynamic->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);
		}
		else
		{
			// if there is a rigidbody, we gotta recalculate stuff because we just added a shape (?)
			// WAIT YES THAT'S ACTUALLY IT YATTA!!!
			PxRigidBodyExt::updateMassAndInertia(*sharedData.m_RigidDynamic, 1.0f);
		}

		sharedData.m_AttachedComponents |= PhysicsComponentTypes::BoxCollider;

		BoxCollider& boxCollider = entity->GetComponent<BoxCollider>();
		boxCollider.m_HalfExtents = halfExtents;
		// TODO: assign more data here
		// boxCollider.m_IsTrigger = ...
	}

	void PhysicsSystem::ResizeBoxCollider(const Entity& entity, const Vector3& newHalfExtents) const
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

	void PhysicsSystem::DestructBoxCollider(const Entity& entity) const
	{
		PhysicsComponentDestructorAssertion(BoxCollider);

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

		entity->RemoveComponent<BoxCollider>();
	}

	void PhysicsSystem::ConstructRigidBody(const Entity& entity) const
	{
		PhysicsComponentConstructorAssertion(Rigidbody);

		// if there are no existing physics components on the entity
		if (!m_Actors.contains(entity->GetGUID()))
		{
			SharedData tempSharedData;
			const Vector3& pos = entity->GetComponent<Transform>().m_Position;

			tempSharedData.m_RigidDynamic = m_Physics->createRigidDynamic(PxTransform{ VEC3_CAST(PxVec3, pos) });
			m_Scene->addActor(*tempSharedData.m_RigidDynamic);

			m_Actors[entity->GetGUID()] = tempSharedData;
		}

		SharedData& sharedData = m_Actors[entity->GetGUID()];
		PxRigidBodyExt::updateMassAndInertia(*sharedData.m_RigidDynamic, 1.0f);

		// activate gravity by default
		bool useGravity = true; // TODO: disabling gravity
		sharedData.m_RigidDynamic->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, !useGravity);

		sharedData.m_AttachedComponents |= PhysicsComponentTypes::Rigidbody;
	}

	void PhysicsSystem::AddForce(const Entity& entity, Vector3 force) const
	{
		PhysicsComponentAssertion(Rigidbody);
		m_Actors[entity->GetGUID()].m_RigidDynamic->addForce(VEC3_CAST(PxVec3, force));
	}

	void PhysicsSystem::DestructRigidBody(const Entity& entity) const
	{
		PhysicsComponentDestructorAssertion(Rigidbody);

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

		entity->RemoveComponent<Rigidbody>();
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
				entity->AddComponent<Rigidbody>();		ConstructRigidBody(entity);
				entity->AddComponent<SphereCollider>();	ConstructSphereCollider(entity, halfExtent);
				entity->AddComponent<BoxCollider>();	ConstructBoxCollider(entity, Vector3{ halfExtent });
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
}
