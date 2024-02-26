using System;
using System.Collections.Generic;
using System.Reflection;
using System.Runtime.CompilerServices;
using static System.Runtime.CompilerServices.RuntimeHelpers;
using System.Threading;
using GlmSharp;

namespace TRE
{
	public class RollingObj : Entity
	{
		public int moveDir = 1;
		public vec3 defaultVector = new vec3(1, 0, 0);
		public vec3 moveVector = new vec3(1, 0, 0);

		public vec3 rotateVector = new vec3(0, 0, -1);
		public vec3 threesixty = new vec3(0, 0, 360);

		// For checking ledges
		Entity parent;
		Entity lLedge;
		Entity rLedge;

		private float moveSpeed = 18.0f;
		private float rotateSpeed = 200.0f;

		private float cooldown = 0f;
		private float cooldownDefault = 2f;

		public RollingObj()
		{

		}

		public void Start()
		{
			parent = parenting.parent;
			lLedge = parenting.parent.parenting.GetChildFromName("LeftLedge");
			rLedge = parenting.parent.parenting.GetChildFromName("RightLedge");

			// Rotate moveVector based on angle
			moveVector = TransformSystem.RotateVector(defaultVector, parent.transform.Rotation);
			rotateVector = new vec3(moveVector.z, moveVector.y, -moveVector.x);

			if (Scene.GetSceneName() == "Level_1")
			{
				if (parent.name == "RollingObject_4" || parent.name == "RollingObject_5" || parent.name == "RollingObject_6")
				{
					moveSpeed = 25.0f;
				}

				if (parent.name == "RollingObject_7" || parent.name == "RollingObject_8" || parent.name == "RollingObject_9" || parent.name == "RollingObject_10")
				{
					moveSpeed = 32.0f;
				}
			}
			cooldown = cooldownDefault;
		}

		public void Update()
		{
			if (cooldown > 0) cooldown -= Time.deltaTime;

			// Check if the ledges is no longer being triggered
			if (lLedge == null || rLedge == null) return;

			if (cooldown > 0) return;

			transform.Position += moveVector * moveDir * moveSpeed * Time.deltaTime;
			transform.Rotation += rotateVector * moveDir * rotateSpeed * Time.deltaTime;
			transform.Rotation = transform.Rotation.z > 360 ? transform.Rotation - threesixty : transform.Rotation;
			transform.Rotation = transform.Rotation.z < 0 ? transform.Rotation + threesixty : transform.Rotation;
		}

		public void Bounceback(vec3 WallPosition)
		{
			// Check if the WallPosition is against the moveDir, else ignore
			vec3 wallDir = WallPosition - transform.Position;
			if (vec3.Dot(wallDir, moveVector * moveDir) <= 0) return;

			moveDir = moveDir == 1 ? -1 : 1;
		}

		private void OnTriggerStay(System.UInt64 otherID)
		{
			Entity other = new Entity(otherID);
			if (other.ID == lLedge.ID || other.ID == rLedge.ID)
			{
				Bounceback(other.transform.Position);
			}
		}

		private void OnCollisionEnter(System.UInt64 otherID)
		{
			Entity other = new Entity(otherID);
			if (other.CompareTag("Red"))
			{
				// Bounce back if the Moley is using their strawberry powerUp
				MoleyController ctrl = other.GetComponent<MoleyController>();
				if (ctrl != null && ctrl.isScaled && ctrl.mainStrawberry)
				{
					Bounceback(other.transform.Position);
				}
				else
				{
					ctrl.TakeDamage();
				}
			}
			else if (other.CompareTag("Blue"))
			{
				other.GetComponent<HoleyController>().TakeDamage();
			}

		}

		private void OnCollisionStay(System.UInt64 otherID)
		{
			Entity other = new Entity(otherID);
			if (other.CompareTag("Red"))
			{
				// Bounce back if the Moley is using their strawberry powerUp
				MoleyController ctrl = other.GetComponent<MoleyController>();
				if (ctrl != null && ctrl.isScaled && ctrl.mainStrawberry)
				{
					// Not colliding with other ledges
					if (NotAtLedge())
					{
						Bounceback(other.transform.Position);
					}
				}
				else
				{
					ctrl.TakeDamage();
				}
			}
			else if (other.CompareTag("Blue"))
			{
				other.GetComponent<HoleyController>().TakeDamage();
			}
			else if (other.ID == lLedge.ID || other.ID == rLedge.ID)
			{
				Bounceback(other.transform.Position);
			}
		}

		private bool NotAtLedge()
		{
			bool atLLedge = PhysicsSystem.IsCollisionEnter(this.ID, lLedge.ID) || PhysicsSystem.IsCollisionStay(this.ID, lLedge.ID) || PhysicsSystem.IsCollisionExit(this.ID, lLedge.ID);
			bool atRLedge = PhysicsSystem.IsCollisionEnter(this.ID, rLedge.ID) || PhysicsSystem.IsCollisionStay(this.ID, rLedge.ID) || PhysicsSystem.IsCollisionExit(this.ID, rLedge.ID);
			return !atLLedge && !atRLedge;
		}
	}
}