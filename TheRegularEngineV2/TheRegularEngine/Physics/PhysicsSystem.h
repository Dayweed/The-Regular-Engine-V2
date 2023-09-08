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
#include "Core/System.h"
#include "Core/ECS.h"
#include "PhysX/PxPhysicsAPI.h"
#include "Vector3.h"

// USE_PHYSX_PVD is not defined in Release
#ifdef _DEBUG
#define USE_PHYSX_PVD 0
#endif

// PhysX 5.1.3 Docs: https://nvidia-omniverse.github.io/PhysX/physx/5.1.3/_build/physx/latest/physx_api.html

namespace TRE
{
	struct SphereCollider
	{
		physx::PxRigidActor* m_RigidActor = nullptr;
		float m_Radius = 1.0f;
	};

	struct BoxCollider
	{
		physx::PxRigidActor* m_RigidActor = nullptr;
		glm::vec3 m_HalfExtents = Vector3(0.5f);
	};

	// GOTTA SEPARATE ACTOR/BODY AND SHAPES!!!
	// RIGIDBODY AND COLLIDERSSSSSS


	//struct Rigidbody
	//{
	//	physx::PxRigidBody* m_thingy = nullptr;
	//	float m_Mass = 1.0f;
	//	float m_Drag = 0.0f;
	//	float m_AngularDrag = 0.05f;
	//	bool m_UseGravity = true;
	//	bool m_IsKinematic = false;
	//	// interpolation modes
	//	// collision detection modes
	//	// constraints - freeze position x,y,z & rotation x, y, z
	//};

	class PhysicsSystem : public ECSSystem
	{
	public:
		PhysicsSystem();

		bool TESTUpdate();
		void Update() override;
		void OnDestroyGO() override;
		void Shutdown() override;

		/* !
		@function      ConstructSphereCollider
		@author        Prashanth Subrahmanyam Sharma (p.sharma@digipen.edu)

		@params        entity    The entity to create the component for.
		@params        radius    The collider's radius.
		@params        offset    The offset from the entity's position, if applicable.

		@brief         Creates a SphereCollider component for the given entity,
					   overwriting any current SphereCollider for this entity.

		Example:
		Entity e1 = ECSManager::Instance().CreateEntity("ball");
		ConstructSphereCollider(e1);
		*//*__________________________________________________________________________*/
		void ConstructSphereCollider(const Entity& entity, const float radius = 1.0f, const Vector3& offset = Vector3::Zero()) const;

		/* !
		@function      DestructSphereCollider
		@author        Prashanth Subrahmanyam Sharma (p.sharma@digipen.edu)

		@params        entity    The entity containing the collider to destroy.

		@brief         Destroys an entity's SphereCollider component.

		Example:
		Entity e1 = ECSManager::Instance().CreateEntity("ball");
		ConstructSphereCollider(e1);
		// ----- using collider here... -----
		DestructSphereCollider(e1)
		*//*__________________________________________________________________________*/
		void DestructSphereCollider(const Entity& entity) const;

		/* !
		@function      ConstructBoxCollider
		@author        Prashanth Subrahmanyam Sharma (p.sharma@digipen.edu)

		@params        entity         The entity to create the component for.
		@params        halfExtents    The collider's half extents in all axes.
		@params        offset         The offset from the entity's position, if
									  applicable.

		@brief         Creates a BoxCollider component for the given entity,
					   overwriting any current BoxCollider for this entity.

		Example:
		Entity e1 = ECSManager::Instance().CreateEntity("box");
		ConstructBoxCollider(e1);
		*//*__________________________________________________________________________*/
		void ConstructBoxCollider(const Entity& entity, const Vector3& halfExtents = Vector3(0.5f), const Vector3& offset = Vector3::Zero()) const;

		/* !
		@function      DestructBoxCollider
		@author        Prashanth Subrahmanyam Sharma (p.sharma@digipen.edu)

		@params        entity    The entity containing the collider to destroy.

		@brief         Destroys an entity's BoxCollider component.

		Example:
		Entity e1 = ECSManager::Instance().CreateEntity("box");
		ConstructBoxCollider(e1);
		// ----- using collider here... -----
		DestructBoxCollider(e1)
		*//*__________________________________________________________________________*/
		void DestructBoxCollider(const Entity& entity) const;

		// void CreateRigidBody(const Entity& entity) const;

		// void AddForce(const Entity& entity, Vector3 force/*, ForceMode mode = ForceMode.Force*/) const;

		//This test function creates a stack of shapes
		void CreateStack(const physx::PxTransform& t, unsigned size, float halfExtent) const;

	private:

		bool m_IsReadyForUpdate = false;

		physx::PxDefaultAllocator		m_Allocator;
		physx::PxDefaultErrorCallback	m_ErrorCallback;

		physx::PxFoundation*			m_Foundation = nullptr;
		physx::PxPvd*					m_Pvd = nullptr;
		physx::PxPvdTransport*			m_Transport = nullptr;
		physx::PxPhysics*				m_Physics = nullptr;
		physx::PxDefaultCpuDispatcher*	m_Dispatcher = nullptr;
		physx::PxScene*					m_Scene = nullptr;
		physx::PxMaterial*				m_Material = nullptr;

		physx::PxRigidStatic*			m_GroundPlane = nullptr; // REMEMBER TO RELEASE SHAPES, DAMN IT.
	};
}
