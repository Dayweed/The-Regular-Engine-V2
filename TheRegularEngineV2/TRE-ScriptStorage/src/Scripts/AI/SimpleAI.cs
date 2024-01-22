using System;
using System.Collections.Generic;
using System.Reflection;
using System.Runtime.CompilerServices;
using static System.Runtime.CompilerServices.RuntimeHelpers;
using System.Threading;
using GlmSharp;

namespace TRE
{
	// Controls a pawn, if any target is within range, assign target and tell pawn to chase it if it can
	public class SimpleAI : Entity
	{
		// Colliders to keep eye on
        public Entity mDetector;
        //public Entity mPredictor;
        public Entity mGround;
        private vec3 lastValidPosition;

		// Target Variables
		public Entity mHoley;
		public Entity mMoley;
		public Entity mTarget;
		public bool mFoundTarget = false;
		public bool mCanChaseTarget = false;
        private float maxDistFromSpawn = 50f;

        public vec3 originalSpawnPoint = new vec3();
        public vec3 defaultVector = new vec3(1, 0, 0);
        public vec3 moveVector = new vec3(1, 0, 0);
        private float moveSpeed = 50.0f;

        public void Start()
		{
            originalSpawnPoint = transform.Position;
            mHoley = new Entity(ECSManager.FindIDFromName("Holey"));
            mMoley = new Entity(ECSManager.FindIDFromName("Moley"));
            mTarget = new Entity(); // Invalid ID
            mGround = new Entity(); // Invalid ID
            mDetector = parenting.GetChildFromName("Detector");
            //mPredictor = parenting.GetChildFromName("Predictor");
        }

		public void Update()
		{
            transform.Rotation = new vec3(0, transform.Rotation.y, 0);

            // Check if foundTarget
            if (!mFoundTarget)
            {
                // Check if collide with moley or holey
                if (PhysicsSystem.IsTriggerStay(mDetector.ID, mHoley.ID))
                {
                    mTarget = mHoley;
                    mFoundTarget = true;
                }
                else if (PhysicsSystem.IsTriggerStay(mDetector.ID, mMoley.ID))
                {
                    mTarget = mMoley;
                    mFoundTarget = true;
                }
            }
            // Check if target is out of detect range
            else
            {
                // Check if collide with moley or holey / out of range
                float distanceFromSpawn = (transform.Position - mTarget.transform.Position).Length;
                if (PhysicsSystem.IsTriggerExit(mDetector.ID, mTarget.ID) || distanceFromSpawn > maxDistFromSpawn)
                {
                    transform.Rotation = new vec3 (transform.Rotation.x, 0, transform.Rotation.z);
                    PhysicsSystem.SetLinearVelocity(ID, vec3.Zero);
                    mFoundTarget = false;
                }
            }

            // Check if player is within detect sphere
            if (mFoundTarget)
			{
                // Rotate moveVector based on angle
                moveVector = mTarget.transform.Position - transform.Position;
                // Ignore y-axis
                PhysicsSystem.GetLinearVelocity(ID, out vec3 currVelocity);
                moveVector = new vec3(moveVector.x, 0, moveVector.z);
                moveVector = moveVector.NormalizedSafe;
                vec3 moveDir = moveVector * moveSpeed * Time.deltaTime;

                // Determine if it can chase the target if it move that direction
                PhysicsSystem.SetLinearVelocity(ID, currVelocity + moveDir);


                //mPredictor.transform.Position = transform.Position + moveDir;
                //if (PhysicsSystem.IsTriggerEnter(mPredictor.ID, mGround.ID))
                //{
                //    // Chase Target
                //    Debug.Log("CAHSE");
                //    PhysicsSystem.SetLinearVelocity(ID, moveDir);
                //}
                //else
                //{
                //    mCanChaseTarget = false;
                //    PhysicsSystem.SetLinearVelocity(ID, vec3.Zero);
                //}
            }
		}

        private void OnCollisionEnter(/*Collider*/System.UInt64 otherID)
        {
            // Assign ground if valid
            Entity other = new Entity(otherID);
            if (mGround.ID == 0 && other.CompareTag("Ground"))
            {
                mGround = other;
                mCanChaseTarget = true;
            }
            return;
            if (mGround.ID != 0)
            {
                lastValidPosition = transform.Position;
            }
        }

        private void OnCollisionStay(/*Collider*/System.UInt64 otherID)
        {
            // Assign ground if valid
            if (mGround.ID == otherID)
            {
                //lastValidPosition = transform.Position;
                mCanChaseTarget = true;
            }
        }

        private void OnCollisionExit(System.UInt64 otherID)
        {
            return;
            // Make sure it cannot chase anymore if leave ground
            if (mGround.ID == otherID)
            {
                mCanChaseTarget = false;
                transform.Position = lastValidPosition;
                transform.Rotation = new vec3(transform.Rotation.x, 0, transform.Rotation.z);
                PhysicsSystem.SetLinearVelocity(ID, vec3.Zero);
            }
        }

        bool PtAABB(vec2 pt, vec2 aabbMin, vec2 aabbMax)
        {
            bool withinX = aabbMin.x < pt.x && pt.x < aabbMax.x;
            bool withinY = aabbMin.y < pt.y && pt.y < aabbMax.y;
            return withinX && withinY;
        }
    }
}