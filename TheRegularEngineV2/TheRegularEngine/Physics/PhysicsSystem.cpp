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
			pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
			pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
			pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
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

		//const Entity e1 = ECSManager::Instance().CreateEntity("box 1");
		//e1->GetComponent<Transform>().m_Position = { 0, 10, 0 };
		//e1->AddComponent<BoxCollider>(); ConstructBoxCollider(e1);
		//e1->AddComponent<Rigidbody>(); ConstructRigidBody(e1);

		//const Entity e2 = ECSManager::Instance().CreateEntity("ball 1");
		//e2->GetComponent<Transform>().m_Position = { 1,5,0 };
		//e2->AddComponent<SphereCollider>(); ConstructSphereCollider(e2);
		//e2->AddComponent<Rigidbody>(); ConstructRigidBody(e2);

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
			auto vec = ECSManager::Instance().GetEntities<Rigidbody>();
			if (!vec.empty())
			{
				//BoxCollider collider = vec.front()->GetComponent<BoxCollider>();
				//auto* rigidbody = collider.m_RigidDynamic->is<PxRigidBody>();
				//rigidbody->addForce({ 0, 800, 0 });

				AddForce(vec.front(), { 0, 800, 0 });
			}
			std::time(&start_timer); // reset timer
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
			glm::vec3 pos = entity->GetComponent<Transform>().m_Position;
			glm::vec3 rot = entity->GetComponent<Transform>().m_Rotation;
			printf("pos: %f %f %f\n", pos.x, pos.y, pos.z);
			printf("rot: %f %f %f\n\n", rot.x, rot.y, rot.z);
		}
	}

	void PhysicsSystem::OnDestroyGO() {}

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
		if (!entity->HasComponent<SphereCollider>())
		{
			const std::string funcName{ __FUNCTION__ };
			TRE_CORE_ERROR("[" + funcName + "] Entity \"" + entity->GetName() + "\" has no SphereCollider to construct.");
			assert(entity->HasComponent<SphereCollider>());
		}

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
			PxActorFlags actorFlags = sharedData.m_RigidDynamic->getActorFlags();
			actorFlags.raise(PxActorFlag::eDISABLE_GRAVITY);
			sharedData.m_RigidDynamic->setActorFlags(actorFlags);
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

	void PhysicsSystem::DestructSphereCollider(const Entity& entity) const
	{
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
			const unsigned nbShapes = sharedData.m_RigidDynamic->getNbShapes();
			const std::shared_ptr<PxShape* []> buffer(new PxShape * [nbShapes]); // I hate that I have to do this...
			const unsigned actualNbShapes = sharedData.m_RigidDynamic->getShapes(buffer.get(), nbShapes);

			for (unsigned i = 0; i < actualNbShapes; ++i)
			{
				if (buffer[i]->getGeometryType() != PxGeometryType::eSPHERE) continue;

				// there should only be ONE of each physics component, so it's safe to stop looping here
				sharedData.m_RigidDynamic->detachShape(*buffer[i]); break;
			}

			PxRigidBodyExt::updateMassAndInertia(*sharedData.m_RigidDynamic, 1.0);
		}

		entity->RemoveComponent<SphereCollider>();
	}

	void PhysicsSystem::ConstructBoxCollider(const Entity& entity, const Vector3& halfExtents, const Vector3& offset) const
	{
		if (!entity->HasComponent<BoxCollider>())
		{
			const std::string funcName{ __FUNCTION__ };
			TRE_CORE_ERROR("[" + funcName + "] Entity \"" + entity->GetName() + "\" has no BoxCollider to construct.");
			assert(entity->HasComponent<BoxCollider>());
		}

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
			PxActorFlags actorFlags = sharedData.m_RigidDynamic->getActorFlags();
			actorFlags.raise(PxActorFlag::eDISABLE_GRAVITY);
			sharedData.m_RigidDynamic->setActorFlags(actorFlags);
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

	void PhysicsSystem::DestructBoxCollider(const Entity& entity) const
	{
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
			const unsigned nbShapes = sharedData.m_RigidDynamic->getNbShapes();
			const std::shared_ptr<PxShape* []> buffer(new PxShape * [nbShapes]); // I hate that I have to do this...
			const unsigned actualNbShapes = sharedData.m_RigidDynamic->getShapes(buffer.get(), nbShapes);

			for (unsigned i = 0; i < actualNbShapes; ++i)
			{
				if (buffer[i]->getGeometryType() != PxGeometryType::eBOX) continue;

				// there should only be ONE of each physics component, so it's safe to stop looping here
				sharedData.m_RigidDynamic->detachShape(*buffer[i]); break;
			}

			PxRigidBodyExt::updateMassAndInertia(*sharedData.m_RigidDynamic, 1.0);
		}

		entity->RemoveComponent<BoxCollider>();
	}

	void PhysicsSystem::ConstructRigidBody(const Entity& entity) const
	{
		if (!entity->HasComponent<Rigidbody>())
		{
			const std::string funcName{ __FUNCTION__ };
			TRE_CORE_ERROR("[" + funcName + "] Entity \"" + entity->GetName() + "\" has no Rigidbody to construct.");
			assert(entity->HasComponent<Rigidbody>());
		}

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
		PxActorFlags actorFlags = sharedData.m_RigidDynamic->getActorFlags();
		if (sharedData.m_RigidDynamic) // enabling gravity
			actorFlags.clear(PxActorFlag::eDISABLE_GRAVITY);
		else // TODO: disabling gravity
			actorFlags.raise(PxActorFlag::eDISABLE_GRAVITY);
		sharedData.m_RigidDynamic->setActorFlags(actorFlags);

		sharedData.m_AttachedComponents |= PhysicsComponentTypes::Rigidbody;
	}

	void PhysicsSystem::DestructRigidBody(const Entity& entity) const
	{
		SharedData& sharedData = m_Actors[entity->GetGUID()];
		sharedData.m_AttachedComponents &= ~PhysicsComponentTypes::Rigidbody;

		// if colliders are *still* present on this entity
		if (sharedData.m_AttachedComponents)
		{
			// turn the gravity off so that these colliders won't 'fall'
			PxActorFlags actorFlags = sharedData.m_RigidDynamic->getActorFlags();
			actorFlags.raise(PxActorFlag::eDISABLE_GRAVITY);
			sharedData.m_RigidDynamic->setActorFlags(actorFlags);
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

	void PhysicsSystem::AddForce(const Entity& entity, Vector3 force) const
	{
		assert(entity->HasComponent<Rigidbody>()); // just in case
		m_Actors[entity->GetGUID()].m_RigidDynamic->addForce(VEC3_CAST(PxVec3, force));
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
				// ConstructSphereCollider(entity, halfExtent);
				ConstructBoxCollider(entity, Vector3{ halfExtent });
			}
		}
	}
}
