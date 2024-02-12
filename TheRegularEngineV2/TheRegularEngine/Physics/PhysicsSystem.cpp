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
#include "Core/Engine.h"
#include "Core/Transform.h"
#include "PhysicsSystem.h"
#include "Physics/SphereCollider.h"
#include "Physics/BoxCollider.h"
#include "Physics/CapsuleCollider.h"
#include "Physics/CylinderCollider.h"
#include "Core/Serialization.h"

// USE_PHYSX_PVD is not defined in Release
#ifdef _DEBUG
#define USE_PHYSX_PVD 1
#endif

using namespace physx;
// to save my dwindling sanity

namespace TRE
{
	// largely identical to PhysX's SnippetTriggers' implementation
	PxFilterFlags SimulationFilterShader(PxFilterObjectAttributes attributes0, PxFilterData filterData0,
		PxFilterObjectAttributes attributes1, PxFilterData filterData1,
		PxPairFlags& pairFlags, const void* constantBlock, PxU32 constantBlockSize)
	{
		UNUSED_VALUE(constantBlock);
		UNUSED_VALUE(constantBlockSize);

		// let triggers through
		if (PxFilterObjectIsTrigger(attributes0) || PxFilterObjectIsTrigger(attributes1))
		{
			pairFlags = PxPairFlag::eTRIGGER_DEFAULT;
			return PxFilterFlags();
		}

		// Group vs Group Collision Suppression
		// .word0 is guaranteed to be the group (I dove into PhysX's source for this info :_) )

		// I'll be exceptionally upset if this works.
		if (!PxGetGroupCollisionFlag(static_cast<PxU16>(filterData0.word0), static_cast<PxU16>(filterData1.word0)))
		{
			// I am now exceptionally upset.
			return PxFilterFlag::eSUPPRESS;
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
		e1->GetComponent<Transform>().m_Position = { 0,8,0 };
		e1->AddComponent<BoxCollider>();
		ConstructBoxCollider(e1, { 7, 3, 7 });
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

	void PhysicsSystem::Init()
	{
		TRE_CORE_INFO("Initializing Physics/PhysX systems...");

		//Create foundation is similar to initializing the scene
		m_Foundation = PxCreateFoundation(PX_PHYSICS_VERSION, m_Allocator, m_ErrorCallback);
		assert(m_Foundation);

#ifdef _DEBUG
		// doesn't pass in name parameter when allocating stuff on its own now
		// one less thing passed in, the better I guess.
		m_Foundation->setReportAllocationNames(_DEBUG);
#else
		m_Foundation->setReportAllocationNames(false);
#endif

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

		CreatePhysXScene();

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
		m_DefaultMaterial = m_Physics->createMaterial(PX_MAX_F32, PX_MAX_F32, 0.f);

		m_FrictionlessMaterial = m_Physics->createMaterial(0, 0, 0);

		// initialize collision matrix
		for (auto& row : m_CollisionMatrix)
			row = 0;

		SetLayerNames();
		LoadCollisionMatrix();
		ApplyCollisionMatrix();

		TRE_CORE_INFO("Physics/PhysX systems initialization complete! :D");
	}

	void PhysicsSystem::Update()
	{
		UpdateAllComponents();
		ResizeAllColliders();

		//if (!m_IsReadyForUpdate) TESTUpdate();
		// makes a non-void function only run once
		// without any if branches, using short-circuiting! :D
		isReadyForUpdate || TESTUpdate();

		DestroyOutdatedComponents();
	}

	void PhysicsSystem::GameUpdate()
	{
		if (m_PauseState) return;
		// Accumulator, courtesy of 
		// https://nvidia-omniverse.github.io/PhysX/physx/5.1.3/docs/Simulation.html#the-simulation-loop
		static float accumulator = 0.0f;
		constexpr float step = 1.0f / 60.0f;
		accumulator += Engine::GetInstance().GetWindow()->GetDeltaTime();
		if (accumulator < step) return;

#if 0
		static std::time_t start_timer = std::time(nullptr);
		const long long result = std::time(nullptr) - start_timer;

		if (result >= 1)
		{
			try
			{
				Entity e1 = ECSManager::Instance().CreateEntity("cylin");
				e1->GetComponent<Transform>().m_Position = glm::vec3(10, 10, 10);
				e1->AddComponent<Rigidbody>(); ConstructRigidbody(e1);
				e1->AddComponent<CylinderCollider>(); ConstructCylinderCollider(e1);
			}
			catch (std::exception& e)
			{
				std::cout << e.what() << "\n";
			}
			catch (...)
			{
				std::cout << "ummmmmm" << "\n";
			}


			printf("====================================================\n");

			// reset timer
			std::time(&start_timer);
		}
#endif

		m_SimulationEventCallback.m_CollisionHistory.clear();

		// there's previous trigger history (PTH) & (current) trigger history (CTH)
		// step 1) replace all entries in PTH with those from CTH
		m_SimulationEventCallback.m_PrevTriggerHistory = std::move(m_SimulationEventCallback.m_TriggerHistory);

		// step 2) clear CTH
		m_SimulationEventCallback.m_TriggerHistory = std::vector<TriggerHistoryEntry>();

		accumulator -= step;
		m_Scene->simulate(step);
		m_Scene->fetchResults(true);
		// ^ step 3) CTH is overwritten by fetchResults()

#pragma region Manual IsTriggerStay Handling
		// Manually handling stuff for IsTriggerStay to work properly because PhysX said no :_)
		for (const auto& prevEntry : m_SimulationEventCallback.m_PrevTriggerHistory)
		{
			auto& CTH = m_SimulationEventCallback.m_TriggerHistory; // current trigger history

			// const auto iter = std::find_if(CTH.begin(), CTH.end(), equalityPredicate);
			const auto iter = std::ranges::find_if(CTH, [prevEntry](const TriggerHistoryEntry entry)
				{
					return entry.m_First == prevEntry.m_First && entry.m_Second == prevEntry.m_Second;
				});

			// step 4: if nothing changed (no entry/exit) AND PTH says there was an entry / stay, CTH.stay = true
			if (iter == CTH.end() && prevEntry.m_Flags & (TriggerHistoryEntryEnum::Enter | TriggerHistoryEntryEnum::Stay))
				CTH.emplace_back(prevEntry.m_First, prevEntry.m_Second, TriggerHistoryEntryEnum::Stay);
		}
#pragma endregion

		for (const auto& pair : m_Actors)
		{
			const Entity entity = ECSManager::Instance().FindEntity(pair.first);
			if (!entity) continue; // HERE WE ARE FOR SOME REASON??
			const SharedData& sharedData = pair.second;

			glm::vec3 offset{};
			const unsigned attachedComponents = sharedData.m_AttachedComponents;
			if (attachedComponents & PhysicsComponentTypes::SphereCollider)
			{
				const SphereCollider& sphereCollider = entity->GetComponent<SphereCollider>();
				offset = sphereCollider.m_Offset;
			}
			else if (attachedComponents & PhysicsComponentTypes::BoxCollider)
			{
				const BoxCollider& boxCollider = entity->GetComponent<BoxCollider>();
				offset = boxCollider.m_Offset;
			}
			else if (attachedComponents & PhysicsComponentTypes::CapsuleCollider)
			{
				const CapsuleCollider& capsuleCollider = entity->GetComponent<CapsuleCollider>();
				offset = capsuleCollider.m_Offset;
			}
			else if (attachedComponents & PhysicsComponentTypes::CylinderCollider)
			{
				const CylinderCollider& cylinderCollider = entity->GetComponent<CylinderCollider>();
				offset = cylinderCollider.m_Offset;
			}

			Transform& transform = entity->GetComponent<Transform>();
			const PxVec3 pos = sharedData.m_RigidDynamic->getGlobalPose().p;
			transform.m_Position = VEC3_CAST(glm::vec3, pos) - offset;

			const PxQuat rotQuat = sharedData.m_RigidDynamic->getGlobalPose().q;
			const glm::vec3 eulerAnglesInRad = glm::eulerAngles(glm::quat{ rotQuat.w, rotQuat.x, rotQuat.y, rotQuat.z });
			transform.m_Rotation = eulerAnglesInRad / PI * 180.0f;

			transform.m_IsDirty = true;
		}
	}

	void PhysicsSystem::BeforeReset()
	{
		for (const auto& actor : m_Actors)
		{
			const unsigned attachedComponents = actor.second.m_AttachedComponents;
			Entity entity = ECSManager::Instance().FindEntity(actor.first);

			if (attachedComponents & PhysicsComponentTypes::Rigidbody)
				DestructRigidbody(entity);

			if (attachedComponents & PhysicsComponentTypes::SphereCollider)
				DestructSphereCollider(entity);

			if (attachedComponents & PhysicsComponentTypes::BoxCollider)
				DestructBoxCollider(entity);

			if (attachedComponents & PhysicsComponentTypes::CapsuleCollider)
				DestructCapsuleCollider(entity);

			if (attachedComponents & PhysicsComponentTypes::CylinderCollider)
				DestructCylinderCollider(entity);
		}
		m_Actors.clear();
		PX_RELEASE(m_Scene);

		m_SimulationEventCallback.m_CollisionHistory.clear();
		m_SimulationEventCallback.m_TriggerHistory.clear();
		m_SimulationEventCallback.m_PrevTriggerHistory.clear();
	}

	void PhysicsSystem::AfterReset()
	{
		CreatePhysXScene();

		for (const Entity& entity : ECSManager::Instance().GetEntities<Rigidbody>())
		{
			// i just want the squiggly lines to go away...
			UNUSED_VALUE(ConstructRigidbody(entity));
		}

		for (const Entity& entity : ECSManager::Instance().GetEntities<SphereCollider>())
		{
			SphereCollider& component{ entity->GetComponent<SphereCollider>() };
			UNUSED_VALUE(ConstructSphereCollider(entity, component.m_Radius, component.m_Offset));
		}

		for (const Entity& entity : ECSManager::Instance().GetEntities<BoxCollider>())
		{
			BoxCollider& component{ entity->GetComponent<BoxCollider>() };
			UNUSED_VALUE(ConstructBoxCollider(entity, component.m_HalfExtents, component.m_Offset));
		}

		for (const Entity& entity : ECSManager::Instance().GetEntities<CapsuleCollider>())
		{
			CapsuleCollider& component{ entity->GetComponent<CapsuleCollider>() };
			UNUSED_VALUE(ConstructCapsuleCollider(entity, component.m_Radius, component.m_HalfHeight, component.m_Offset));
		}

		for (const Entity& entity : ECSManager::Instance().GetEntities<CylinderCollider>())
		{
			CylinderCollider& component{ entity->GetComponent<CylinderCollider>() };
			UNUSED_VALUE(ConstructCylinderCollider(entity, component.m_Radius, component.m_Height, component.m_Offset));
		}
	}

	void PhysicsSystem::Shutdown()
	{
		// HOW THE HECK DID THIS MAGICALLY WORK ?!?
		// WAIT I FOUND OUT.
		// NEVER CLOSE THE PVD BEFORE THE APPLICATION AAAAAAAAAAAAA

		m_Actors.clear();
		PX_RELEASE(m_DefaultMaterial);
		PX_RELEASE(m_Dispatcher);
		PX_RELEASE(m_Scene);
		if (m_Physics)
			PxCloseExtensions();
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

	std::unordered_map<unsigned, Entity> PhysicsSystem::GenerateEntityActorVector() const
	{
		std::unordered_map<unsigned, Entity> vector;
		vector.reserve(m_Actors.size());
		for (const auto& actor : m_Actors)
		{
			vector.emplace(actor.second.m_RigidDynamic->getInternalActorIndex(), ECSManager::Instance().FindEntity(actor.first));
		}

		return vector;
	}

	void PhysicsSystem::GetCollisionHistory(VectorCollidedEntities& onEnter, VectorCollidedEntities& onStay, VectorCollidedEntities& onExit)
	{
		onEnter.clear();
		onStay.clear();
		onExit.clear();

		std::vector<std::pair<unsigned, unsigned>> CollisionsStayID{};
		std::vector<std::pair<unsigned, unsigned>> CollisionsEnterID{};
		std::vector<std::pair<unsigned, unsigned>> CollisionsExitID{};

		for (size_t i{}; i < m_SimulationEventCallback.m_CollisionHistory.size(); ++i)	// Doing this way cos the lambda crashes when iterating
		{
			const CollisionHistoryEntry& entry{ m_SimulationEventCallback.m_CollisionHistory[i] };
			if (entry.m_Flags & CollisionHistoryEntryEnum::Enter)
			{
				std::pair<unsigned, unsigned> pair{ entry.m_First, entry.m_Second };
				CollisionsEnterID.emplace_back(pair);
			}
			else if (entry.m_Flags & CollisionHistoryEntryEnum::Stay)
			{
				std::pair<unsigned, unsigned> pair{ entry.m_First, entry.m_Second };
				CollisionsStayID.emplace_back(pair);
			}
			else if (entry.m_Flags & CollisionHistoryEntryEnum::Exit)
			{
				std::pair<unsigned, unsigned> pair{ entry.m_First, entry.m_Second };
				CollisionsExitID.emplace_back(pair);
			}
		}

		// Generate Entity Actor Vector
		std::unordered_map<unsigned, Entity> EntityActor{ GenerateEntityActorVector() };

		// Find Entity
		for (auto IDs : CollisionsStayID)
		{
			if (EntityActor[IDs.first] != nullptr && ECSManager::Instance().IsValidEntity(EntityActor[IDs.first])
				&& EntityActor[IDs.second] != nullptr && ECSManager::Instance().IsValidEntity(EntityActor[IDs.second]))
			{
				onStay.emplace_back(EntityActor[IDs.first], EntityActor[IDs.second]);
			}
		}
		for (auto IDs : CollisionsEnterID)
		{
			if (EntityActor[IDs.first] != nullptr && ECSManager::Instance().IsValidEntity(EntityActor[IDs.first])
				&& EntityActor[IDs.second] != nullptr && ECSManager::Instance().IsValidEntity(EntityActor[IDs.second]))
			{
				onEnter.emplace_back(EntityActor[IDs.first], EntityActor[IDs.second]);
			}
		}
		for (auto IDs : CollisionsExitID)
		{
			if (EntityActor[IDs.first] != nullptr && ECSManager::Instance().IsValidEntity(EntityActor[IDs.first])
				&& EntityActor[IDs.second] != nullptr && ECSManager::Instance().IsValidEntity(EntityActor[IDs.second]))
			{
				onExit.emplace_back(EntityActor[IDs.first], EntityActor[IDs.second]);
			}
		}
	}

	void PhysicsSystem::GetTriggerHistory(VectorCollidedEntities& onEnter, VectorCollidedEntities& onStay, VectorCollidedEntities& onExit)
	{
		onEnter.clear();
		onStay.clear();
		onExit.clear();

		std::vector<std::pair<unsigned, unsigned>> CollisionsStayID{};
		std::vector<std::pair<unsigned, unsigned>> CollisionsEnterID{};
		std::vector<std::pair<unsigned, unsigned>> CollisionsExitID{};

		for (size_t i{}; i < m_SimulationEventCallback.m_TriggerHistory.size(); ++i)	// Doing this way cos the lambda crashes when iterating
		{
			const TriggerHistoryEntry& entry{ m_SimulationEventCallback.m_TriggerHistory[i] };
			if (entry.m_Flags & TriggerHistoryEntryEnum::Enter)
			{
				std::pair<unsigned, unsigned> pair{ entry.m_First, entry.m_Second };
				CollisionsEnterID.emplace_back(pair);
			}
			else if (entry.m_Flags & TriggerHistoryEntryEnum::Stay)
			{
				std::pair<unsigned, unsigned> pair{ entry.m_First, entry.m_Second };
				CollisionsStayID.emplace_back(pair);
			}
			else if (entry.m_Flags & TriggerHistoryEntryEnum::Exit)
			{
				std::pair<unsigned, unsigned> pair{ entry.m_First, entry.m_Second };
				CollisionsExitID.emplace_back(pair);
			}
		}

		// Generate Entity Actor Vector
		std::unordered_map<unsigned, Entity> EntityActor{ GenerateEntityActorVector() };

		// Find Entity
		for (auto IDs : CollisionsStayID)
		{
			if (EntityActor[IDs.first] != nullptr && ECSManager::Instance().IsValidEntity(EntityActor[IDs.first])
				&& EntityActor[IDs.second] != nullptr && ECSManager::Instance().IsValidEntity(EntityActor[IDs.second]))
			{
				onStay.emplace_back(EntityActor[IDs.first], EntityActor[IDs.second]);
			}
		}
		for (auto IDs : CollisionsEnterID)
		{
			if (EntityActor[IDs.first] != nullptr && ECSManager::Instance().IsValidEntity(EntityActor[IDs.first])
				&& EntityActor[IDs.second] != nullptr && ECSManager::Instance().IsValidEntity(EntityActor[IDs.second]))
			{
				onEnter.emplace_back(EntityActor[IDs.first], EntityActor[IDs.second]);
			}
		}
		for (auto IDs : CollisionsExitID)
		{
			if (EntityActor[IDs.first] != nullptr && ECSManager::Instance().IsValidEntity(EntityActor[IDs.first])
				&& EntityActor[IDs.second] != nullptr && ECSManager::Instance().IsValidEntity(EntityActor[IDs.second]))
			{
				onExit.emplace_back(EntityActor[IDs.first], EntityActor[IDs.second]);
			}
		}
	}

	std::vector<std::pair<Entity, Entity>> PhysicsSystem::GetPrevTriggerHistory() const
	{
		std::vector<std::pair<unsigned, unsigned>> CollisionsID;
		CollisionsID.reserve(m_SimulationEventCallback.m_PrevTriggerHistory.size());

		for (size_t i{}; i < m_SimulationEventCallback.m_PrevTriggerHistory.size(); ++i)	// Doing this way cos the lambda crashes when iterating
		{
			const CollisionHistoryEntry& entry{ m_SimulationEventCallback.m_PrevTriggerHistory[i] };
			std::pair<unsigned, unsigned> pair{ entry.m_First, entry.m_Second };
			CollisionsID.emplace_back(pair);
		}

		// Generate Entity Actor Vector
		std::unordered_map<unsigned, Entity> EntityActor{ GenerateEntityActorVector() };

		// Find Entity
		std::vector<std::pair<Entity, Entity>> Collisions;
		Collisions.reserve(CollisionsID.size());
		for (auto IDs : CollisionsID)
		{
		// Make sure the entities still exists
			if (EntityActor[IDs.first] != nullptr && ECSManager::Instance().IsValidEntity(EntityActor[IDs.first])
			&& EntityActor[IDs.second] != nullptr && ECSManager::Instance().IsValidEntity(EntityActor[IDs.second]))
			{
				Collisions.emplace_back(EntityActor[IDs.first], EntityActor[IDs.second]);
			}
		}

		return Collisions;
	}

	void PhysicsSystem::SetDrawDebug(bool draw)
	{
		m_DrawDebugLines = draw;
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
		(void)hasRigidbody;
		// if there's a rigidbody attached, enable gravity
		rigidDynamic->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, !entity->GetComponent<Rigidbody>().m_UseGravity);

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
		MarkAsTrigger.operator() < CylinderCollider > (entity);
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
		(void)hasRigidbody;
		// if there's a rigidbody attached, enable gravity
		rigidDynamic->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, !entity->GetComponent<Rigidbody>().m_UseGravity);

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
		MarkAsCollider.operator() < CylinderCollider > (entity);
	}

	bool PhysicsSystem::IsCollisionEnter(const Entity& entity_1, const Entity& entity_2) const
	{
		if (!m_Actors.contains(entity_1->GetGUID()))
		{
			TRE_CORE_WARN("[" + std::string{ __FUNCTION__ } + "] Entity 1 (\"" + entity_1->GetName() + "\") did not contain any physics components.");
			return false;
		}
		if (!m_Actors.contains(entity_2->GetGUID()))
		{
			TRE_CORE_WARN("[" + std::string{ __FUNCTION__ } + "] Entity 2 (\"" + entity_2->GetName() + "\") did not contain any physics components.");
			return false;
		}

		unsigned actor1Index = m_Actors[entity_1->GetGUID()].m_RigidDynamic->getInternalActorIndex();
		unsigned actor2Index = m_Actors[entity_2->GetGUID()].m_RigidDynamic->getInternalActorIndex();

		// ensure that the first index is lesser than (<) the second index
		if (actor1Index > actor2Index) std::swap(actor1Index, actor2Index);

		for (const auto& [first, second, flags] : m_SimulationEventCallback.m_CollisionHistory)
		{
			if (first == actor1Index && second == actor2Index)
			{
				return flags & CollisionHistoryEntryEnum::Enter;
			}
		}
		return false;
	}

	bool PhysicsSystem::IsCollisionStay(const Entity& entity_1, const Entity& entity_2) const
	{
		if (!m_Actors.contains(entity_1->GetGUID()))
		{
			TRE_CORE_WARN("[" + std::string{ __FUNCTION__ } + "] Entity 1 (\"" + entity_1->GetName() + "\") did not contain any physics components.");
			return false;
		}
		if (!m_Actors.contains(entity_2->GetGUID()))
		{
			TRE_CORE_WARN("[" + std::string{ __FUNCTION__ } + "] Entity 2 (\"" + entity_2->GetName() + "\") did not contain any physics components.");
			return false;
		}

		unsigned actor1Index = m_Actors[entity_1->GetGUID()].m_RigidDynamic->getInternalActorIndex();
		unsigned actor2Index = m_Actors[entity_2->GetGUID()].m_RigidDynamic->getInternalActorIndex();

		// ensure that the first index is lesser than (<) the second index
		if (actor1Index > actor2Index) std::swap(actor1Index, actor2Index);

		for (const auto& [first, second, flags] : m_SimulationEventCallback.m_CollisionHistory)
		{
			if (first == actor1Index && second == actor2Index)
			{
				return flags & CollisionHistoryEntryEnum::Stay;
			}
		}
		return false;
	}

	bool PhysicsSystem::IsCollisionExit(const Entity& entity_1, const Entity& entity_2) const
	{
		if (!m_Actors.contains(entity_1->GetGUID()))
		{
			TRE_CORE_WARN("[" + std::string{ __FUNCTION__ } + "] Entity 1 (\"" + entity_1->GetName() + "\") did not contain any physics components.");
			return false;
		}
		if (!m_Actors.contains(entity_2->GetGUID()))
		{
			TRE_CORE_WARN("[" + std::string{ __FUNCTION__ } + "] Entity 2 (\"" + entity_2->GetName() + "\") did not contain any physics components.");
			return false;
		}

		unsigned actor1Index = m_Actors[entity_1->GetGUID()].m_RigidDynamic->getInternalActorIndex();
		unsigned actor2Index = m_Actors[entity_2->GetGUID()].m_RigidDynamic->getInternalActorIndex();

		// ensure that the first index is lesser than (<) the second index
		if (actor1Index > actor2Index) std::swap(actor1Index, actor2Index);

		for (const auto& [first, second, flags] : m_SimulationEventCallback.m_CollisionHistory)
		{
			if (first == actor1Index && second == actor2Index)
			{
				return flags & CollisionHistoryEntryEnum::Exit;
			}
		}
		return false;
	}

	bool PhysicsSystem::IsTriggerEnter(const Entity& entity_1, const Entity& entity_2) const
	{
		if (!m_Actors.contains(entity_1->GetGUID()))
		{
			TRE_CORE_WARN("[" + std::string{ __FUNCTION__ } + "] Entity 1 (\"" + entity_1->GetName() + "\") did not contain any physics components.");
			return false;
		}
		if (!m_Actors.contains(entity_2->GetGUID()))
		{
			TRE_CORE_WARN("[" + std::string{ __FUNCTION__ } + "] Entity 2 (\"" + entity_2->GetName() + "\") did not contain any physics components.");
			return false;
		}

		unsigned actor1Index = m_Actors[entity_1->GetGUID()].m_RigidDynamic->getInternalActorIndex();
		unsigned actor2Index = m_Actors[entity_2->GetGUID()].m_RigidDynamic->getInternalActorIndex();

		// ensure that the first index is lesser than (<) the second index
		if (actor1Index > actor2Index) std::swap(actor1Index, actor2Index);

		for (const auto& [first, second, flags] : m_SimulationEventCallback.m_TriggerHistory)
		{
			if (first == actor1Index && second == actor2Index)
			{
				return flags & TriggerHistoryEntryEnum::Enter;
			}
		}
		return false;
	}

	bool PhysicsSystem::IsTriggerStay(const Entity& entity_1, const Entity& entity_2) const
	{
		if (!m_Actors.contains(entity_1->GetGUID()))
		{
			TRE_CORE_WARN("[" + std::string{ __FUNCTION__ } + "] Entity 1 (\"" + entity_1->GetName() + "\") did not contain any physics components.");
			return false;
		}
		if (!m_Actors.contains(entity_2->GetGUID()))
		{
			TRE_CORE_WARN("[" + std::string{ __FUNCTION__ } + "] Entity 2 (\"" + entity_2->GetName() + "\") did not contain any physics components.");
			return false;
		}

		unsigned actor1Index = m_Actors[entity_1->GetGUID()].m_RigidDynamic->getInternalActorIndex();
		unsigned actor2Index = m_Actors[entity_2->GetGUID()].m_RigidDynamic->getInternalActorIndex();

		// ensure that the first index is lesser than (<) the second index
		if (actor1Index > actor2Index) std::swap(actor1Index, actor2Index);

		for (const auto& [first, second, flags] : m_SimulationEventCallback.m_TriggerHistory)
		{
			if (first == actor1Index && second == actor2Index)
			{
				return flags & TriggerHistoryEntryEnum::Stay;
			}
		}
		return false;
	}

	bool PhysicsSystem::IsTriggerExit(const Entity& entity_1, const Entity& entity_2) const
	{
		if (!m_Actors.contains(entity_1->GetGUID()))
		{
			TRE_CORE_WARN("[" + std::string{ __FUNCTION__ } + "] Entity 1 (\"" + entity_1->GetName() + "\") did not contain any physics components.");
			return false;
		}
		if (!m_Actors.contains(entity_2->GetGUID()))
		{
			TRE_CORE_WARN("[" + std::string{ __FUNCTION__ } + "] Entity 2 (\"" + entity_2->GetName() + "\") did not contain any physics components.");
			return false;
		}

		unsigned actor1Index = m_Actors[entity_1->GetGUID()].m_RigidDynamic->getInternalActorIndex();
		unsigned actor2Index = m_Actors[entity_2->GetGUID()].m_RigidDynamic->getInternalActorIndex();

		// ensure that the first index is lesser than (<) the second index
		if (actor1Index > actor2Index) std::swap(actor1Index, actor2Index);

		for (const auto& [first, second, flags] : m_SimulationEventCallback.m_TriggerHistory)
		{
			if (first == actor1Index && second == actor2Index)
			{
				return flags & TriggerHistoryEntryEnum::Exit;
			}
		}
		return false;
	}

	void PhysicsSystem::ResizeAllColliders()
	{
		// Update Sphere Collider if Dirty
		for (const Entity& entity : ECSManager::Instance().GetEntities<SphereCollider>())
		{
			auto& collider = entity->GetComponent<SphereCollider>();
			const auto& transform = entity->GetComponent<Transform>();

			if (transform.m_IsDirty || collider.m_IsDirty)
			{
				UpdateColliderData(entity, collider.m_Offset);

				if (collider.m_IsDirty)
				{
					ResizeSphereCollider(entity, collider.m_Radius);
					collider.m_IsDirty = false;
				}
			}
		}

		// Update Box Collider if Dirty
		for (const Entity& entity : ECSManager::Instance().GetEntities<BoxCollider>())
		{
			auto& collider = entity->GetComponent<BoxCollider>();
			const auto& transform = entity->GetComponent<Transform>();

			if (transform.m_IsDirty || collider.m_IsDirty)
			{
				UpdateColliderData(entity, collider.m_Offset);

				if (collider.m_IsDirty)
				{
					ResizeBoxCollider(entity, collider.m_HalfExtents);
					collider.m_IsDirty = false;
				}
			}
		}

		// Update the Update if Update
		for (const Entity& entity : ECSManager::Instance().GetEntities<CapsuleCollider>())
		{
			auto& collider = entity->GetComponent<CapsuleCollider>();
			const auto& transform = entity->GetComponent<Transform>();

			if (transform.m_IsDirty || collider.m_IsDirty)
			{
				UpdateColliderData(entity, collider.m_Offset);

				if (collider.m_IsDirty)
				{
					ResizeCapsuleCollider(entity, collider.m_Radius, collider.m_HalfHeight);
					collider.m_IsDirty = false;
				}
			}
		}

		// Update the Update UPDATE if the Update Update
		for (const Entity& entity : ECSManager::Instance().GetEntities<CylinderCollider>())
		{
			auto& collider = entity->GetComponent<CylinderCollider>();
			const auto& transform = entity->GetComponent<Transform>();

			if (transform.m_IsDirty || collider.m_IsDirty)
			{
				UpdateColliderData(entity, collider.m_Offset);

				if (collider.m_IsDirty)
				{
					ResizeCylinderCollider(entity, collider.m_Radius, collider.m_Height);
					collider.m_IsDirty = false;
				}
			}
		}
	}

	void PhysicsSystem::DestroyOutdatedComponents() const
	{
		// Physics Component Destruction and Removal
		for (const auto& [guid, sharedData] : m_Actors)
		{
			// Scenario 1: Entity with physics components was just deleted (Has the Removal component)
			// if Has Removal component and Has physics component, destruct physics component
			// HasRemoval && HasPhysicsComp -> Destruct()
			// A && X

			// Scenario 2: physics component was just removed from entity
			// if attachedComps had it, but the entity doesn't, destruct physics component
			// (attachedComps & ComponentEnum) && !HasPhysicsComp -> Destruct()
			// B && !X

			// Accounting for both scenarios..
			// if (A && X) || (B && !X)

			Entity entity = ECSManager::Instance().FindEntity(guid);

			// Skip this entity if invalid
			if (entity == nullptr) continue;

			const bool hasRemovalComponent = entity->HasComponent<Removal>();

			// if the attached comps say yes, but the entity says no...
			// there is a mismatch. Thus, destroy that component.
			const unsigned attachedComponents = sharedData.m_AttachedComponents;

			// Rigidbody
			{
				const bool isInAttachedComponents = attachedComponents & PhysicsComponentTypes::Rigidbody;
				const bool hasPhysicsComponent = entity->HasComponent<Rigidbody>();
				if (hasRemovalComponent && hasPhysicsComponent || isInAttachedComponents && !hasPhysicsComponent)
					DestructRigidbody(entity);
			}

			// SphereCollider
			{
				const bool isInAttachedComponents = attachedComponents & PhysicsComponentTypes::SphereCollider;
				const bool hasPhysicsComponent = entity->HasComponent<SphereCollider>();
				if (hasRemovalComponent && hasPhysicsComponent || isInAttachedComponents && !hasPhysicsComponent)
					DestructSphereCollider(entity);
			}

			// BoxCollider
			{
				const bool isInAttachedComponents = attachedComponents & PhysicsComponentTypes::BoxCollider;
				const bool hasPhysicsComponent = entity->HasComponent<BoxCollider>();
				if (hasRemovalComponent && hasPhysicsComponent || isInAttachedComponents && !hasPhysicsComponent)
					DestructBoxCollider(entity);
			}

			// CapsuleCollider
			{
				const bool isInAttachedComponents = attachedComponents & PhysicsComponentTypes::CapsuleCollider;
				const bool hasPhysicsComponent = entity->HasComponent<CapsuleCollider>();
				if (hasRemovalComponent && hasPhysicsComponent || isInAttachedComponents && !hasPhysicsComponent)
					DestructCapsuleCollider(entity);
			}

			// CylinderCollider
			{
				const bool isInAttachedComponents = attachedComponents & PhysicsComponentTypes::CylinderCollider;
				const bool hasPhysicsComponent = entity->HasComponent<CylinderCollider>();
				if (hasRemovalComponent && hasPhysicsComponent || isInAttachedComponents && !hasPhysicsComponent)
					DestructCylinderCollider(entity);
			}

		}

		// erasing elements in a map: https://stackoverflow.com/a/8234813
		for (auto it = m_Actors.begin(); it != m_Actors.end();)
		{
			// Remove nullptr entities or mark for removal
			if (ECSManager::Instance().FindEntity(it->second.m_GUID) == nullptr || it->second.m_MarkForRemoval)
				it = m_Actors.erase(it);
			else
				++it;
		}

		// The actual ECS components are also removed in the Destruct##Comp() functions above.
		// This is because the Update components does GetEntities<Comp>(), which could
		// include outdated comps if RemoveComponent<Comp>() was not done by this point.
	}

	void PhysicsSystem::UpdateAllComponents() const
	{
		for (const Entity& entity : ECSManager::Instance().GetEntities<Rigidbody>())
			UpdateRigidbody(entity);

		for (const Entity& entity : ECSManager::Instance().GetEntities<SphereCollider>())
			UpdateSphereCollider(entity);

		for (const Entity& entity : ECSManager::Instance().GetEntities<BoxCollider>())
			UpdateBoxCollider(entity);

		for (const Entity& entity : ECSManager::Instance().GetEntities<CapsuleCollider>())
			UpdateCapsuleCollider(entity);

		for (const Entity& entity : ECSManager::Instance().GetEntities<CylinderCollider>())
			UpdateCylinderCollider(entity);
	}

	void PhysicsSystem::UpdateActorPose(const Entity& entity, const glm::vec3& offset) const
	{
		const glm::vec3 pos = entity->GetComponent<Transform>().m_Position + offset;
		const PxVec3 colliderPos = VEC3_CAST(PxVec3, pos);

		const glm::vec3 eulerAnglesInRad = entity->GetComponent<Transform>().m_Rotation * PI / 180.0f;
		const glm::quat rotQuat{ eulerAnglesInRad };

		const PxTransform transform(colliderPos, PxQuat{ rotQuat.x, rotQuat.y, rotQuat.z, rotQuat.w });
		m_Actors[entity->GetGUID()].m_RigidDynamic->setGlobalPose(transform);
	}

	void PhysicsSystem::CreatePhysXScene()
	{
		if (m_Scene)
			PX_RELEASE(m_Scene);

		PxSceneDesc sceneDesc(m_Physics->getTolerancesScale());
		sceneDesc.gravity = PxVec3(0.0f, -9.81f * 25, 0.0f);

		//A cpu thread for the scene
		PX_RELEASE(m_Dispatcher);

		m_Dispatcher = PxDefaultCpuDispatcherCreate(2);
		assert(m_Dispatcher);
		sceneDesc.cpuDispatcher = m_Dispatcher;

		// SimulationEventCallback must inherit PxSimulationEventCallback
		// PUBLICLY in order to work, otherwise...
		// C2243: 'type cast': conversion from 'TRE::SimulationEventCallback *'
		// to 'physx::PxSimulationEventCallback *' exists, but is inaccessible
		sceneDesc.simulationEventCallback = &m_SimulationEventCallback;

		//A thread that will do collision management
		sceneDesc.filterShader = SimulationFilterShader;

		sceneDesc.broadPhaseType = PxBroadPhaseType::eABP;

		m_Scene = m_Physics->createScene(sceneDesc);
		assert(m_Scene);
	}

#pragma region Collision Layers
	void PhysicsSystem::SetCollisionMatrix(const CollisionMatrix& matrix)
	{
		m_CollisionMatrix = matrix;
	}

	PhysicsSystem::CollisionMatrix PhysicsSystem::GetCollisionMatrix()
	{
		return m_CollisionMatrix;
	}

	void PhysicsSystem::SaveCollisionMatrix()
	{
		constexpr const char* collisionMatrixFileName = "Resources/CollisionMatrix.json";
		constexpr const char* collisionMatrixObjectName = "Collision Matrix Values";

		rapidjson::Document doc;
		doc.SetObject();
		WriteToExternalFile(doc, collisionMatrixFileName);

		ObjectSerializer serializer(collisionMatrixFileName);
		ObjectBuilder objBuilder;
		Allocator& allocator = serializer.getDoc().GetAllocator();

		for (int i = 0; i < CollisionLayer::TOTAL; ++i)
		{
			std::string variableName = CollisionLayer::m_LayerNameList[i].first;
			const int value = static_cast<int>(m_CollisionMatrix[i].to_ullong());
			objBuilder.insertValue(variableName, value, allocator);
		}

		serializer.AddObjectToDoc(objBuilder.getValue(), collisionMatrixObjectName);
		serializer.writeToDoc(collisionMatrixFileName);
	}

	void PhysicsSystem::LoadCollisionMatrix()
	{
		constexpr const char* collisionMatrixFileName = "Resources/CollisionMatrix.json";
		constexpr const char* collisionMatrixObjectName = "Collision Matrix Values";

		rapidjson::Document doc;
		doc.SetObject();
		ReadExternalFile(doc, collisionMatrixFileName);

		const ObjectDeserializer deserializer(collisionMatrixFileName);
		for (int i = 0; i < CollisionLayer::TOTAL; ++i)
		{
			std::string variableName = CollisionLayer::m_LayerNameList[i].first;
			int value = 0;
			if (!deserializer.get_value(collisionMatrixObjectName, variableName.c_str(), value))
				TRE_CORE_ERROR("Unable to read value with name \"{0}.{1}\"", collisionMatrixObjectName, variableName);
			m_CollisionMatrix[i] = value;
		}
	}

	void PhysicsSystem::ApplyCollisionMatrix()
	{
		for (PxU16 i = 0; i < m_CollisionMatrix.size(); ++i)
			for (PxU16 j = 0; j < m_CollisionMatrix.size(); ++j)
				PxSetGroupCollisionFlag(i, j, m_CollisionMatrix[i].test(j));
	}

	void PhysicsSystem::ChangeCollisionLayer(const Entity& entity) const
	{
		// get the collider component and obtain the new layer from it
		int layer = 0;

		const unsigned attachedComponents = m_Actors[entity->GetGUID()].m_AttachedComponents;
		if (attachedComponents & PhysicsComponentTypes::BoxCollider)
			layer = entity->GetComponent<BoxCollider>().m_CollisionLayer.m_LayerID;
		else if (attachedComponents & PhysicsComponentTypes::SphereCollider)
			layer = entity->GetComponent<SphereCollider>().m_CollisionLayer.m_LayerID;
		else if (attachedComponents & PhysicsComponentTypes::CapsuleCollider)
			layer = entity->GetComponent<CapsuleCollider>().m_CollisionLayer.m_LayerID;
		else if (attachedComponents & PhysicsComponentTypes::CylinderCollider)
			layer = entity->GetComponent<CylinderCollider>().m_CollisionLayer.m_LayerID;

		// ensure that it's between 0 and 31
		const PxU16 value = static_cast<PxU16>(layer);
		const PxU16 group = value <= 31 ? value : 0;

		PxSetGroup(*m_Actors[entity->GetGUID()].m_RigidDynamic, group);
	}

	void PhysicsSystem::SetLayerNames()
	{
		constexpr const char* collisionLayerFileName = "../Resources/CollisionLayerNames.txt";
		std::ifstream file(collisionLayerFileName);
		if (!file)
			return;

		for (int i = 0; i < CollisionLayer::TOTAL; ++i)
		{
			std::string huh;
			file >> huh;
			CollisionLayer::m_LayerNameList[i].first = huh;
		}

		file.close();
	}
#pragma endregion

	void PhysicsSystem::ChangeIsActive(const Entity& entity) const
	{
		auto& [rigidDynamic, attachedComponents, _unused1, _unused2] = m_Actors[entity->GetGUID()];
		UNUSED_VALUE(_unused1);
		UNUSED_VALUE(_unused2);
		bool isActive = true;

		if (attachedComponents & PhysicsComponentTypes::BoxCollider)
			isActive = entity->GetComponent<BoxCollider>().m_IsActive;
		else if (attachedComponents & PhysicsComponentTypes::SphereCollider)
			isActive = entity->GetComponent<SphereCollider>().m_IsActive;
		else if (attachedComponents & PhysicsComponentTypes::CapsuleCollider)
			isActive = entity->GetComponent<CapsuleCollider>().m_IsActive;
		else if (attachedComponents & PhysicsComponentTypes::CylinderCollider)
			isActive = entity->GetComponent<CylinderCollider>().m_IsActive;

		rigidDynamic->setActorFlag(PxActorFlag::eDISABLE_SIMULATION, !isActive);
	}

	void PhysicsSystem::ChangeMaterial(const Entity& entity) const
	{
		auto& [rigidDynamic, attachedComponents, _unused1, _unused2] = m_Actors[entity->GetGUID()];
		UNUSED_VALUE(_unused1);
		UNUSED_VALUE(_unused2);

		int material = PhysicsMaterial::Default;

		if (attachedComponents & PhysicsComponentTypes::BoxCollider)
			material = entity->GetComponent<BoxCollider>().m_PhysicsMaterial.m_MaterialID;
		else if (attachedComponents & PhysicsComponentTypes::SphereCollider)
			material = entity->GetComponent<SphereCollider>().m_PhysicsMaterial.m_MaterialID;
		else if (attachedComponents & PhysicsComponentTypes::CapsuleCollider)
			material = entity->GetComponent<CapsuleCollider>().m_PhysicsMaterial.m_MaterialID;
		else if (attachedComponents & PhysicsComponentTypes::CylinderCollider)
			material = entity->GetComponent<CylinderCollider>().m_PhysicsMaterial.m_MaterialID;

		// there are only 4 shapes that can possibly be added - Sphere, Box, Capsule & Cylinder
		PxShape* shapes[4] = { nullptr };
		unsigned numberOfShapes = rigidDynamic->getShapes(shapes, 4);

		for (unsigned i = 0; i < numberOfShapes; ++i)
		{
			if (material == PhysicsMaterial::Default)
				shapes[i]->setMaterials(&m_DefaultMaterial, 1);
			else if (material == PhysicsMaterial::Frictionless)
				shapes[i]->setMaterials(&m_FrictionlessMaterial, 1);
		}
	}

	void PhysicsSystem::SetIsActive(const Entity& entity, bool state) const
	{
		const unsigned attachedComponents = m_Actors[entity->GetGUID()].m_AttachedComponents;
		if (attachedComponents & PhysicsComponentTypes::BoxCollider)
			entity->GetComponent<BoxCollider>().m_IsActive = state;
		else if (attachedComponents & PhysicsComponentTypes::SphereCollider)
			entity->GetComponent<SphereCollider>().m_IsActive = state;
		else if (attachedComponents & PhysicsComponentTypes::CapsuleCollider)
			entity->GetComponent<CapsuleCollider>().m_IsActive = state;
		else if (attachedComponents & PhysicsComponentTypes::CylinderCollider)
			entity->GetComponent<CylinderCollider>().m_IsActive = state;

		// now that the bool inside the component has been changed, the change function can be called
		ChangeIsActive(entity);
	}

	bool PhysicsSystem::GetIsActive(const Entity& entity) const
	{
		const unsigned attachedComponents = m_Actors[entity->GetGUID()].m_AttachedComponents;
		if (attachedComponents & PhysicsComponentTypes::BoxCollider)
			return entity->GetComponent<BoxCollider>().m_IsActive;
		else if (attachedComponents & PhysicsComponentTypes::SphereCollider)
			return entity->GetComponent<SphereCollider>().m_IsActive;
		else if (attachedComponents & PhysicsComponentTypes::CapsuleCollider)
			return entity->GetComponent<CapsuleCollider>().m_IsActive;
		else if (attachedComponents & PhysicsComponentTypes::CylinderCollider)
			return entity->GetComponent<CylinderCollider>().m_IsActive;
		else
			return false;
	}

	void PhysicsSystem::UpdateColliderData(const Entity& entity, const glm::vec3& offset)
	{
		if (m_Actors.contains(entity->GetGUID()) == false)
			return;

		const auto& transform = entity->GetComponent<Transform>();
		const glm::vec3 eulerAnglesInRad = transform.m_Rotation * PI / 180.0f;
		const glm::quat rotQuat{ eulerAnglesInRad };
		const auto xform = PxTransform(VEC3_CAST(PxVec3, transform.m_Position + offset), PxQuat{ rotQuat.x, rotQuat.y, rotQuat.z, rotQuat.w });

		m_Actors[entity->GetGUID()].m_RigidDynamic->setGlobalPose(xform);

		const unsigned attachedComponents = m_Actors[entity->GetGUID()].m_AttachedComponents;

		if (attachedComponents & PhysicsComponentTypes::SphereCollider)
			entity->GetComponent<SphereCollider>().m_Offset = offset;

		if (attachedComponents & PhysicsComponentTypes::BoxCollider)
			entity->GetComponent<BoxCollider>().m_Offset = offset;

		if (attachedComponents & PhysicsComponentTypes::CapsuleCollider)
			entity->GetComponent<CapsuleCollider>().m_Offset = offset;

		if (attachedComponents & PhysicsComponentTypes::CylinderCollider)
			entity->GetComponent<CylinderCollider>().m_Offset = offset;
	}

	void PhysicsSystem::SetPauseState(bool state)
	{
		m_PauseState = state;
	}

	bool PhysicsSystem::GetPauseState()
	{
		return m_PauseState;
	}
}

// DISCO RGB FONT FOR EDITOR COMPONENTS?????
