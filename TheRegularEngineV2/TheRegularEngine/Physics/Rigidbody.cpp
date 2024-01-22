#include "pch.h"
#include "Rigidbody.h"
#include "PhysicsSystem.h"
#include "Core/Transform.h"

using namespace physx;
// to save my dwindling sanity

namespace TRE
{
	void to_json(nlohmann::json& j, const Rigidbody& t)
	{
		j = nlohmann::json{
			// WriteMemberToJSON(m_IsActive),
			WriteMemberToJSON(m_Mass),
			WriteMemberToJSON(m_Drag),
			WriteMemberToJSON(m_AngularDrag),
			WriteMemberToJSON(m_UseGravity),
			WriteMemberToJSON(m_IsKinematic),
			WriteMemberToJSON(m_FreezePositionX),
			WriteMemberToJSON(m_FreezePositionY),
			WriteMemberToJSON(m_FreezePositionZ),
			WriteMemberToJSON(m_FreezeRotationX),
			WriteMemberToJSON(m_FreezeRotationY),
			WriteMemberToJSON(m_FreezeRotationZ),
		};
	}

	void from_json(const nlohmann::json& j, Rigidbody& t)
	{
		// ReadMemberFromJSON(m_IsActive);
		ReadMemberFromJSON(m_Mass);
		ReadMemberFromJSON(m_Drag);
		ReadMemberFromJSON(m_AngularDrag);
		ReadMemberFromJSON(m_UseGravity);
		ReadMemberFromJSON(m_IsKinematic);
		ReadMemberFromJSON(m_FreezePositionX);
		ReadMemberFromJSON(m_FreezePositionY);
		ReadMemberFromJSON(m_FreezePositionZ);
		ReadMemberFromJSON(m_FreezeRotationX);
		ReadMemberFromJSON(m_FreezeRotationY);
		ReadMemberFromJSON(m_FreezeRotationZ);
	}

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
			tempSharedData.m_RigidDynamic->setActorFlag(PxActorFlag::eSEND_SLEEP_NOTIFIES, true);

#ifdef _DEBUG
			{
				char* string = nullptr;
				if (entity->GetName() == "Holey")
					string = (char*)"Holey";
				else if (entity->GetName() == "Moley")
					string = (char*)"Moley";
				else if (entity->GetName() == "Slippery Body")
					string = (char*)"Slippery Body";
				else
					string = (char*)"Rigidbody";

				tempSharedData.m_RigidDynamic->setName(string);
			}
#endif
			m_Scene->addActor(*tempSharedData.m_RigidDynamic);

			tempSharedData.m_GUID = entity->GetGUID();

			m_Actors[entity->GetGUID()] = tempSharedData;
		}

		SharedData& sharedData = m_Actors[entity->GetGUID()];
		PxRigidBodyExt::updateMassAndInertia(*sharedData.m_RigidDynamic, 1.0f);

		// activate gravity by default
		bool useGravity = entity->GetComponent<Rigidbody>().m_UseGravity; // TODO: disabling gravity
		(void)useGravity;
		sharedData.m_RigidDynamic->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, !entity->GetComponent<Rigidbody>().m_UseGravity);

		// this is necessary to allow the actor to freakin move by physics and forces and such
		sharedData.m_RigidDynamic->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, entity->GetComponent<Rigidbody>().m_IsKinematic);

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

	void PhysicsSystem::AddForce(const Entity& entity, const glm::vec3& force, const ForceMode::Enum mode) const
	{
		PhysicsComponentAssertion(Rigidbody);
		PxForceMode::Enum physxForceMode;
		switch (mode)
		{
		case ForceMode::Impulse:        physxForceMode = PxForceMode::eIMPULSE; break;
		case ForceMode::VelocityChange: physxForceMode = PxForceMode::eVELOCITY_CHANGE; break;
		case ForceMode::Acceleration:   physxForceMode = PxForceMode::eACCELERATION; break;
		case ForceMode::Force:          [[fallthrough]];
		default:                        physxForceMode = PxForceMode::eFORCE; break;
		}

		m_Actors[entity->GetGUID()].m_RigidDynamic->addForce(VEC3_CAST(PxVec3, force), physxForceMode);
	}

	void PhysicsSystem::ConstrainPositionX(const Entity& entity, bool state) const
	{
		PhysicsComponentAssertion(Rigidbody);
		m_Actors[entity->GetGUID()].m_RigidDynamic->setRigidDynamicLockFlag(PxRigidDynamicLockFlag::eLOCK_LINEAR_X, state);
		entity->GetComponent<Rigidbody>().m_FreezePositionX = state;
	}

	void PhysicsSystem::ConstrainPositionY(const Entity& entity, bool state) const
	{
		PhysicsComponentAssertion(Rigidbody);
		m_Actors[entity->GetGUID()].m_RigidDynamic->setRigidDynamicLockFlag(PxRigidDynamicLockFlag::eLOCK_LINEAR_Y, state);
		entity->GetComponent<Rigidbody>().m_FreezePositionY = state;
	}

	void PhysicsSystem::ConstrainPositionZ(const Entity& entity, bool state) const
	{
		PhysicsComponentAssertion(Rigidbody);
		m_Actors[entity->GetGUID()].m_RigidDynamic->setRigidDynamicLockFlag(PxRigidDynamicLockFlag::eLOCK_LINEAR_Z, state);
		entity->GetComponent<Rigidbody>().m_FreezePositionZ = state;
	}

	void PhysicsSystem::ConstrainRotationX(const Entity& entity, bool state) const
	{
		PhysicsComponentAssertion(Rigidbody);
		m_Actors[entity->GetGUID()].m_RigidDynamic->setRigidDynamicLockFlag(PxRigidDynamicLockFlag::eLOCK_ANGULAR_X, state);
		entity->GetComponent<Rigidbody>().m_FreezeRotationX = state;
	}

	void PhysicsSystem::ConstrainRotationY(const Entity& entity, bool state) const
	{
		PhysicsComponentAssertion(Rigidbody);
		m_Actors[entity->GetGUID()].m_RigidDynamic->setRigidDynamicLockFlag(PxRigidDynamicLockFlag::eLOCK_ANGULAR_Y, state);
		entity->GetComponent<Rigidbody>().m_FreezeRotationY = state;
	}

	void PhysicsSystem::ConstrainRotationZ(const Entity& entity, bool state) const
	{
		PhysicsComponentAssertion(Rigidbody);
		m_Actors[entity->GetGUID()].m_RigidDynamic->setRigidDynamicLockFlag(PxRigidDynamicLockFlag::eLOCK_ANGULAR_Z, state);
		entity->GetComponent<Rigidbody>().m_FreezeRotationZ = state;
	}

	glm::vec3 PhysicsSystem::GetLinearVelocity(const Entity& entity) const
	{
		PhysicsComponentAssertion(Rigidbody);
		const PxVec3 vel = m_Actors[entity->GetGUID()].m_RigidDynamic->getLinearVelocity();
		return VEC3_CAST(glm::vec3, vel);
	}

	void PhysicsSystem::SetLinearVelocity(const Entity& entity, const glm::vec3& vel) const
	{
		PhysicsComponentAssertion(Rigidbody);
		m_Actors[entity->GetGUID()].m_RigidDynamic->setLinearVelocity(VEC3_CAST(PxVec3, vel));
	}

	void PhysicsSystem::UpdateRigidbody(const Entity& entity) const
	{
		PhysicsComponentAssertion(Rigidbody);

		Rigidbody& rigidbody = entity->GetComponent<Rigidbody>();

		rigidbody.m_IsInitialized || ConstructRigidbody(entity);

		PxRigidDynamic* rigidDynamic = m_Actors[entity->GetGUID()].m_RigidDynamic;

		UpdateActorPose(entity);

		rigidbody.m_Mass = rigidDynamic->getMass();
		// rigidbody.m_Drag = ;
		// rigidbody.m_AngularDrag = ;
		//rigidbody.m_UseGravity = !rigidDynamic->getActorFlags().isSet(PxActorFlag::eDISABLE_GRAVITY);
		rigidDynamic->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, !rigidbody.m_UseGravity);
		rigidDynamic->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, rigidbody.m_IsKinematic);
	}

	void PhysicsSystem::DestructRigidbody(const Entity& entity) const
	{
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
			sharedData.m_MarkForRemoval = true;
		}
		//entity->RemoveComponent<Rigidbody>();
	}
}
