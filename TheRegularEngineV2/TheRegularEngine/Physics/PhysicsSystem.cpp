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
#if 1
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
#endif

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

		const Entity e1 = ECSManager::Instance().CreateEntity("box 1");
		e1->GetComponent<Transform>().m_Position = { 0, 10, 0 };
		ConstructBoxCollider(e1);
		// e1->AddComponent<Rigidbody>(); ConstructRigidBody(e1);

		//Entity e2 = ECSManager::Instance().CreateEntity("box 1");
		//e2->GetComponent<Transform>().m_Position = { 1,1,0 };
		//ConstructSphereCollider(e2);

		return m_IsReadyForUpdate = true;
	}

	void PhysicsSystem::Update()
	{
		//if (!m_IsReadyForUpdate) TESTUpdate();
		// makes a non-void function only run once
		// without any if branches, using short-circuiting! :D
		m_IsReadyForUpdate || TESTUpdate();

#if 1
		static std::time_t start_timer = std::time(nullptr);
		const long long result = std::time(nullptr) - start_timer;
		if (result >= 1)
		{
			auto vec = ECSManager::Instance().GetEntities<Rigidbody>();
			if (!vec.empty())
			{
				//BoxCollider collider = vec.front()->GetComponent<BoxCollider>();
				//auto* rb = collider.m_RigidDynamic->is<PxRigidBody>();
				//rb->addForce({ 0, 800, 0 });

				AddForce(vec.front(), { 0, 8, 0 });
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
			const PhysicsComponent* component = pair.second;
			entity->GetComponent<Transform>().m_Position = VEC3_CAST(glm::vec3, component->m_RigidDynamic->getGlobalPose().p);

			// I hope this is right XO
			const PxVec3 eulerAngles = QuatToEulerAngles(component->m_RigidDynamic->getGlobalPose().q);
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
		auto& sphereCollider = entity->AddComponent<SphereCollider>();
		// add component if missing, otherwise get existing component

		const Vector3 objPos = entity->GetComponent<Transform>().m_Position;
		const PxVec3 colliderPos = VEC3_CAST(PxVec3, objPos) + VEC3_CAST(PxVec3, offset);

		// create a 'container', that being the PxActor (specifically a PxRigidDynamic in this case)
		PxRigidDynamic* body = m_Physics->createRigidDynamic(PxTransform(colliderPos));

		// fill the 'container' with a shape, that being a sphere
		PxRigidActorExt::createExclusiveShape(*body, PxSphereGeometry(radius), *m_DefaultMaterial);

		PxRigidBodyExt::updateMassAndInertia(*body, 1.0f);

		m_Scene->addActor(*body);
		sphereCollider.m_AttachedComponents |= PhysicsComponents::SphereCollider;
		sphereCollider.m_RigidDynamic = body;
		sphereCollider.m_Radius = radius;

		m_Actors[entity->GetGUID()] = &sphereCollider;
	}

	void PhysicsSystem::DestructSphereCollider(const Entity& entity) const
	{
		SphereCollider& sphereCollider = entity->GetComponent<SphereCollider>();
		PxRigidDynamic* rigidDynamic = sphereCollider.m_RigidDynamic;

		// reset bit for this component
		sphereCollider.m_AttachedComponents &= ~PhysicsComponents::SphereCollider;

		if (!sphereCollider.m_AttachedComponents)
		{
			m_Scene->removeActor(*rigidDynamic);
			rigidDynamic->release();
			m_Actors.erase(entity->GetGUID());
		}
		else // there's still more attached physics components
		{
			const unsigned nbShapes = rigidDynamic->getNbShapes();
			const std::shared_ptr<PxShape* []> buffer(new PxShape * [nbShapes]); // I hate that I have to do this...
			const unsigned actualNbShapes = rigidDynamic->getShapes(buffer.get(), nbShapes);

			for (unsigned i = 0; i < actualNbShapes; ++i)
			{
				if (buffer[i]->getGeometryType() != PxGeometryType::eSPHERE) continue;

				// there should only be ONE of each physics component, so it's safe to stop looping here
				rigidDynamic->detachShape(*buffer[i]); break;
			}
		}

		entity->RemoveComponent<SphereCollider>();
	}

	void PhysicsSystem::ConstructBoxCollider(const Entity& entity, const Vector3& halfExtents, const Vector3& offset) const
	{
		auto& boxCollider = entity->AddComponent<BoxCollider>();
		// add component if missing, otherwise get existing component

		const Vector3 objPos = entity->GetComponent<Transform>().m_Position;
		const PxVec3 colliderPos = VEC3_CAST(PxVec3, objPos) + VEC3_CAST(PxVec3, offset);

		// create a 'container', that being the PxActor (specifically a PxRigidDynamic in this case)
		PxRigidDynamic* body = m_Physics->createRigidDynamic(PxTransform(colliderPos));

		// fill the 'container' with a shape, that being a box
		/*auto huh = */PxRigidActorExt::createExclusiveShape(*body, PxBoxGeometry(VEC3_CAST(PxVec3, halfExtents)), *m_DefaultMaterial);
		// huh->setFlag(PxShapeFlag::eSIMULATION_SHAPE, false);

		PxRigidBodyExt::updateMassAndInertia(*body, 1.0f);

		m_Scene->addActor(*body);
		boxCollider.m_AttachedComponents |= PhysicsComponents::BoxCollider;
		boxCollider.m_RigidDynamic = body;
		boxCollider.m_HalfExtents = halfExtents;

		m_Actors[entity->GetGUID()] = &boxCollider;
	}

	void PhysicsSystem::DestructBoxCollider(const Entity& entity) const
	{
		BoxCollider& boxCollider = entity->GetComponent<BoxCollider>();
		PxRigidDynamic* rigidDynamic = boxCollider.m_RigidDynamic;

		// reset bit for this component
		boxCollider.m_AttachedComponents &= ~PhysicsComponents::BoxCollider;

		if (!boxCollider.m_AttachedComponents)
		{
			m_Scene->removeActor(*rigidDynamic);
			rigidDynamic->release();
			m_Actors.erase(entity->GetGUID());
		}
		else // there's still more attached physics components
		{
			const unsigned nbShapes = rigidDynamic->getNbShapes();
			const std::shared_ptr<PxShape* []> buffer(new PxShape * [nbShapes]); // I hate that I have to do this...
			const unsigned actualNbShapes = rigidDynamic->getShapes(buffer.get(), nbShapes);

			for (unsigned i = 0; i < actualNbShapes; ++i)
			{
				if (buffer[i]->getGeometryType() != PxGeometryType::eBOX) continue;

				// there should only be ONE of each physics component, so it's safe to stop looping here
				rigidDynamic->detachShape(*buffer[i]); break;
			}
		}

		entity->RemoveComponent<BoxCollider>();
	}

#if 1
	void PhysicsSystem::ConstructRigidBody(const Entity& entity) const
	{
		auto it = m_Actors.find(entity->GetGUID());

		if (it == m_Actors.end()) // create rigidbody here
		{
			Rigidbody& rb = entity->AddComponent<Rigidbody>();
			const Vector3& pos = entity->GetComponent<Transform>().m_Position;

			rb.m_RigidDynamic = m_Physics->createRigidDynamic(PxTransform{ VEC3_CAST(PxVec3, pos) });
			// rb.m_RigidDynamic->setActorFlags(PxActorFlag::eDISABLE_GRAVITY);
			m_Scene->addActor(*rb.m_RigidDynamic);

			rb.m_AttachedComponents |= PhysicsComponents::Rigidbody;
			m_Actors[entity->GetGUID()] = &rb;
		}

		// assign rigidbody here(?)
		// auto& rb = entity->GetComponent<Rigidbody>();
	}
	void PhysicsSystem::DestructRigidBody(const Entity& entity) const
	{
		;
	}
#endif

#if 1
	void PhysicsSystem::AddForce(const Entity& entity, Vector3 force) const
	{
		Rigidbody& rb = entity->GetComponent<Rigidbody>();
		rb.m_RigidDynamic->addForce(VEC3_CAST(PxVec3, force));
	}
#endif

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
