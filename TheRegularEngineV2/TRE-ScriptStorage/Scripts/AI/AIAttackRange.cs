using System;
using GlmSharp;

namespace TRE
{
	using PS = PhysicsSystem;
	// Controls a pawn, if any target is within range, assign target and tell pawn to chase it if it can
	public class AIAttackRange : Entity
	{
		public Entity mTarget;

		public void Start()
		{

		}

		public void Update()
		{
			
		}

		private void OnTriggerEnter(/*Collider*/System.UInt64 otherID)
		{
            mTarget = new Entity(otherID);
        }

		private void OnTriggerStay(/*Collider*/System.UInt64 otherID)
        {
            mTarget = new Entity(otherID);
        }

		private void OnTriggerExit(/*Collider*/System.UInt64 otherID)
		{
			if (mTarget.ID == otherID) mTarget = null;

        }
	}
}