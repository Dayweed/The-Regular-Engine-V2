#include "pch.h"
#include "ECS/Components/SphereCollider.h"
#include "PhysicsSystem.h"
#include "ECS/Components/Transform.h"

using namespace physx;
// to save my dwindling sanity

namespace TRE
{
	bool PhysicsSystem::ConstructSphereCollider(const Entity& entity, const float radius, const glm::vec3& offset) const
	{
		PhysicsComponentConstructorAssertion(SphereCollider);

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
					string = (char*)"SphereCollider";

				tempSharedData.m_RigidDynamic->setName(string);
			}
#endif
			m_Scene->addActor(*tempSharedData.m_RigidDynamic);

			tempSharedData.m_GUID = entity->GetGUID();

			m_Actors[entity->GetGUID()] = tempSharedData;
		}

		auto& [rigidDynamic, attachedComponents, GUID, _unused] = m_Actors[entity->GetGUID()];
		SphereCollider& sphereCollider = entity->GetComponent<SphereCollider>();

		// determine physics material being used
		PxMaterial* shapeMaterial = m_DefaultMaterial;
		if (sphereCollider.m_PhysicsMaterial.m_MaterialID == PhysicsMaterial::Default)
			shapeMaterial = m_DefaultMaterial;
		else if (sphereCollider.m_PhysicsMaterial.m_MaterialID == PhysicsMaterial::Frictionless)
			shapeMaterial = m_FrictionlessMaterial;

		if (sphereCollider.m_IsTrigger)
			PxRigidActorExt::createExclusiveShape(*rigidDynamic, PxSphereGeometry(radius), *shapeMaterial,
				PxShapeFlag::eVISUALIZATION | PxShapeFlag::eSCENE_QUERY_SHAPE | PxShapeFlag::eTRIGGER_SHAPE);
		else
			PxRigidActorExt::createExclusiveShape(*rigidDynamic, PxSphereGeometry(radius), *shapeMaterial,
				PxShapeFlag::eVISUALIZATION | PxShapeFlag::eSCENE_QUERY_SHAPE | PxShapeFlag::eSIMULATION_SHAPE);

		PxSetGroup(*rigidDynamic, static_cast<PxU16>(sphereCollider.m_CollisionLayer.m_LayerID));

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

		attachedComponents |= PhysicsComponentTypes::SphereCollider;
		// assert(entity->GetGUID() == sharedData.m_GUID);

		// TODO: assign more data here
		// sphereCollider.m_IsTrigger = ...
		sphereCollider.m_Offset = offset;
		sphereCollider.m_Radius = radius;
		sphereCollider.m_IsVisible = m_DrawDebugLines;

		return sphereCollider.m_IsInitialized = true;
	}

	void PhysicsSystem::ResizeSphereCollider(const Entity& entity, const float newRadius) const
	{
		PhysicsComponentAssertion(SphereCollider);

		PxRigidDynamic*& rigidDynamic = m_Actors[entity->GetGUID()].m_RigidDynamic;

		unsigned nbShapes = rigidDynamic->getNbShapes();
		PxShape* shapes[PhysicsComponentTypes::TOTAL - 1] = { nullptr };
		nbShapes = rigidDynamic->getShapes(shapes, nbShapes);

		for (unsigned i = 0; i < nbShapes; ++i)
		{
			if (!shapes[i] || shapes[i]->getGeometryType() != PxGeometryType::eSPHERE)
				continue;

			shapes[i]->setGeometry(PxSphereGeometry(fabs(newRadius)));
			break;
		}

		entity->GetComponent<SphereCollider>().m_Radius = fabs(newRadius);
	}

	void PhysicsSystem::UpdateSphereCollider(const Entity& entity) const
	{
		PhysicsComponentAssertion(SphereCollider);

		SphereCollider& sphereCollider = entity->GetComponent<SphereCollider>();

		sphereCollider.m_IsInitialized || ConstructSphereCollider(entity);

		UpdateActorPose(entity, sphereCollider.m_Offset);

		SetSphereColliderTrigger(entity, sphereCollider.m_IsTrigger);

		if (sphereCollider.m_IsDirty)
		{
			ChangeCollisionLayer(entity);
			ChangeIsActive(entity);
			ChangeMaterial(entity);
		}
	}

	void PhysicsSystem::DestructSphereCollider(const Entity& entity) const
	{
		SharedData& sharedData = m_Actors[entity->GetGUID()];

		// reset bit for this component
		sharedData.m_AttachedComponents &= ~PhysicsComponentTypes::SphereCollider;

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
				if (!shapes[i] || shapes[i]->getGeometryType() != PxGeometryType::eSPHERE)
					continue;

				// there should only be ONE of each physics component, so it's safe to stop looping here
				sharedData.m_RigidDynamic->detachShape(*shapes[i]);
				break;
			}

			PxRigidBodyExt::updateMassAndInertia(*sharedData.m_RigidDynamic, 1.0);
		}
		//entity->RemoveComponent<SphereCollider>();
	}

	void PhysicsSystem::SetSphereColliderTrigger(const Entity& entity, const bool isTrigger) const
	{
		SphereCollider& sphereCollider = entity->GetComponent<SphereCollider>();
		PxRigidDynamic* rigidDynamic = m_Actors[entity->GetGUID()].m_RigidDynamic;

		sphereCollider.m_IsTrigger = isTrigger;

		constexpr unsigned maxNbShapes = 4; // sphere, box, capsule, cylinder
		PxShape* shapes[maxNbShapes] = { nullptr };
		rigidDynamic->getShapes(shapes, maxNbShapes);

		// obtain the index of the box shape
		for (auto& shape : shapes)
		{
			if (!shape || shape->getGeometryType() != PxGeometryType::eSPHERE)
				continue;

			if (sphereCollider.m_IsTrigger)
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
