using System;
using System.Collections.Generic;
using System.Reflection;
using System.Runtime.CompilerServices;
using static System.Runtime.CompilerServices.RuntimeHelpers;
using System.Threading;
using System.Diagnostics.Eventing.Reader;

namespace TRE
{
	using PS = PhysicsSystem;
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

		private float lerpSpeed = 0.05f;

        //Capsule Collider
        public List<Entity> pickedPowerUps;	// For dropping
        public bool haveBlueberry = false;  // Scaling
        public bool haveStrawberry = false; // Shape
        private bool isScaled = false;
		private float defaultRadius = 2f;
		private float superRadius = 4.8f;
		private float currentRadius = 2f;
		private float defaultHeight = 2f;
		private float superHeight = 4.2f;
		private float currentHeight = 2f;
		//Transform Scale
        private Vector3 defaultXform = new Vector3(0.75f, 0.75f, 0.75f);
		private Vector3 scaledXform = new Vector3(1f, 2.7f, 1f);

        private Vector3 playerDirection = new Vector3(0, 0, 1);

		public void Start()
		{
			TransformSystem.SetRotation(this.ID, new Vector3(0, 0, 0));
			PS.ConstrainRotationX(this.ID, true);
			PS.ConstrainRotationY(this.ID, true);
			PS.ConstrainRotationZ(this.ID, true);
		}

		public void Update()
		{
			TransformSystem.GetPosition(this.ID, out Vector3 pos);

			//Movement Related stuff
			PS.GetLinearVelocity(this.ID, out Vector3 currVelocity);

			dirVec = new Vector3(0, 0, 0);
            #region Movement
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
					Vector3 maxHeight = new Vector3(0, 35, 0);
					Jump(maxHeight);
				}
			}
            #endregion

            #region Abilities
            if (InputSystem.GetKeyTrigger(InputKeys.Backspace))
            {
                if (haveBlueberry)
                {
                    isScaled = !isScaled;
                }
            }

			if ((isScaled == false || !haveBlueberry))
			{
				currentHeight = MathF.Lerp(currentHeight, defaultHeight, lerpSpeed);
				currentRadius = MathF.Lerp(currentRadius, defaultRadius, lerpSpeed);
				PS.ResizeCapsuleCollider(this.ID, currentRadius, currentHeight);
				TransformSystem.SetScaling(this.ID, defaultXform);
			}
			else
			{
				currentHeight = MathF.Lerp(currentHeight, defaultHeight, lerpSpeed);
				currentRadius = MathF.Lerp(currentRadius, defaultRadius, lerpSpeed);
				PS.ResizeCapsuleCollider(this.ID, currentRadius, currentHeight);
				TransformSystem.SetScaling(this.ID, scaledXform);
			}
            #endregion

            dirVec.Normalize();

			if (dirVec != Vector3.zero)
			{
				if (Math.Sqrt(currVelocity.x * currVelocity.x + currVelocity.z * currVelocity.z) < maxVelocity)
				{
					finalVelocity = currVelocity + (dirVec * acceleration * Time.GetDeltaTime());
					PS.SetLinearVelocity(this.ID, finalVelocity);
				}
				else
				{
					Vector3 tmp = dirVec * maxVelocity;
					finalVelocity = new Vector3(tmp.x, currVelocity.y, tmp.z);
					PS.SetLinearVelocity(this.ID, finalVelocity);
				}
			}

			TransformSystem.SetRotation(this.ID, playerDirection);
		}
		private void Jump(Vector3 JumpHeight)
		{
			PS.AddForce(this.ID, JumpHeight, ForceMode.VelocityChange);
		}

		private void OnCollisionStay(System.UInt64 otherID)
		{
			isGrounded = false;

			Entity other = new Entity(otherID);
			if (PS.IsCollisionStay(this.ID, otherID))
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
