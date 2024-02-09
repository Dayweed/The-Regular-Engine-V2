#include "pch.h"
#include "SimulationEventCallback.h"
#include "Core/Logger.h"

using namespace physx;
// to save my dwindling sanity

namespace TRE
{
	void SimulationEventCallback::onAdvance(const PxRigidBody* const* bodyBuffer, const PxTransform* poseBuffer, const PxU32 count)
	{
		UNUSED_VALUE(bodyBuffer); UNUSED_VALUE(poseBuffer); UNUSED_VALUE(count);
	}

	void SimulationEventCallback::onConstraintBreak(PxConstraintInfo* constraints, PxU32 count)
	{
		UNUSED_VALUE(constraints); UNUSED_VALUE(count);
	}

	void SimulationEventCallback::onContact(const PxContactPairHeader& pairHeader, const PxContactPair* pairs, PxU32 nbPairs)
	{
		if (!nbPairs) return;

		for (unsigned i = 0; i < nbPairs; ++i)
		{
			const PxContactPair& pair = pairs[i];

			// I sure hope these are equivalent!!!
			// pairHeader.flags & PxContactPairHeaderFlag::eREMOVED_ACTOR_0;
			// pairs[0].flags & PxContactPairFlag::eREMOVED_SHAPE_0;
			if (pairHeader.flags & (PxContactPairHeaderFlag::eREMOVED_ACTOR_0 | PxContactPairHeaderFlag::eREMOVED_ACTOR_1))
				continue;

			unsigned actor0Index = pairHeader.actors[0]->is<PxRigidActor>()->getInternalActorIndex();
			unsigned actor1Index = pairHeader.actors[1]->is<PxRigidActor>()->getInternalActorIndex();
			unsigned flags = 0;

			// ensure that actor0Index is lesser than (<) actor1Index 
			if (actor0Index > actor1Index) std::swap(actor0Index, actor1Index);

			if (pair.flags & PxContactPairFlag::eACTOR_PAIR_HAS_FIRST_TOUCH)
				flags |= CollisionHistoryEntryEnum::Enter;

			if (pair.flags & PxContactPairFlag::eACTOR_PAIR_LOST_TOUCH)
				flags |= CollisionHistoryEntryEnum::Exit;

			// if (!(pair.flags & (PxContactPairFlag::eACTOR_PAIR_HAS_FIRST_TOUCH | PxContactPairFlag::eACTOR_PAIR_LOST_TOUCH)))  // trust...right?
			if (pair.events & PxPairFlag::eNOTIFY_TOUCH_PERSISTS)
				flags |= CollisionHistoryEntryEnum::Stay;

			if (actor0Index >= (1ULL << MAX_ENTITIES_BIT) || actor1Index >= (1ULL << MAX_ENTITIES_BIT))
			{
				std::cout << "[SimulationEventCallback] There are more actors/entities than allocated memory, increase MAX_ENTITIES_BIT to change HistoryEntry::m_First and HistoryEntry::m_Second!\n";
				TRE_ERROR("[SimulationEventCallback] There are more actors/entities than allocated memory, increase MAX_ENTITIES_BIT to change HistoryEntry::m_First and HistoryEntry::m_Second!");
				assert(false && "Refer to Error above");
			}

			m_CollisionHistory.emplace_back(actor0Index, actor1Index, flags);
		}
	}

	void SimulationEventCallback::onSleep(PxActor** actors, PxU32 count)
	{
		UNUSED_VALUE(actors); UNUSED_VALUE(count);
	}

	void SimulationEventCallback::onTrigger(PxTriggerPair* pairs, PxU32 count)
	{
		if (!count) return;

		for (unsigned i = 0; i < count; ++i)
		{
			const PxTriggerPair& pair = pairs[i];

			// OH. MY. GOD. WTF.
			if (pair.flags & (PxTriggerPairFlag::eREMOVED_SHAPE_TRIGGER | PxTriggerPairFlag::eREMOVED_SHAPE_OTHER))
				continue;

			assert(pair.triggerActor->is<PxRigidActor>());
			assert(pair.otherActor->is<PxRigidActor>());

			unsigned actor0Index = pair.triggerActor->is<PxRigidDynamic>()->getInternalActorIndex();
			unsigned actor1Index = pair.otherActor->is<PxRigidDynamic>()->getInternalActorIndex();
			unsigned flags = 0;

			// ensure that actor0Index is lesser than (<) actor1Index 
			if (actor0Index > actor1Index) std::swap(actor0Index, actor1Index);

			if (pair.status & PxPairFlag::eNOTIFY_TOUCH_FOUND)
				flags |= TriggerHistoryEntryEnum::Enter;

			if (pair.status & PxPairFlag::eNOTIFY_TOUCH_LOST)
				flags |= TriggerHistoryEntryEnum::Exit;

			if (actor0Index >= (1ULL << MAX_ENTITIES_BIT) || actor1Index >= (1ULL << MAX_ENTITIES_BIT))
			{
				std::cout << "[SimulationEventCallback] There are more actors/entities than allocated memory, increase MAX_ENTITIES_BIT to change HistoryEntry::m_First and HistoryEntry::m_Second!\n";
				TRE_ERROR("[SimulationEventCallback] There are more actors/entities than allocated memory, increase MAX_ENTITIES_BIT to change HistoryEntry::m_First and HistoryEntry::m_Second!");
				assert(false && "Refer to Error above");
			}

			// Because of https://nvidia-omniverse.github.io/PhysX/physx/5.1.3/_build/physx/latest/struct_px_pair_flag.html?highlight=enotify_touch_persists#_CPPv4N10PxPairFlag4Enum22eNOTIFY_TOUCH_PERSISTSE,
			// IsTriggerStay needs to use the results of eNOTIFY_TOUCH_FOUND and eNOTIFY_TOUCH_LOST, which is done in GameUpdate().
			m_TriggerHistory.emplace_back(actor0Index, actor1Index, flags);
		}
	}

	void SimulationEventCallback::onWake(PxActor** actors, PxU32 count)
	{
		UNUSED_VALUE(actors); UNUSED_VALUE(count);
	}
}

