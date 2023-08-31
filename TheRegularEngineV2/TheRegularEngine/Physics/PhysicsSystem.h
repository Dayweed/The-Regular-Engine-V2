#pragma once
#include "Core/System.h"
#include "PhysX/PxPhysicsAPI.h"

// USE_PHYSX_PVD is not defined in Release
#if defined(DEBUG) | defined(_DEBUG)
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
		glm::vec3 m_HalfExtents = glm::vec3(1);
	};

	class PhysicsSystem : public ECSSystem
	{
	public:
		PhysicsSystem();
		~PhysicsSystem() override;

		bool TESTUpdate();
		void Update() override;
		// void RenderImgui() override;
		void OnDestroyGO() override;
		void Shutdown() override;

		/*!
		\brief	Creates a SphereCollider component for the given entity.
		\author	Prashanth S. Sharma p.sharma@digipen.edu

		\param	[in,out] go		The `const Entity&` representing the entity to create the component for.
		\param	[in]	 radius	The `const float` representing the collider's radius.
		\param	[in]	 offset	The offset from the entity's position, if applicable.
		*/
		void ConstructSphereCollider(const Entity& go, const float radius = 1.0f, const glm::vec3& offset = glm::vec3(0)) const;

		/*!
		\brief	Destroys an entity's SphereCollider component.
		\author	Prashanth S. Sharma p.sharma@digipen.edu

		\param	[in,out] go		The `const Entity&` representing the entity containing the collider to destroy.
		*/
		void DestructSphereCollider(const Entity& go) const;

		/*!
		\brief	Creates a BoxCollider component for the given entity.
		\author	Prashanth S. Sharma p.sharma@digipen.edu

		\param	[in,out] go				The `const Entity&` representing the entity to create the component for.
		\param	[in]	 halfExtents	The `const glm::vec3&` representing the collider's half extents in all axes.
		\param	[in]	 offset			The offset from the entity's position, if applicable.
		*/
		void ConstructBoxCollider(const Entity& go, const glm::vec3& halfExtents = glm::vec3(0.5f), const glm::vec3& offset = glm::vec3(0)) const;

		/*!
		\brief	Destroys an entity's BoxCollider component.
		\author	Prashanth S. Sharma p.sharma@digipen.edu

		\param	[in,out] go		The `const Entity&` representing the entity containing the collider to destroy.
		*/
		void DestructBoxCollider(const Entity& go) const;

		//This test function creates a stack of shapes
		void CreateStack(const physx::PxTransform& t, unsigned size, float halfExtent) const;

	private:

		// Systems aren't meant to have variables, Components are.
		// But for variables that all components should be able to access,
		// being in a System is alright.

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
