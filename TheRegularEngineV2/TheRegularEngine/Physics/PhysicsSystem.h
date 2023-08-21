#pragma once
#include "Core/System.h"

#define HAHA_PHYSX_TEST 0
#if HAHA_PHYSX_TEST
#include "PhysX/PxPhysicsAPI.h"
#define HAHA_PHYSX_PVD 0
#endif

// PhysX 5.1.3 Docs: https://nvidia-omniverse.github.io/PhysX/physx/5.1.3/_build/physx/latest/physx_api.html

namespace TRE
{
	class PhysicsSystem : public System
	{
	public:
		PhysicsSystem();
		~PhysicsSystem() override;

		bool TESTInit();
		void Update() override;
		void RenderImgui() override;
		void OnDestroyGO() override;
		void Shutdown() override;

	private:

		// Systems aren't meant to have variables, Components are.
		// But for variables that all components should be able to access,
		// being in a System is alright.

		bool m_IsInitialized = false;

#if HAHA_PHYSX_TEST
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
		physx::PxReal					m_stackZ = 10.0f;

		//This function creates a stack of boxes
		void CreateStack(const physx::PxTransform& t, physx::PxU32 size, physx::PxReal halfExtent);
#endif
	};
}
