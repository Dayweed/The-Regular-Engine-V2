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
			// WriteMemberToJSON(m_IsActive),
			WriteMemberToJSON(m_IsTrigger),
			// WriteMemberToJSON(m_Layer),
			WriteVec3MemberToJSON(m_Offset),
			WriteVec3MemberToJSON(m_HalfExtents),
		};
	}

	void from_json(const nlohmann::json& j, BoxCollider& t)
	{
		// ReadMemberFromJSON(m_IsActive);
		ReadMemberFromJSON(m_IsTrigger);
		// ReadMemberFromJSON(m_Layer);
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
			// PxSetGroup(*tempSharedData.m_RigidDynamic, 0);

#ifdef _DEBUG
			tempSharedData.m_RigidDynamic->setName("BoxCollider");
#endif
			m_Scene->addActor(*tempSharedData.m_RigidDynamic);

			tempSharedData.m_GUID = entity->GetGUID();

			m_Actors[entity->GetGUID()] = tempSharedData;
		}

		SharedData& sharedData = m_Actors[entity->GetGUID()];
		BoxCollider& boxCollider = entity->GetComponent<BoxCollider>();
		if (boxCollider.m_IsTrigger)
			PxRigidActorExt::createExclusiveShape(*sharedData.m_RigidDynamic, PxBoxGeometry(VEC3_CAST(PxVec3, halfExtents)), *m_DefaultMaterial,
				PxShapeFlag::eVISUALIZATION | PxShapeFlag::eSCENE_QUERY_SHAPE | PxShapeFlag::eTRIGGER_SHAPE);
		else
			PxRigidActorExt::createExclusiveShape(*sharedData.m_RigidDynamic, PxBoxGeometry(VEC3_CAST(PxVec3, halfExtents)), *m_DefaultMaterial,
				PxShapeFlag::eVISUALIZATION | PxShapeFlag::eSCENE_QUERY_SHAPE | PxShapeFlag::eSIMULATION_SHAPE);

		// if no rigidbody, turn the gravity off so that these colliders won't 'fall'
		if (!(sharedData.m_AttachedComponents & PhysicsComponentTypes::Rigidbody))
		{
			sharedData.m_RigidDynamic->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);

			// so that colliders without rigidbodies will stay put when hit
			sharedData.m_RigidDynamic->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, true);
		}
		else
		{
			// if there is a rigidbody, we gotta recalculate stuff because we just added a shape (?)
			// WAIT YES THAT'S ACTUALLY IT YATTA!!!
			PxRigidBodyExt::updateMassAndInertia(*sharedData.m_RigidDynamic, 1.0f);
			sharedData.m_RigidDynamic->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, !entity->GetComponent<Rigidbody>().m_UseGravity);
		}

		sharedData.m_AttachedComponents |= PhysicsComponentTypes::BoxCollider;
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
		const std::unique_ptr<PxShape* []> shapes(new PxShape * [nbShapes]); // I hate that I have to do this...
		nbShapes = rigidDynamic->getShapes(shapes.get(), nbShapes);

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

		//if (boxCollider.m_IsDirty)
		//{
		//	// set the new layer(determined by the string) here
		//	const int newLayer = GetLayerFromLayerString(boxCollider.m_LayerString);
		//	SetLayer(entity, newLayer);
		//}

		//boxCollider.m_IsDirty = false;
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
			const std::unique_ptr<PxShape* []> shapes(new PxShape * [nbShapes]); // I hate that I have to do this...
			nbShapes = sharedData.m_RigidDynamic->getShapes(shapes.get(), nbShapes);

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
