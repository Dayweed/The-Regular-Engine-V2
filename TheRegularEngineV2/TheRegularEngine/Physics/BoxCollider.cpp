#include "pch.h"
#include "BoxCollider.h"
#include "PhysicsSystem.h"
#include "Core/Transform.h"

using namespace physx;
// to save my dwindling sanity

namespace TRE
{
	void to_json(nlohmann::json& j, const BoxCollider& t)
	{
		j = nlohmann::json{
			WriteMemberToJSON(m_IsActive),
			WriteMemberToJSON(m_IsTrigger),
			WriteMemberToJSON(m_CollisionLayer.m_LayerID),
			WriteMemberToJSON(m_PhysicsMaterial.m_MaterialID),
			WriteVec3MemberToJSON(m_Offset),
			WriteVec3MemberToJSON(m_HalfExtents),
		};
	}

	void from_json(const nlohmann::json& j, BoxCollider& t)
	{
		ReadMemberFromJSON(m_IsActive);
		ReadMemberFromJSON(m_IsTrigger);
		ReadMemberFromJSON(m_CollisionLayer.m_LayerID);
		ReadMemberFromJSON(m_PhysicsMaterial.m_MaterialID);
		ReadVec3MemberFromJSON(m_Offset);
		ReadVec3MemberFromJSON(m_HalfExtents);
	}

	bool PhysicsSystem::ConstructBoxCollider(const Entity& entity, const glm::vec3& halfExtents, const glm::vec3& offset) const
	{
		PhysicsComponentConstructorAssertion(BoxCollider);

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
			{
				char* string = nullptr;
				if (entity->GetName() == "Holey")
					string = (char*)"Holey";
				else if (entity->GetName() == "Moley")
					string = (char*)"Moley";
				else if (entity->GetName() == "Slippery Body")
					string = (char*)"Slippery Body";
				else
					string = (char*)"BoxCollider";

				tempSharedData.m_RigidDynamic->setName(string);
			}
#endif
			m_Scene->addActor(*tempSharedData.m_RigidDynamic);

			tempSharedData.m_GUID = entity->GetGUID();

			m_Actors[entity->GetGUID()] = tempSharedData;
		}

		auto& [rigidDynamic, attachedComponents, GUID, _unused] = m_Actors[entity->GetGUID()];
		BoxCollider& boxCollider = entity->GetComponent<BoxCollider>();

		// determine physics material being used
		PxMaterial* shapeMaterial = nullptr;
		if (boxCollider.m_PhysicsMaterial.m_MaterialID == PhysicsMaterial::Default)
			shapeMaterial = m_DefaultMaterial;
		else if (boxCollider.m_PhysicsMaterial.m_MaterialID == PhysicsMaterial::Frictionless)
			shapeMaterial = m_FrictionlessMaterial;

		if (boxCollider.m_IsTrigger)
			PxRigidActorExt::createExclusiveShape(*rigidDynamic, PxBoxGeometry(VEC3_CAST(PxVec3, halfExtents)), *shapeMaterial,
				PxShapeFlag::eVISUALIZATION | PxShapeFlag::eSCENE_QUERY_SHAPE | PxShapeFlag::eTRIGGER_SHAPE);
		else
			PxRigidActorExt::createExclusiveShape(*rigidDynamic, PxBoxGeometry(VEC3_CAST(PxVec3, halfExtents)), *shapeMaterial,
				PxShapeFlag::eVISUALIZATION | PxShapeFlag::eSCENE_QUERY_SHAPE | PxShapeFlag::eSIMULATION_SHAPE);

		PxSetGroup(*rigidDynamic, static_cast<PxU16>(boxCollider.m_CollisionLayer.m_LayerID));

		// if no rigidbody, turn the gravity off so that these colliders won't 'fall'
		if (!(attachedComponents & PhysicsComponentTypes::Rigidbody))
		{
			rigidDynamic->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);

			// so that colliders without rigidbodies will stay put when hit
			rigidDynamic->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, true);
		}
		else
		{
			// if there is a rigidbody, we gotta recalculate stuff because we just added a shape (?)
			// WAIT YES THAT'S ACTUALLY IT YATTA!!!
			PxRigidBodyExt::updateMassAndInertia(*rigidDynamic, 1.0f);
			rigidDynamic->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, !entity->GetComponent<Rigidbody>().m_UseGravity);
		}

		attachedComponents |= PhysicsComponentTypes::BoxCollider;
		// assert(entity->GetGUID() == sharedData.m_GUID);
		boxCollider.m_Offset = offset;
		boxCollider.m_HalfExtents = halfExtents;
		boxCollider.m_IsVisible = m_DrawDebugLines;

		return boxCollider.m_IsInitialized = true;
	}

	void PhysicsSystem::ResizeBoxCollider(const Entity& entity, const glm::vec3& newHalfExtents) const
	{
		PhysicsComponentAssertion(BoxCollider);

		PxRigidDynamic*& rigidDynamic = m_Actors[entity->GetGUID()].m_RigidDynamic;

		unsigned nbShapes = rigidDynamic->getNbShapes();
		PxShape* shapes[PhysicsComponentTypes::TOTAL - 1] = { nullptr };
		nbShapes = rigidDynamic->getShapes(shapes, nbShapes);

		for (unsigned i = 0; i < nbShapes; ++i)
		{
			if (shapes[i]->getGeometryType() != PxGeometryType::eBOX) continue;

			shapes[i]->setGeometry(PxBoxGeometry(
				fabs(newHalfExtents.x),
				fabs(newHalfExtents.y),
				fabs(newHalfExtents.z)));
			break;
		}

		entity->GetComponent<BoxCollider>().m_HalfExtents = glm::vec3(fabs(newHalfExtents.x), fabs(newHalfExtents.y), fabs(newHalfExtents.z));
	}

	void PhysicsSystem::UpdateBoxCollider(const Entity& entity) const
	{
		PhysicsComponentAssertion(BoxCollider);

		BoxCollider& boxCollider = entity->GetComponent<BoxCollider>();

		boxCollider.m_IsInitialized || ConstructBoxCollider(entity);

		UpdateActorPose(entity, boxCollider.m_Offset);

		SetBoxColliderTrigger(entity, boxCollider.m_IsTrigger);

		if (boxCollider.m_IsDirty)
		{
			ChangeCollisionLayer(entity);
			ChangeIsActive(entity);
			ChangeMaterial(entity);
		}
	}

	void PhysicsSystem::DestructBoxCollider(const Entity& entity) const
	{
		SharedData& sharedData = m_Actors[entity->GetGUID()];

		// reset bit for this component
		sharedData.m_AttachedComponents &= ~PhysicsComponentTypes::BoxCollider;

		if (!sharedData.m_AttachedComponents)
		{
			m_Scene->removeActor(*sharedData.m_RigidDynamic);
			sharedData.m_RigidDynamic->release();
			sharedData.m_MarkForRemoval = true;
		}
		else // there's still more attached physics components
		{
			unsigned nbShapes = sharedData.m_RigidDynamic->getNbShapes();
			PxShape* shapes[PhysicsComponentTypes::TOTAL - 1] = { nullptr };
			nbShapes = sharedData.m_RigidDynamic->getShapes(shapes, nbShapes);

			for (unsigned i = 0; i < nbShapes; ++i)
			{
				if (shapes[i]->getGeometryType() != PxGeometryType::eBOX) continue;

				// there should only be ONE of each physics component, so it's safe to stop looping here
				sharedData.m_RigidDynamic->detachShape(*shapes[i]); break;
			}

			PxRigidBodyExt::updateMassAndInertia(*sharedData.m_RigidDynamic, 1.0);
		}
		//entity->RemoveComponent<BoxCollider>();
	}

	void PhysicsSystem::SetBoxColliderTrigger(const Entity& entity, const bool isTrigger) const
	{
		BoxCollider& boxCollider = entity->GetComponent<BoxCollider>();
		PxRigidDynamic* rigidDynamic = m_Actors[entity->GetGUID()].m_RigidDynamic;

		boxCollider.m_IsTrigger = isTrigger;

		constexpr unsigned maxNbShapes = 3; // sphere, box, capsule
		PxShape* shapes[maxNbShapes] = { nullptr };
		rigidDynamic->getShapes(shapes, maxNbShapes);

		// obtain the index of the box shape
		for (auto& shape : shapes)
		{
			if (!shape)
				continue;

			if (shape->getGeometryType() != PxGeometryType::eBOX)
				continue;

			if (boxCollider.m_IsTrigger)
			{
				shape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, false);
				shape->setFlag(PxShapeFlag::eTRIGGER_SHAPE, true);
			}
			else
			{
				shape->setFlag(PxShapeFlag::eTRIGGER_SHAPE, false);
				shape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, true);
			}
			break;
		}
	}
}
