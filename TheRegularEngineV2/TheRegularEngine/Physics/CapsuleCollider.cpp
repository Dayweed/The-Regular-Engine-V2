#include "pch.h"
#include "PhysicsSystem.h"
#include "TREIncludes.h"

using namespace physx;
// to save my dwindling sanity

namespace TRE
{
	void to_json(nlohmann::json& j, const CapsuleCollider& t)
	{
		j = nlohmann::json{
			WriteVec3MemberToJSON(m_Offset),
			WriteMemberToJSON(m_Radius),
			WriteMemberToJSON(m_HalfHeight),
			WriteMemberToJSON(m_IsTrigger),
		};
	}

	void from_json(const nlohmann::json& j, CapsuleCollider& t)
	{
		ReadVec3MemberFromJSON(m_Offset);
		ReadMemberFromJSON(m_Radius);
		ReadMemberFromJSON(m_HalfHeight);
		ReadMemberFromJSON(m_IsTrigger);
	}

	bool PhysicsSystem::ConstructCapsuleCollider(const Entity& entity, const float radius, const float halfHeight, const glm::vec3& offset) const
	{
		PhysicsComponentConstructorAssertion(CapsuleCollider);

		// if there are no existing physics components on the entity
		if (!m_Actors.contains(entity->GetGUID()))
		{
			SharedData tempSharedData;

			const glm::vec3 pos = entity->GetComponent<Transform>().m_Position + offset;
			const PxVec3 colliderPos = VEC3_CAST(PxVec3, pos);

			const glm::vec3 eulerAnglesInRad = entity->GetComponent<Transform>().m_Rotation * PI / 180.0f;
			const glm::quat rotQuat{ eulerAnglesInRad };

			const PxTransform transform(colliderPos, PxQuat{ rotQuat.x, rotQuat.y, rotQuat.z, rotQuat.w });

			tempSharedData.m_RigidDynamic = m_Physics->createRigidDynamic(transform);
			tempSharedData.m_RigidDynamic->setActorFlag(PxActorFlag::eSEND_SLEEP_NOTIFIES, true);
#ifdef _DEBUG
			tempSharedData.m_RigidDynamic->setName("CapsuleCollider");
#endif
			m_Scene->addActor(*tempSharedData.m_RigidDynamic);

			tempSharedData.m_GUID = entity->GetGUID();

			m_Actors[entity->GetGUID()] = tempSharedData;
		}

		SharedData& sharedData = m_Actors[entity->GetGUID()];

		PxShape* capsuleShape = PxRigidActorExt::createExclusiveShape(*sharedData.m_RigidDynamic, PxCapsuleGeometry(radius, halfHeight), *m_DefaultMaterial);

		// making the capsule stand upright by default
		// thank you nick!!!
		const glm::quat localRotQuat(glm::vec3(0, 0, PI / 2));
		const PxQuat pxLocalRotQuat(localRotQuat.x, localRotQuat.y, localRotQuat.z, localRotQuat.w);
		capsuleShape->setLocalPose(PxTransform(pxLocalRotQuat));

		// if no rigidbody, turn the gravity off so that these colliders won't 'fall'
		if (!(sharedData.m_AttachedComponents & PhysicsComponentTypes::Rigidbody))
		{
			sharedData.m_RigidDynamic->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);

			// so that colliders without rigidbodies will stay put when hit
			// sharedData.m_RigidDynamic->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, true);

			// SO TEMPORARY
			sharedData.m_RigidDynamic->is<PxRigidDynamic>()->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, true);
		}
		else
		{
			// if there is a rigidbody, we gotta recalculate stuff because we just added a shape (?)
			// WAIT YES THAT'S ACTUALLY IT YATTA!!!
			// PxRigidBodyExt::updateMassAndInertia(*sharedData.m_RigidDynamic, 1.0f);

			// SO TEMPORARY
			PxRigidBodyExt::updateMassAndInertia(*(sharedData.m_RigidDynamic->is<PxRigidDynamic>()), 1.0f);
		}

		sharedData.m_AttachedComponents |= PhysicsComponentTypes::CapsuleCollider;

		CapsuleCollider& capsuleCollider = entity->GetComponent<CapsuleCollider>();
		// TODO: assign more data here
		// capsuleCollider.m_IsTrigger = ...
		capsuleCollider.m_Offset = offset;
		capsuleCollider.m_Radius = radius;

		return capsuleCollider.m_IsInitialized = true;
	}

	void PhysicsSystem::ResizeCapsuleCollider(const Entity& entity, const float newRadius, const float newHalfHeight) const
	{
		PhysicsComponentAssertion(CapsuleCollider);

		// SO TEMPORARY
		PxRigidDynamic* rigidDynamic = m_Actors[entity->GetGUID()].m_RigidDynamic->is<PxRigidDynamic>();

		unsigned nbShapes = rigidDynamic->getNbShapes();
		const std::unique_ptr<PxShape* []> shapes(new PxShape * [nbShapes]); // I hate that I have to do this...
		nbShapes = rigidDynamic->getShapes(shapes.get(), nbShapes);

		for (unsigned i = 0; i < nbShapes; ++i)
		{
			if (shapes[i]->getGeometryType() != PxGeometryType::eCAPSULE) continue;

			shapes[i]->setGeometry(PxCapsuleGeometry(fabs(newRadius), fabs(newHalfHeight))); break;
		}

		entity->GetComponent<CapsuleCollider>().m_Radius = fabs(newRadius);
		entity->GetComponent<CapsuleCollider>().m_HalfHeight = fabs(newHalfHeight);
	}

	void PhysicsSystem::UpdateCapsuleCollider(const Entity& entity) const
	{
		PhysicsComponentAssertion(CapsuleCollider);

		CapsuleCollider& capsuleCollider = entity->GetComponent<CapsuleCollider>();

		capsuleCollider.m_IsInitialized || ConstructCapsuleCollider(entity);

		// PxRigidDynamic*& rigidDynamic = m_Actors[entity->GetGUID()].m_RigidDynamic;

		// SO TEMPORARY
		//PxRigidDynamic* rigidDynamic = m_Actors[entity->GetGUID()].m_RigidDynamic->is<PxRigidDynamic>();

		//// capsuleCollider.m_IsTrigger = rigidDynamic->getSomeFlags().isSet(/*whatever the heck is used for triggers*/)

		//capsuleCollider.m_Offset = VEC3_CAST(glm::vec3, rigidDynamic->getGlobalPose().p) - entity->GetComponent<Transform>().m_Position;

		//unsigned nbShapes = rigidDynamic->getNbShapes();
		//const std::unique_ptr<PxShape* []> shapes(new PxShape * [nbShapes]); // I hate that I have to do this...
		//nbShapes = rigidDynamic->getShapes(shapes.get(), nbShapes);

		//// obtain the index of the capsule shape
		//unsigned i = 0;
		//for (; i < nbShapes; ++i)
		//{
		//	if (shapes[i]->getGeometryType() != PxGeometryType::eCAPSULE) continue;

		//	break;
		//}

		//PxCapsuleGeometry capsuleGeometry;
		//shapes[i]->getCapsuleGeometry(capsuleGeometry);
		//capsuleCollider.m_Radius = capsuleGeometry.radius;
	}

	void PhysicsSystem::DestructCapsuleCollider(const Entity& entity) const
	{
		SharedData& sharedData = m_Actors[entity->GetGUID()];

		// reset bit for this component
		sharedData.m_AttachedComponents &= ~PhysicsComponentTypes::CapsuleCollider;

		if (!sharedData.m_AttachedComponents)
		{
			m_Scene->removeActor(*sharedData.m_RigidDynamic);
			sharedData.m_RigidDynamic->release();
			sharedData.m_MarkForRemoval = true;
		}
		else // there's still more attached physics components
		{
			unsigned nbShapes = sharedData.m_RigidDynamic->getNbShapes();
			const std::unique_ptr<PxShape* []> shapes(new PxShape * [nbShapes]); // I hate that I have to do this...
			nbShapes = sharedData.m_RigidDynamic->getShapes(shapes.get(), nbShapes);

			for (unsigned i = 0; i < nbShapes; ++i)
			{
				if (shapes[i]->getGeometryType() != PxGeometryType::eCAPSULE) continue;

				// there should only be ONE of each physics component, so it's safe to stop looping here
				sharedData.m_RigidDynamic->detachShape(*shapes[i]); break;
			}

			// PxRigidBodyExt::updateMassAndInertia(*sharedData.m_RigidDynamic, 1.0);

			// SO TEMPORARY
			PxRigidBodyExt::updateMassAndInertia(*(sharedData.m_RigidDynamic->is<PxRigidDynamic>()), 1.0);
		}
		entity->RemoveComponent<CapsuleCollider>();
	}
}
