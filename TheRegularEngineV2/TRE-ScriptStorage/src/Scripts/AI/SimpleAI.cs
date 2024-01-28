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
		public Entity mGround;

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
		private float padPredict = 10.0f;
		private float moveSpeed = 50.0f;
		private float maxVelocity = 50.0f;
		private bool isGrounded = false;

		public void Start()
		{
			originalSpawnPoint = transform.Position;
			mHoley = new Entity(ECSManager.FindIDFromName("Holey"));
			mMoley = new Entity(ECSManager.FindIDFromName("Moley"));
			mTarget = new Entity(); // Invalid ID
			mGround = new Entity(); // Invalid ID
			mDetector = parenting.GetChildFromName("Detector");
		}

		public void Update()
		{
			transform.Rotation = new vec3(0, transform.Rotation.y, 0);

			// Check if foundTarget
			if (!mFoundTarget || !mCanChaseTarget)
			{
				float distanceFromHoley = (transform.Position - mHoley.transform.Position).Length;
				float distanceFromMoley = (transform.Position - mMoley.transform.Position).Length;
				// Check if collide with moley or holey
				if (PhysicsSystem.IsTriggerStay(mDetector.ID, mHoley.ID))
				{
					mTarget = mHoley;
					mFoundTarget = true;
				}
				if (PhysicsSystem.IsTriggerStay(mDetector.ID, mMoley.ID) && distanceFromHoley > distanceFromMoley)
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
					PhysicsSystem.SetLinearVelocity(ID, vec3.Zero);
					mFoundTarget = false;
				}
			}

			// Check if player is within detect sphere
			if (mFoundTarget && mGround.ID != 0)
			{
				// Set moveVector based on angle
				moveVector = mTarget.transform.Position - transform.Position;

				// Ignore y-axis
				PhysicsSystem.GetLinearVelocity(ID, out vec3 currVelocity);
				moveVector = new vec3(moveVector.x, 0, moveVector.z);
				moveVector = moveVector.NormalizedSafe;
				vec3 moveDir = moveVector * moveSpeed * Time.deltaTime;

				// Rotate Character to look at target
				vec2 rotAxis = MathF.GetLookAtAxis(transform.Position, mTarget.transform.Position);
				transform.Rotation = new vec3(rotAxis.x, rotAxis.y, 0);

				// Determine if it can chase the target if it move that direction
				vec3 predictedPos = transform.Position + currVelocity + moveDir * padPredict;
				predictedPos = new vec3(predictedPos.x, mGround.transform.Position.y, predictedPos.z);

				BoxCollider grndCdr = mGround.GetComponent<BoxCollider>();
				vec3 min = new vec3(mGround.transform.Position.x - grndCdr.HalfExtents.x, mGround.transform.Position.y - grndCdr.HalfExtents.y, mGround.transform.Position.z - grndCdr.HalfExtents.z);
				vec3 max = new vec3(mGround.transform.Position.x + grndCdr.HalfExtents.x, mGround.transform.Position.y + grndCdr.HalfExtents.y, mGround.transform.Position.z + grndCdr.HalfExtents.z);

				if (PtAABB(predictedPos, min, max))
				{
					if (Math.Sqrt(currVelocity.x * currVelocity.x + currVelocity.z * currVelocity.z) >= maxVelocity)
					{
						currVelocity = moveDir * maxVelocity;
					}
					PhysicsSystem.SetLinearVelocity(ID, currVelocity + moveDir);

					// If it is grounded, jump up
					if (isGrounded)
					{
						PhysicsSystem.AddForce(ID, new vec3(0, 10f, 0), ForceMode.VelocityChange);
					}

					mCanChaseTarget = true;
				}
				else
				{
					PhysicsSystem.SetLinearVelocity(ID, vec3.Zero);
					mCanChaseTarget = false;
				}
			}
		}

		private void OnCollisionEnter(/*Collider*/System.UInt64 otherID)
		{
			// Assign ground if valid
			Entity other = new Entity(otherID);
			if (mGround.ID == 0 && other.CompareTag("Ground") && other.HasComponent<BoxCollider>())
			{
				mGround = other;
				mCanChaseTarget = true;
				isGrounded = true;
			}
			else if (mGround.ID == otherID && other.CompareTag("Ground") && other.HasComponent<BoxCollider>())
			{
				isGrounded = true;
			}
		}

		private void OnCollisionStay(/*Collider*/System.UInt64 otherID)
		{
			// Assign ground if valid
			Entity other = new Entity(otherID);
			if (other.CompareTag("Red") || other.CompareTag("RedCollider"))
			{
				mMoley.GetComponent<MoleyController>().TakeDamage();
			}
			else if (other.CompareTag("Blue") || other.CompareTag("BlueCollider"))
			{
				mHoley.GetComponent<HoleyController>().TakeDamage();
			}
		}

		private void OnCollisionExit(/*Collider*/System.UInt64 otherID)
		{
			// Assign ground if valid
			Entity other = new Entity(otherID);
			if (mGround.ID == otherID && other.CompareTag("Ground") && other.HasComponent<BoxCollider>())
			{
				isGrounded = false;
			}
		}

		bool PtAABB(vec3 pt, vec3 aabbMin, vec3 aabbMax)
		{
			bool withinX = aabbMin.x < pt.x && pt.x < aabbMax.x;
			bool withinY = aabbMin.y < pt.y && pt.y < aabbMax.y;
			bool withinZ = aabbMin.z < pt.z && pt.z < aabbMax.z;
			return withinX && withinY && withinZ;
		}
	}
}