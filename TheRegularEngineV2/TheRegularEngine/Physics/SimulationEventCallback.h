#pragma once
#include <vector>
#include "PhysX/PxPhysicsAPI.h"

#define UNUSED_VALUE(value) static_cast<void>(value) // (void)value

namespace TRE
{
	typedef struct HistoryEntryEnum
	{
		enum Enum : unsigned char
		{
			Enter = 1 << 0,
			Stay  = 1 << 1,
			Exit  = 1 << 2
		};
	} CollisionHistoryEntryEnum, TriggerHistoryEntryEnum;

	typedef struct HistoryEntry
	{
		unsigned m_First : 7, m_Second : 7, m_Flags : 3;
		// I wonder if these bitfield lengths need to be bigger...
	} CollisionHistoryEntry, TriggerHistoryEntry;

	class SimulationEventCallback : public physx::PxSimulationEventCallback
	{
	public:
		void onAdvance(const physx::PxRigidBody* const* bodyBuffer, const physx::PxTransform* poseBuffer, const physx::PxU32 count) override;
		void onConstraintBreak(physx::PxConstraintInfo* constraints, physx::PxU32 count) override;
		void onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs) override;
		void onSleep(physx::PxActor** actors, physx::PxU32 count) override;
		void onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count) override;
		void onWake(physx::PxActor** actors, physx::PxU32 count) override;

		// keeps track of IsCollisionEnter, IsCollisionStay and IsCollisionExit 'results'
		std::vector<CollisionHistoryEntry> m_CollisionHistory;

		// keeps track of IsTriggerEnter, IsTriggerStay and IsTriggerExit 'results'
		std::vector<TriggerHistoryEntry> m_TriggerHistory, m_PrevTriggerHistory;
		// why can't physx just handle this for me? :_)
	};
}
