#include "pch.h"
#include "PhysicsSystem.h"
#include "TREIncludes.h"

using namespace physx;
// to save my dwindling sanity

namespace TRE
{
	// NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Rigidbody, m_Mass, m_Drag, m_AngularDrag, m_UseGravity, m_IsKinematic);

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
}
