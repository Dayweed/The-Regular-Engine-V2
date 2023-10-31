using System;
using System.Collections.Generic;
using System.Reflection;
using System.Runtime.CompilerServices;
using static System.Runtime.CompilerServices.RuntimeHelpers;
using System.Threading;
using System.Diagnostics.Eventing.Reader;

namespace TRE
{
	class MoleController2 : Entity
	{
		//Check if player is on the ground
		private bool isGrounded = true;
		//direction vector
		private Vector3 dirVec;
		//Max velocity
		private float maxVelocity = 15f;
		//Acceleration
		private float acceleration = 300f;
		//final velocity
		private Vector3 finalVelocity = Vector3.zero;
		//Max height
		private Vector3 maxHeight = new Vector3(0, 5, 0);

		//check if player used super power
		private bool isScaled = false;
		//Default scale
		private float defaultScale = 0.5f;
		//Increase character scale
		private float superScale = 5;
		//Current scale
		private float currentScale = 1;

		private Vector3 playerDirection = new Vector3(0, 0, 1);

		public void Start()
		{
			TransformSystem.SetRotation(this.ID, new Vector3(0, 0, 0));
			PhysicsSystem.ConstrainRotationX(this.ID, true);
			PhysicsSystem.ConstrainRotationY(this.ID, true);
			PhysicsSystem.ConstrainRotationZ(this.ID, true);
		}

		public void Update()
		{
			TransformSystem.GetPosition(this.ID, out Vector3 pos);

			//Movement Related stuff
			PhysicsSystem.GetLinearVelocity(this.ID, out Vector3 currVelocity);

			dirVec = new Vector3(0, 0, 0);

			if (InputSystem.GetKeyDown(InputKeys.I))
			{
				dirVec.z += -1;
				playerDirection.y = 180;
			}

			if (InputSystem.GetKeyDown(InputKeys.K))
			{
				dirVec.z += 1;
				playerDirection.y = 0;
			}

			if (InputSystem.GetKeyDown(InputKeys.J))
			{
				dirVec.x += -1;
				playerDirection.y = 270;
			}

			if (InputSystem.GetKeyDown(InputKeys.L))
			{
				dirVec.x += 1;
				playerDirection.y = 90;
			}

			if (InputSystem.GetKeyDown(InputKeys.I))
			{
				if (InputSystem.GetKeyDown(InputKeys.L))
				{
					playerDirection.y = 135;
				}
				if (InputSystem.GetKeyDown(InputKeys.J))
				{
					playerDirection.y = 225;
				}
			}

			if (InputSystem.GetKeyDown(InputKeys.K))
			{
				if (InputSystem.GetKeyDown(InputKeys.L))
				{
					playerDirection.y = 45;
				}
				if (InputSystem.GetKeyDown(InputKeys.J))
				{
					playerDirection.y = 315;
				}
			}

			if (InputSystem.GetKeyTrigger(InputKeys.Enter))
			{
				if (isGrounded)
				{
                    Vector3 maxHeight = new Vector3(0, 30, 0);
                    Jump(maxHeight);
                }
			}

			if (InputSystem.GetKeyTrigger(InputKeys.Backspace))
			{
				isScaled = !isScaled;
			}

			if(isScaled == false)
			{
				currentScale = MathF.Lerp(currentScale, defaultScale, 0.2f);
				PhysicsSystem.ResizeCapsuleCollider(this.ID, 2, currentScale);
			}
			else
			{
				currentScale = MathF.Lerp(currentScale, superScale, 0.2f);
				PhysicsSystem.ResizeCapsuleCollider(this.ID, 2, currentScale);
			}

			dirVec.Normalize();

			if (dirVec != Vector3.zero)
			{
				if (Math.Sqrt(currVelocity.x * currVelocity.x + currVelocity.z * currVelocity.z) < maxVelocity)
				{
					finalVelocity = currVelocity + (dirVec * acceleration * Time.GetDeltaTime());
					PhysicsSystem.SetLinearVelocity(this.ID, finalVelocity);
				}
				else
				{
					Vector3 tmp = dirVec * maxVelocity;
					finalVelocity = new Vector3(tmp.x, currVelocity.y, tmp.z);
					PhysicsSystem.SetLinearVelocity(this.ID, finalVelocity);
				}
			}

			TransformSystem.SetRotation(this.ID, playerDirection);
		}
		private void Jump(Vector3 JumpHeight)
		{
            PhysicsSystem.AddForce(this.ID, JumpHeight, ForceMode.VelocityChange);
        }

		private void OnCollisionStay(System.UInt64 otherID)
		{
			isGrounded = false;

			Entity other = new Entity(otherID);
			if (PhysicsSystem.IsCollisionStay(this.ID, otherID))
			{
				if (EngineGetTag(otherID) == "Ground" || EngineGetTag(otherID) == "Player")
				{
					isGrounded = true;
				}
				else
				{
					isGrounded = false;
				}
			}
		}
	}
}
