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
	using CS = CameraSystem;
	class HoleyController : Entity
	{
		public PowerUpManager MyPowerManager;

		//Check if player is boosted jump
		private bool isBoostedJump = false;
		//Check if player is on the ground
		private bool isGrounded = true;
		//direction vector
		private Vector3 dirVec;
		//Max velocity
		private float maxVelocity = 30f;
		//Acceleration
		private float acceleration = 700f;
		//final velocity
		private Vector3 finalVelocity = Vector3.zero;
		//maxJumpHeight
		private float maxJumpHeight = 70f;
		//Check if player is walking
		private bool isWalking = false;
		private bool walkingSFXPlayed = false;

		private float lerpSpeed = 5f;

		//Capsule Collider
		public bool mainBlueberry = false;  // Scaling
		public bool mainStrawberry = false; // Shape
		private bool isScaled = false;
		private float defaultRadius = 2f;
		private float blueberrysuperRadius = 4.8f;
		private float strawberrysuperRadius = 2.4f;
		private float currentRadius = 2f;
		private float defaultHeight = 1f;
		private float blueberrysuperHeight = 4.8f;
		private float strawberrysuperHeight = 1.2f;
		private float currentHeight = 1f;
		//Transform Scale
		private Vector3 defaultXform = new Vector3(0.75f, 0.75f, 0.75f);
		private Vector3 blueberryscaledXform = new Vector3(1f, 2.2f, 1f);
		private Vector3 strawberryscaledXform = new Vector3(0.5f, 1.5f, 1f);
		private Vector3 currentXform = new Vector3(0.75f, 0.75f, 0.75f);

		private Vector3 playerDirection = new Vector3(0, 0, 1);

		private Vector3 InitialPosition = new Vector3(0.0f, 0.0f, 0.0f);
		private Vector3 OutofMapPos = new Vector3(0.0f, 0.0f, 0.0f);
		private bool DroppingOutOfMap = false;

		public void Start()
		{
			MyPowerManager = parenting.GetChildFromName("Power Manager").GetComponent<PowerUpManager>();

			TransformSystem.SetRotation(this.ID, new Vector3(0, 0, 0));
			PS.ConstrainRotationX(this.ID, true);
			PS.ConstrainRotationY(this.ID, true);
			PS.ConstrainRotationZ(this.ID, true);

			TransformSystem.GetPosition(this.ID, out Vector3 InitialPos);
			InitialPosition = InitialPos;
			OutofMapPos = InitialPos;
			OutofMapPos.y = InitialPos.y - 5.0f;
		}

		public void Update()
		{
			TransformSystem.GetPosition(this.ID, out Vector3 pos);

			//Movement Related stuff
			PS.GetLinearVelocity(this.ID, out Vector3 currVelocity);

			if (pos.y < OutofMapPos.y)
			{
				DroppingOutOfMap = true;
				//Debug.Log("Out of map");
			}
			else
			{
				DroppingOutOfMap = false;
				//Debug.Log("Not out of map");
			}

			if (pos.y < (InitialPosition.y - 50.0f))
			{
				TransformSystem.SetPosition(this.ID, InitialPosition);
				//Debug.Log("Respawn");
			}

			dirVec = new Vector3(0, 0, 0);
			#region Movement

			if (DroppingOutOfMap)
			{
				dirVec.x = 0.0f;
				dirVec.z = 0.0f;
			}
			else if (DroppingOutOfMap == false)
			{
				if (InputSystem.GetKeyDown(InputKeys.I))
				{
					//dirVec.z += -1;
					dirVec += CS.GetMainCameraForwardVec();

					playerDirection.y = 180;
				}

				if (InputSystem.GetKeyDown(InputKeys.K))
				{
					//dirVec.z += 1;
					dirVec -= CS.GetMainCameraForwardVec();
					playerDirection.y = 0;
				}

				if (InputSystem.GetKeyDown(InputKeys.J))
				{
					//dirVec.x += -1;
					dirVec += CS.GetMainCameraRightVec();
					playerDirection.y = 270;
				}

				if (InputSystem.GetKeyDown(InputKeys.L))
				{
					//dirVec.x += 1;
					dirVec -= CS.GetMainCameraRightVec();
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
					AudioSystem.PlayOnce(6503599471310675157);
					isWalking = false;

					if (isGrounded)
					{
						// Boosted Jump
						if (isBoostedJump)
						{
							Vector3 maxHeight = new Vector3(0, 150, 0);
							Jump(maxHeight);
						}
						else
						{
							Vector3 maxHeight = new Vector3(0, 70, 0);
							Jump(maxHeight);
						}
					}
				}
			}
			#endregion

			#region Audio
			if (InputSystem.GetKeyDown(InputKeys.I) || InputSystem.GetKeyDown(InputKeys.K) ||
			InputSystem.GetKeyDown(InputKeys.J) || InputSystem.GetKeyDown(InputKeys.L))
			{
				isWalking = true;
			}

			if (!(InputSystem.GetKeyDown(InputKeys.I) || InputSystem.GetKeyDown(InputKeys.J) ||
				InputSystem.GetKeyDown(InputKeys.K) || InputSystem.GetKeyDown(InputKeys.L)))
			{
				isWalking = false;
			}

			if (isWalking && walkingSFXPlayed == false)
			{
				AudioSystem.Play(15348080909718226430);
				walkingSFXPlayed = true;
			}

			if (!isWalking || !isGrounded)
			{
				AudioSystem.Stop(15348080909718226430);
				walkingSFXPlayed = false;
			}


			//Debug.Log("Audio:" + AudioSystem.GetIsPlaying(15348080909718226430));
			//Debug.Log("isWalking: " + isWalking);

			#endregion

			#region Swap
			// Check if can swap ability
			if (InputSystem.GetKeyTrigger(InputKeys.Backslash))
			{
				MyPowerManager.SwapPowerUps();
				isScaled = false;
			}
			#endregion

			#region Drop
			// Check if can trigger ability
			if (InputSystem.GetKeyTrigger(InputKeys.RightShift))
			{
				MyPowerManager.DropMain();
				isScaled = false;
			}
			#endregion

			#region Abilities
			mainBlueberry = MyPowerManager.powerUps.Count > 0 && MyPowerManager.powerUps[0].CompareTag("Blueberry");
			mainStrawberry = MyPowerManager.powerUps.Count > 0 && MyPowerManager.powerUps[0].CompareTag("Strawberry");
			if (isScaled && !mainBlueberry && !mainStrawberry)
			{
				isScaled = false;
			}
			if (InputSystem.GetKeyTrigger(InputKeys.Backspace))
			{
				if (mainBlueberry || mainStrawberry)
				{
					isScaled = !isScaled;
				}
			}

			if (isScaled == false || (!mainBlueberry && !mainStrawberry))
			{
				currentHeight = MathF.Lerp(currentHeight, defaultHeight, lerpSpeed * Time.deltaTime);
				currentRadius = MathF.Lerp(currentRadius, defaultRadius, lerpSpeed * Time.deltaTime);
				PS.ResizeCapsuleCollider(this.ID, currentRadius, currentHeight);
				currentXform.x = MathF.Lerp(currentXform.x, defaultXform.x, lerpSpeed * Time.deltaTime);
				currentXform.y = MathF.Lerp(currentXform.y, defaultXform.y, lerpSpeed * Time.deltaTime);
				currentXform.z = MathF.Lerp(currentXform.z, defaultXform.z, lerpSpeed * Time.deltaTime);
				TransformSystem.SetScaling(this.ID, currentXform);
			}
			else if (mainBlueberry)
			{
				currentHeight = MathF.Lerp(currentHeight, blueberrysuperHeight, lerpSpeed * Time.deltaTime);
				currentRadius = MathF.Lerp(currentRadius, blueberrysuperRadius, lerpSpeed * Time.deltaTime);
				PS.ResizeCapsuleCollider(this.ID, currentRadius, currentHeight);
				currentXform.x = MathF.Lerp(currentXform.x, blueberryscaledXform.x, lerpSpeed * Time.deltaTime);
				currentXform.y = MathF.Lerp(currentXform.y, blueberryscaledXform.y, lerpSpeed * Time.deltaTime);
				currentXform.z = MathF.Lerp(currentXform.z, blueberryscaledXform.z, lerpSpeed * Time.deltaTime);
				TransformSystem.SetScaling(this.ID, currentXform);
			}
			else if (mainStrawberry)
			{
				currentHeight = MathF.Lerp(currentHeight, strawberrysuperHeight, lerpSpeed * Time.deltaTime);
				currentRadius = MathF.Lerp(currentRadius, strawberrysuperRadius, lerpSpeed * Time.deltaTime);
				PS.ResizeCapsuleCollider(this.ID, currentRadius, currentHeight);
				currentXform.x = MathF.Lerp(currentXform.x, strawberryscaledXform.x, lerpSpeed * Time.deltaTime);
				currentXform.y = MathF.Lerp(currentXform.y, strawberryscaledXform.y, lerpSpeed * Time.deltaTime);
				currentXform.z = MathF.Lerp(currentXform.z, strawberryscaledXform.z, lerpSpeed * Time.deltaTime);
				TransformSystem.SetScaling(this.ID, currentXform);
			}
			#endregion

			dirVec.y = 0;
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

			TransformSystem.SetRotation(this.ID, new Vector3(0, playerDirection.y, 0));
		}
		private void Jump(Vector3 JumpHeight)
		{
			PS.AddForce(this.ID, JumpHeight, ForceMode.VelocityChange);
		}

		private void OnCollisionStay(System.UInt64 otherID)
		{
			isGrounded = false;

			Entity other = new Entity(otherID);
			// Check is activated jumppad
			if (other.CompareTag("JumpPad"))
			{
				if (other.GetComponent<JumpPad>().isActivated) isBoostedJump = true;
			}
			if (PS.IsCollisionStay(this.ID, otherID))
			{
				if (EngineGetTag(otherID) == "Ground" || EngineGetTag(otherID) == "JumpPad" || EngineGetTag(otherID) == "Platform"
					|| EngineGetTag(otherID) == "Red" || EngineGetTag(otherID) == "RedCollider")
				{
					isGrounded = true;
				}
				else
				{
					isGrounded = false;
				}
			}
			if (PS.IsCollisionExit(this.ID, otherID))
			{
				if (EngineGetTag(otherID) == "Red")
				{
					PS.GetLinearVelocity(this.ID, out Vector3 output);
					if (output.y > maxJumpHeight)
						output.y = maxJumpHeight;
					PS.SetLinearVelocity(this.ID, output);
				}
				// No longer boosted if leave jumppad
				else if (EngineGetTag(otherID) == "JumpPad")
				{
					isBoostedJump = false;
					isGrounded = false;
				}
			}
		}
	}
}
