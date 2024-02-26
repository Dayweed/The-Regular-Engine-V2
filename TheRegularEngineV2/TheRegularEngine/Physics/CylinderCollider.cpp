#include "pch.h"
#include "ECS/Components/CylinderCollider.h"
#include "PhysicsSystem.h"
#include "ECS/Components/Transform.h"

using namespace physx;
// to save my dwindling sanity

namespace TRE
{
	PxConvexMesh* PhysicsSystem::CreateCylinderMesh(const float radius, const float height) const
	{
		constexpr int iterations = 24;
		// constexpr float radius = 0.5f;
		// constexpr float halfHeight = 0.5f;
		const float halfHeight = 0.5f * height;
		// the multiple of PI used for each iteration
		constexpr float angleStep = 2.0f / iterations;
		std::vector<PxVec3> vertices;
		vertices.reserve((iterations + 1) * 2);

		for (int i = 0; i < iterations; ++i)
		{
			float x = radius * cos(i * angleStep * PI);
			float y = halfHeight;
			float z = radius * sin(i * angleStep * PI);
			vertices.emplace_back(x, y, z);
			vertices.emplace_back(x, -y, z);
		}

		// add back first vertices as ending points (?)
		vertices.emplace_back(radius, halfHeight, 0.0f);
		vertices.emplace_back(radius, -halfHeight, 0.0f);

		PxConvexMeshDesc convexDesc;
		convexDesc.points.count = static_cast<PxU32>(vertices.size());
		convexDesc.points.data = vertices.data();
		convexDesc.points.stride = static_cast<PxU32>(sizeof(PxVec3));
		convexDesc.flags = PxConvexFlag::eCOMPUTE_CONVEX;

		const PxCookingParams params(m_Physics->getTolerancesScale());
		PxDefaultMemoryOutputStream buf;
		PxConvexMeshCookingResult::Enum result;
		PxCookConvexMesh(params, convexDesc, buf, &result);

		PxDefaultMemoryInputData input(buf.getData(), buf.getSize());
		return m_Physics->createConvexMesh(input);
	}

	bool PhysicsSystem::ConstructCylinderCollider(const Entity& entity, const float radius, const float height, const glm::vec3& offset) const
	{
		PhysicsComponentConstructorAssertion(CylinderCollider);

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
					string = (char*)"CylinderCollider";

				tempSharedData.m_RigidDynamic->setName(string);
			}
#endif
			m_Scene->addActor(*tempSharedData.m_RigidDynamic);

			tempSharedData.m_GUID = entity->GetGUID();

			m_Actors[entity->GetGUID()] = tempSharedData;
		}

		auto& [rigidDynamic, attachedComponents, GUID, _unused] = m_Actors[entity->GetGUID()];

		CylinderCollider& cylinderCollider = entity->GetComponent<CylinderCollider>();

		// determine physics material being used
		PxMaterial* shapeMaterial = m_DefaultMaterial;
		if (cylinderCollider.m_PhysicsMaterial.m_MaterialID == PhysicsMaterial::Default)
			shapeMaterial = m_DefaultMaterial;
		else if (cylinderCollider.m_PhysicsMaterial.m_MaterialID == PhysicsMaterial::Frictionless)
			shapeMaterial = m_FrictionlessMaterial;

		if (cylinderCollider.m_IsTrigger)
			PxRigidActorExt::createExclusiveShape(*rigidDynamic, PxConvexMeshGeometry(CreateCylinderMesh(radius, height)), *shapeMaterial,
				PxShapeFlag::eVISUALIZATION | PxShapeFlag::eSCENE_QUERY_SHAPE | PxShapeFlag::eTRIGGER_SHAPE);
		else
			PxRigidActorExt::createExclusiveShape(*rigidDynamic, PxConvexMeshGeometry(CreateCylinderMesh(radius, height)), *shapeMaterial,
				PxShapeFlag::eVISUALIZATION | PxShapeFlag::eSCENE_QUERY_SHAPE | PxShapeFlag::eSIMULATION_SHAPE);

		PxSetGroup(*rigidDynamic, static_cast<PxU16>(cylinderCollider.m_CollisionLayer.m_LayerID));

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

		attachedComponents |= PhysicsComponentTypes::CylinderCollider;

		cylinderCollider.m_Offset = offset;
		cylinderCollider.m_Radius = radius;
		cylinderCollider.m_IsVisible = m_DrawDebugLines;

		return cylinderCollider.m_IsInitialized = true;
	}

	void PhysicsSystem::ResizeCylinderCollider(const Entity& entity, const float newRadius, const float newHeight) const
	{
		PhysicsComponentAssertion(CylinderCollider);

		PxRigidDynamic*& rigidDynamic = m_Actors[entity->GetGUID()].m_RigidDynamic;

		unsigned nbShapes = rigidDynamic->getNbShapes();
		PxShape* shapes[PhysicsComponentTypes::TOTAL - 1] = { nullptr };
		nbShapes = rigidDynamic->getShapes(shapes, nbShapes);

		for (unsigned i = 0; i < nbShapes; ++i)
		{
			if (!shapes[i] || shapes[i]->getGeometryType() != PxGeometryType::eCONVEXMESH)
				continue;

			shapes[i]->setGeometry(PxConvexMeshGeometry(CreateCylinderMesh(fabs(newRadius), fabs(newHeight))));
			break;
		}

		entity->GetComponent<CylinderCollider>().m_Radius = fabs(newRadius);
		entity->GetComponent<CylinderCollider>().m_Height = fabs(newHeight);
	}

	void PhysicsSystem::UpdateCylinderCollider(const Entity& entity) const
	{
		PhysicsComponentAssertion(CylinderCollider);

		CylinderCollider& cylinderCollider = entity->GetComponent<CylinderCollider>();

		cylinderCollider.m_IsInitialized || ConstructCylinderCollider(entity);

		UpdateActorPose(entity, cylinderCollider.m_Offset);

		PxRigidBodyExt::updateMassAndInertia(*m_Actors[entity->GetGUID()].m_RigidDynamic, 1.0f);

		SetCylinderColliderTrigger(entity, cylinderCollider.m_IsTrigger);

		if (cylinderCollider.m_IsDirty)
		{
			ChangeCollisionLayer(entity);
			ChangeIsActive(entity);
			ChangeMaterial(entity);
		}
	}

	void PhysicsSystem::DestructCylinderCollider(const Entity& entity) const
	{
		SharedData& sharedData = m_Actors[entity->GetGUID()];

		// reset bit for this component
		sharedData.m_AttachedComponents &= ~PhysicsComponentTypes::CylinderCollider;

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
				if (!shapes[i] || shapes[i]->getGeometryType() != PxGeometryType::eCONVEXMESH)
					continue;

				// there should only be ONE of each physics component, so it's safe to stop looping here
				sharedData.m_RigidDynamic->detachShape(*shapes[i]);
				break;
			}

			PxRigidBodyExt::updateMassAndInertia(*sharedData.m_RigidDynamic, 1.0);
		}
		//entity->RemoveComponent<CylinderCollider>();
	}

	void PhysicsSystem::SetCylinderColliderTrigger(const Entity& entity, const bool isTrigger) const
	{
		CylinderCollider& cylinderCollider = entity->GetComponent<CylinderCollider>();
		PxRigidDynamic* rigidDynamic = m_Actors[entity->GetGUID()].m_RigidDynamic->is<PxRigidDynamic>();

		cylinderCollider.m_IsTrigger = isTrigger;

		constexpr unsigned maxNbShapes = 4; // sphere, box, capsule, cylinder
		PxShape* shapes[maxNbShapes] = { nullptr };
		rigidDynamic->getShapes(shapes, maxNbShapes);

		// obtain the index of the box shape
		for (auto& shape : shapes)
		{
			if (!shape || shape->getGeometryType() != PxGeometryType::eCONVEXMESH)
				continue;

			if (cylinderCollider.m_IsTrigger)
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
