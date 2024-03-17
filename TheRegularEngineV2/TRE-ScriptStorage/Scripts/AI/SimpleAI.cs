using System;
using GlmSharp;

namespace TRE
{
	using PS = PhysicsSystem;
	// Controls a pawn, if any target is within range, assign target and tell pawn to chase it if it can
	public class SimpleAI : Entity
	{
		// Colliders to keep eye on
		public Entity mDetectorRange;
		public Entity mAttackRange;
		public Entity mGround;

		// Target Variables
		public Entity mHoley;
		public Entity mMoley;
		public Entity mTarget;
		public bool mFoundTarget = false;
		public bool mCanChaseTarget = false;
		private const float maxDistFromSpawn = 50f;

		public vec3 originalSpawnPoint = new vec3();
		public vec3 defaultVector = new vec3(1, 0, 0);
		public vec3 moveVector = new vec3(1, 0, 0);
		private const float padPredict = 10.0f;
		private const float moveSpeed = 50.0f;
		private const float maxVelocity = 50.0f;
		private bool isGrounded = false;

		private bool commitSepuku = false;
		private float armingTimer = 0f;
		private const float armingDuration = 1.25f;

		private Entity pinataEffect;

		public void Start()
		{
			originalSpawnPoint = transform.Position;
			mHoley = new Entity(ECSManager.FindIDFromName("Holey"));
			mMoley = new Entity(ECSManager.FindIDFromName("Moley"));
			pinataEffect = parenting.GetChildFromName("PinataEffect");
			mTarget = new Entity(); // Invalid ID
			mGround = new Entity(); // Invalid ID
			mDetectorRange = parenting.GetChildFromName("DetectorRange");
			mAttackRange = parenting.GetChildFromName("AttackRange");
		}

		public void Update()
		{
			transform.Rotation = new vec3(0, transform.Rotation.y, 0);

			// Jump and commit sepuku
			if (armingTimer > 0f)
			{
				armingTimer -= Time.deltaTime;

				// If it is grounded, jump up
				if (isGrounded)
				{
					PS.AddForce(ID, new vec3(0, 10f, 0), ForceMode.VelocityChange);
				}
			}

			if (commitSepuku && armingTimer <= 0f)
			{
				Explode();
			}

			if (commitSepuku || armingTimer > 0) return;

			// Check if foundTarget
			if (!mFoundTarget || !mCanChaseTarget)
			{
				float distanceFromHoley = (transform.Position - mHoley.transform.Position).Length;
				float distanceFromMoley = (transform.Position - mMoley.transform.Position).Length;
				// Check if collide with moley or holey
				if (PS.IsTriggerStay(mDetectorRange.ID, mHoley.ID))
				{
					mTarget = mHoley;
					mFoundTarget = true;
				}
				if (PS.IsTriggerStay(mDetectorRange.ID, mMoley.ID) && distanceFromHoley > distanceFromMoley)
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
				if (PS.IsTriggerExit(mDetectorRange.ID, mTarget.ID) || distanceFromSpawn > maxDistFromSpawn)
				{
					PS.SetLinearVelocity(ID, vec3.Zero);
					mFoundTarget = false;
				}

				// Check if go explode
				if (mFoundTarget)
				{
					// Check if moley or holey is in attack range
					if (PS.IsTriggerStay(mAttackRange.ID, mHoley.ID) || PS.IsTriggerStay(mAttackRange.ID, mMoley.ID))
					{
						commitSepuku = true;
						armingTimer = armingDuration;
						PS.SetLinearVelocity(ID, vec3.Zero);
					}
				}
			}

			// Check if player is within detect sphere
			if (mFoundTarget && mGround.ID != 0)
			{
				// Set moveVector based on angle
				moveVector = mTarget.transform.Position - transform.Position;

				// Ignore y-axis
				PS.GetLinearVelocity(ID, out vec3 currVelocity);
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
					PS.SetLinearVelocity(ID, currVelocity + moveDir);

					// If it is grounded, jump up
					if (isGrounded)
					{
						PS.AddForce(ID, new vec3(0, 7.5f, 0), ForceMode.VelocityChange);
					}

					mCanChaseTarget = true;
				}
				else
				{
					PS.SetLinearVelocity(ID, vec3.Zero);
					mCanChaseTarget = false;
				}
			}
		}

		private void OnCollisionEnter(/*Collider*/System.UInt64 otherID)
		{
			// Assign ground if valid
			Entity other = new Entity(otherID);
			if (mGround.ID == 0 && (other.CompareTag("Ground") || other.CompareTag("Platform")) && other.HasComponent<BoxCollider>())
			{
				mGround = other;
				mCanChaseTarget = true;
				isGrounded = true;
			}
			else if (mGround.ID == otherID && (other.CompareTag("Ground") || other.CompareTag("Platform")) && other.HasComponent<BoxCollider>())
			{
				isGrounded = true;
			}
		}

		private void OnCollisionExit(/*Collider*/System.UInt64 otherID)
		{
			// Assign ground if valid
			Entity other = new Entity(otherID);
			if (mGround.ID == otherID && (other.CompareTag("Ground") || other.CompareTag("Platform")) && other.HasComponent<BoxCollider>())
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

		private void Explode()
		{
			// Take Damage for Holey or Moley within attack range
			if (PS.IsTriggerStay(mAttackRange.ID, mMoley.ID))
			{
				mMoley.GetComponent<MoleyController>().TakeDamage();
			}
			if (PS.IsTriggerStay(mAttackRange.ID, mHoley.ID))
			{
				mHoley.GetComponent<HoleyController>().TakeDamage();
			}

			// Destroy self
			DestroySelf();
		}

		public void OnDestroy()
		{
			ParticleSystem3D.SetActive(pinataEffect.ID, true);
		}
	}
}