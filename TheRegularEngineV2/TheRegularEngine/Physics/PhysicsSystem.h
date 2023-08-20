#pragma once
#include "Core/System.h"

#define HAHA_PHYSX_TEST 0
#if HAHA_PHYSX_TEST
#include "PhysX/PxPhysicsAPI.h"
#define HAHA_PHYSX_PVD 0
#endif

namespace TRE
{
	class PhysicsSystem : public System
	{
	public:
		PhysicsSystem();
		~PhysicsSystem();

		bool TESTInit();
		void Update() override;
		void RenderImgui() override;
		void OnDestroyGO() override;
		void Shutdown() override;

	private:

		// Systems aren't meant to have variables, Components are.
		// But I need variables that all components should be able to access, but
		// they cannot be in a System...hmm...

		bool m_IsInitialized = false;

#if HAHA_PHYSX_TEST
		physx::PxDefaultAllocator		m_Allocator;
		physx::PxDefaultErrorCallback	m_ErrorCallback;
		physx::PxFoundation*			m_Foundation = nullptr;
		physx::PxPhysics*				m_Physics = nullptr;
		physx::PxDefaultCpuDispatcher*	m_Dispatcher = nullptr;
		physx::PxScene*					m_Scene = nullptr;
		physx::PxMaterial*				m_Material = nullptr;
		physx::PxPvd*					m_Pvd = nullptr;
		physx::PxReal					m_stackZ = 10.0f;

		//This function creates a stack of boxes
		void createStack(const physx::PxTransform& t, physx::PxU32 size, physx::PxReal halfExtent);
#endif
	};
}
