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

		private float moveSpeed = 5.0f;
		private float rotateSpeed = 100.0f;

		private float cooldown = 0f;
		private float cooldownDefault = 2.0f;

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
		}

		public void Update()
		{
			if (cooldown > 0) cooldown -= Time.deltaTime;

			// Check if the ledges is no longer being triggered
			if (lLedge == null || rLedge == null) return;

			transform.Position += moveVector * moveDir * moveSpeed * Time.deltaTime;
			transform.Rotation += rotateVector * moveDir * rotateSpeed * Time.deltaTime;
			transform.Rotation = transform.Rotation.z > 360 ? transform.Rotation - threesixty : transform.Rotation;
			transform.Rotation = transform.Rotation.z < 0 ? transform.Rotation + threesixty : transform.Rotation;
		}

		public void Bounceback()
		{
			if (cooldown > 0) return;

			moveDir = moveDir == 1 ? -1 : 1;
			cooldown = cooldownDefault;
		}

		private void OnTriggerEnter(System.UInt64 otherID)
		{
			if (PhysicsSystem.IsTriggerEnter(ID, lLedge.ID) || PhysicsSystem.IsTriggerEnter(ID, rLedge.ID))
			{
				Bounceback();
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
					Bounceback();
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
				Bounceback();
			}
		}
	}
}