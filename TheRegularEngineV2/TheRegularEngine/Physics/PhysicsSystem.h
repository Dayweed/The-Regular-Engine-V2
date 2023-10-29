/*!
	@file      PhysicsSystem.h
	@author    Prashanth Subrahmanyam Sharma (Code Contribution 100%)
	@email     p.sharma@digipen.edu
	@date      03/09/2023
	@brief     This file contains the definition of the PhysicsSystem
			   class, which is an interface for PhysX functions.

	Copyright (C) 2023 DigiPen Institute of Technology.
	Reproduction or disclosure of this file or its contents without the
	prior written consent of DigiPen Institute of Technology is prohibited.
************************************************************************/
#pragma once
#include "pch.h"
#include "Core/System.h"
#include "Core/ECS.h"
#include "PhysicsComponents.h"

// PhysX 5.1.3 Docs: https://nvidia-omniverse.github.io/PhysX/physx/5.1.3/_build/physx/latest/physx_api.html

#define VEC3_CAST(type, vec) (##type{(vec).x, (vec).y, (vec).z})
#define PI 3.14159265358979323846f
#pragma region PhysicsComponentAssertions
#define PhysicsComponentConstructorAssertion(Type)														\
	if (!entity->HasComponent<Type>())																	\
	{																									\
		TRE_CORE_ERROR("[" __FUNCTION__ "] "															\
			"Entity \"" + entity->GetName() + "\" has no "+  #Type + " to construct.");					\
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

namespace TRE
{
	typedef struct HistoryEntryEnum
	{
		enum Enum : unsigned char
		{
			Enter = 1 << 0,
			Stay  = 1 << 1,
			Exit  = 1 << 2
		};
	} CollisionHistoryEntryEnum, TriggerHistoryEntryEnum;

	typedef struct HistoryEntry
	{
		unsigned m_First : 7, m_Second : 7, m_Flags : 3;
		// I wonder if these bitfield lengths need to be bigger...
	} CollisionHistoryEntry, TriggerHistoryEntry;

	class SimulationEventCallback : public physx::PxSimulationEventCallback
	{
	public:
		void onAdvance(const physx::PxRigidBody* const* bodyBuffer, const physx::PxTransform* poseBuffer, const physx::PxU32 count) override;
		void onConstraintBreak(physx::PxConstraintInfo* constraints, physx::PxU32 count) override;
		void onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs) override;
		void onSleep(physx::PxActor** actors, physx::PxU32 count) override;
		void onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count) override;
		void onWake(physx::PxActor** actors, physx::PxU32 count) override;

		// keeps track of IsCollisionEnter, IsCollisionStay and IsCollisionExit 'results'
		std::vector<CollisionHistoryEntry> m_CollisionHistory;

		// keeps track of IsTriggerEnter, IsTriggerStay and IsTriggerExit 'results'
		std::vector<TriggerHistoryEntry> m_TriggerHistory, m_PrevTriggerHistory;
		// why can't physx just handle this for me? :_)
	};

	class PhysicsSystem : public ECSSystem
	{
	public:
		bool TESTUpdate();

		void Init() override;
		void Update() override;
		void GameUpdate() override;
		void BeforeReset() override;
		void AfterReset() override;
		// void OnDestroyEntities() override;
		void Shutdown() override;

		std::unordered_map<unsigned, Entity> GenerateEntityActorVector();
		std::vector<std::pair<Entity, Entity>> GetCollisionHistory();
		std::vector<std::pair<Entity, Entity>> GetTriggerHistory();
		std::vector<std::pair<Entity, Entity>> GetPrevTriggerHistory();

		void SetDrawDebug(bool draw);

#pragma region Rigidbody Function Declarations
		/* !
		@function      ConstructRigidbody
		@author        Prashanth Subrahmanyam Sharma (p.sharma@digipen.edu)

		@params        entity         The entity to create the component for.

		@brief         Initializes the Rigidbody component for the given entity.

		Example:
		Entity e1 = ECSManager::Instance().CreateEntity("mass");
		e1->AddComponent<Rigidbody>();
		ConstructRigidbody(e1);
		*//*__________________________________________________________________________*/
		bool ConstructRigidbody(const Entity& entity) const;

		/* !
		@function      AddForce
		@author        Prashanth Subrahmanyam Sharma (p.sharma@digipen.edu)

		@params        entity    The entity to add force to.
		@params        force     The force to apply.
		@params        mode      The method of applying the given force.

		@brief         Adds a given force to the given's entity's Rigidbody component.

		Example:
		Entity e1 = ECSManager::Instance().CreateEntity("box");
		e1->AddComponent<Rigidbody>(); ConstructRigidbody(e1);
		AddForce(e1,{0, 80, 0});
		*//*__________________________________________________________________________*/
		void AddForce(const Entity& entity, const glm::vec3& force, const ForceMode::Enum mode) const;

		void ConstrainPositionX(const Entity& entity, const bool state) const;

		void ConstrainPositionY(const Entity& entity, const bool state) const;

		void ConstrainPositionZ(const Entity& entity, const bool state) const;

		void ConstrainRotationX(const Entity& entity, const bool state) const;

		void ConstrainRotationY(const Entity& entity, const bool state) const;

		void ConstrainRotationZ(const Entity& entity, const bool state) const;

		glm::vec3 GetLinearVelocity(const Entity& entity) const;

		void SetLinearVelocity(const Entity& entity, const glm::vec3& vel) const;

		void UpdateRigidbody(const Entity& entity) const;

		/* !
		@function      DestructRigidbody
		@author        Prashanth Subrahmanyam Sharma (p.sharma@digipen.edu)

		@params        entity    The entity containing the Rigidbody to destroy.

		@brief         Destroys an entity's Rigidbody component.

		Example:
		Entity e1 = ECSManager::Instance().CreateEntity("mass");
		e1->AddComponent<Rigidbody>();
		ConstructRigidbody(e1);
		// ----- using Rigidbody here... -----
		DestructRigidbody(e1)
		*//*__________________________________________________________________________*/
		void DestructRigidbody(const Entity& entity) const;
#pragma endregion

#pragma region SphereCollider Function Declarations
		/* !
		@function      ConstructSphereCollider
		@author        Prashanth Subrahmanyam Sharma (p.sharma@digipen.edu)

		@params        entity    The entity to create the component for.
		@params        radius    The collider's radius.
		@params        offset    The offset from the entity's position, if applicable.

		@brief         Initializes the SphereCollider component for the given entity.

		Example:
		Entity e1 = ECSManager::Instance().CreateEntity("ball");
		e1->AddComponent<SphereCollider>();
		ConstructSphereCollider(e1);
		*//*__________________________________________________________________________*/
		bool ConstructSphereCollider(const Entity& entity, const float radius = 1.0f, const glm::vec3& offset = glm::vec3{ 0 }) const;

		/* !
		@function      ResizeSphereCollider
		@author        Prashanth Subrahmanyam Sharma (p.sharma@digipen.edu)

		@params        entity       The entity to create the component for.
		@params        newRadius    The collider's new radius.

		@brief         Resizes the SphereCollider component for the given entity.

		Example:
		Entity e1 = ECSManager::Instance().CreateEntity("ball");
		e1->AddComponent<SphereCollider>(); ConstructSphereCollider(e1);
		ResizeSphereCollider(e1, 2);
		*//*__________________________________________________________________________*/
		void ResizeSphereCollider(const Entity& entity, const float newRadius) const;

		void UpdateSphereCollider(const Entity& entity) const;

		/* !
		@function      DestructSphereCollider
		@author        Prashanth Subrahmanyam Sharma (p.sharma@digipen.edu)

		@params        entity    The entity containing the collider to destroy.

		@brief         Destroys an entity's SphereCollider component.

		Example:
		Entity e1 = ECSManager::Instance().CreateEntity("ball");
		e1->AddComponent<SphereCollider>();
		ConstructSphereCollider(e1);
		// ----- using collider here... -----
		DestructSphereCollider(e1)
		*//*__________________________________________________________________________*/
		void DestructSphereCollider(const Entity& entity) const;
#pragma endregion

#pragma region BoxCollider Function Declarations
		/* !
		@function      ConstructBoxCollider
		@author        Prashanth Subrahmanyam Sharma (p.sharma@digipen.edu)

		@params        entity         The entity to create the component for.
		@params        halfExtents    The collider's half extents in all axes.
		@params        offset         The offset from the entity's position, if
									  applicable.

		@brief         Initializes the BoxCollider component for the given entity.

		Example:
		Entity e1 = ECSManager::Instance().CreateEntity("box");
		e1->AddComponent<BoxCollider>();
		ConstructBoxCollider(e1);
		*//*__________________________________________________________________________*/
		bool ConstructBoxCollider(const Entity& entity, const glm::vec3& halfExtents = glm::vec3{ 0.5f }, const glm::vec3& offset = glm::vec3{ 0 }) const;

		/* !
		@function      ResizeBoxCollider
		@author        Prashanth Subrahmanyam Sharma (p.sharma@digipen.edu)

		@params        entity            The entity to create the component for.
		@params        newHalfExtents    The collider's new half extents in all axes.

		@brief         Resizes the BoxCollider component for the given entity.

		Example:
		Entity e1 = ECSManager::Instance().CreateEntity("box");
		e1->AddComponent<BoxCollider>(); ConstructBoxCollider(e1);
		ResizeBoxCollider(e1, {1, 1, 1});
		*//*__________________________________________________________________________*/
		void ResizeBoxCollider(const Entity& entity, const glm::vec3& newHalfExtents) const;

		void UpdateBoxCollider(const Entity& entity) const;

		/* !
		@function      DestructBoxCollider
		@author        Prashanth Subrahmanyam Sharma (p.sharma@digipen.edu)

		@params        entity    The entity containing the collider to destroy.

		@brief         Destroys an entity's BoxCollider component.

		Example:
		Entity e1 = ECSManager::Instance().CreateEntity("box");
		e1->AddComponent<BoxCollider>();
		ConstructBoxCollider(e1);
		// ----- using collider here... -----
		DestructBoxCollider(e1)
		*//*__________________________________________________________________________*/
		void DestructBoxCollider(const Entity& entity) const;
#pragma endregion

#pragma region CapsuleCollider Function Declarations
		bool ConstructCapsuleCollider(const Entity& entity, const float radius = 1.0f, const float halfHeight = 0.5f, const glm::vec3& offset = glm::vec3{ 0 }) const;

		void ResizeCapsuleCollider(const Entity& entity, const float newRadius, const float newHalfHeight) const;

		void UpdateCapsuleCollider(const Entity& entity) const;

		void DestructCapsuleCollider(const Entity& entity) const;
#pragma endregion

		//This test function creates a stack of shapes
		void CreateStack(const physx::PxTransform& t, unsigned size, float halfExtent) const;

		void ColliderToTrigger(const Entity& entity) const;

		void TriggerToCollider(const Entity& entity) const;

		bool IsCollisionEnter(const Entity& entity_1, const Entity& entity_2) const;

		bool IsCollisionStay(const Entity& entity_1, const Entity& entity_2) const;

		bool IsCollisionExit(const Entity& entity_1, const Entity& entity_2) const;

		bool IsTriggerEnter(const Entity& entity_1, const Entity& entity_2) const;

		bool IsTriggerStay(const Entity& entity_1, const Entity& entity_2) const;

		bool IsTriggerExit(const Entity& entity_1, const Entity& entity_2) const;

	private:

		void ResizeAllColliders();
		void UpdateColliderData(const Entity& entity, const glm::vec3& offset);

		void DestroyOutdatedComponents() const;

		void UpdateAllComponents() const;

		mutable std::unordered_map<std::string, SharedData> m_Actors;

		physx::PxDefaultAllocator		m_Allocator;
		physx::PxDefaultErrorCallback	m_ErrorCallback;
		SimulationEventCallback			m_SimulationEventCallback;
		// OH MY GOD AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA

		physx::PxFoundation*			m_Foundation = nullptr;
		physx::PxPvd*					m_Pvd = nullptr;
		physx::PxPvdTransport*			m_Transport = nullptr;
		physx::PxPhysics*				m_Physics = nullptr;
		physx::PxDefaultCpuDispatcher*	m_Dispatcher = nullptr;
		physx::PxScene*					m_Scene = nullptr;
		physx::PxMaterial*				m_DefaultMaterial = nullptr;

		physx::PxRigidStatic*			m_GroundPlane = nullptr; // TEMPORARY PLANE
		physx::PxMaterial*				m_GroundPlaneMaterial = nullptr; // TEMPORARY MATERIAL

		bool m_DrawDebugLines = false;
	};
}
