using System;
using System.Collections.Generic;
using System.Reflection;
using System.Runtime.CompilerServices;
using static System.Runtime.CompilerServices.RuntimeHelpers;
using System.Threading;
using System.Diagnostics.Eventing.Reader;
using Coroutine;
using GlmSharp;

namespace TRE
{
	using PS = PhysicsSystem;
	using CS = CameraSystem;
	class HoleyController : Entity
	{
		public PowerUpUI MyPowerUpUI;
		public PowerUpManager MyPowerManager;

        //Check if player is boosted jump
        public bool isBoostedJump = false;
		//Check if player is on the ground
		public bool isGrounded = true;
		//direction vector
		private vec3 dirVec;
		//Max velocity
		private float maxVelocity = 30f;
        //Max Air Velocity vector
        private float maxAirVelocity = 25f;
        //Acceleration
        private float acceleration = 700f;
		//final velocity
		private vec3 finalVelocity = vec3.Zero;
		//maxJumpHeight
		private float maxJumpHeight = 70f;
		//Check if player is walking
		private bool isWalking = false;
		private bool walkingSFXPlayed = false;

		private float lerpSpeed = 5f;

		//Capsule Collider
		public bool mainBlueberry = false;  // Scaling
		public bool mainStrawberry = false; // Shape
		public bool isScaled = false;
		private float defaultRadius = 2f;
		private float blueberrysuperRadius = 4.8f;
		private float strawberrysuperRadius = 2.4f;
		private float currentRadius = 2f;
		private float defaultHeight = 1f;
		private float blueberrysuperHeight = 4.8f;
		private float strawberrysuperHeight = 1.2f;
		private float currentHeight = 1f;
		//Transform Scale
		private vec3 defaultXform = new vec3(0.75f, 0.75f, 0.75f);
		private vec3 blueberryscaledXform = new vec3(1f, 2.2f, 1f);
		private vec3 strawberryscaledXform = new vec3(0.5f, 1.5f, 1f);
		private vec3 currentXform = new vec3(0.75f, 0.75f, 0.75f);

		private int playerDirection = 0;
		private int lastPlayerDirection = 0;

		//Respawn variables
		private vec3 RespawnPoint = new vec3(0, 0, 0);
        private bool RespawnPlayer = false;
		private float RespawnTimer = 1.0f;

		// Is Dead
		public bool isDead = false;

		private vec3 InitialPosition = new vec3(0.0f, 0.0f, 0.0f);
		private vec3 OutofMapPos = new vec3(0.0f, 0.0f, 0.0f);
		private bool DroppingOutOfMap = false;

		private bool Invulnerability = false;
		private float InvulCurrent = 1.0f;
		private float InvulPeriod = 1.0f;
		private float InvulBlinkCurrent = 0.1f;
		private float InvulBlinkPeriod = 0.1f;

		private ulong walkingSFX;
		private ulong jumpSFX;
		private ulong changesizeSFX;
		private ulong normalsizeSFX;

		//Transfrom Component
		private Transform holeyTransform;

		public void Start()
		{
			MyPowerUpUI = ECSManager.FindEntityByName("RightCharacter_HUD").GetComponent<PowerUpUI>();
			MyPowerManager = parenting.GetChildFromName("Power Manager").GetComponent<PowerUpManager>();
			MyPowerManager.MyPowerUpUI = MyPowerUpUI;
			MyPowerUpUI.UpdateUI(MyPowerManager.powerUps);

			holeyTransform = GetComponent<Transform>();
			holeyTransform.Rotation = new vec3(0, 0, 0);

			PS.ConstrainRotationX(this.ID, true);
			PS.ConstrainRotationY(this.ID, true);
			PS.ConstrainRotationZ(this.ID, true);

			InitialPosition = holeyTransform.Position;
			OutofMapPos = holeyTransform.Position;
			OutofMapPos.y = holeyTransform.Position.y - 5.0f;

			walkingSFX = ECSManager.FindIDFromName("SFX_HoleyFootsteps");
			jumpSFX = ECSManager.FindIDFromName("SFX_HoleyJump");
			changesizeSFX = ECSManager.FindIDFromName("SFX_Tall");
			normalsizeSFX = ECSManager.FindIDFromName("SFX_NormalSize");

			RespawnPoint = holeyTransform.Position;
		}

		public void Update()
		{
			#region Invulnerability
			if (Invulnerability)
			{
				InvulBlinkCurrent -= Time.deltaTime;
				if (InvulBlinkCurrent <= 0)
				{
					GetComponent<MeshRenderer>().Visible = !GetComponent<MeshRenderer>().Visible;
					InvulBlinkCurrent = InvulBlinkPeriod;
				}
				InvulCurrent -= Time.deltaTime;
				if (InvulCurrent <= 0)
				{
					Invulnerability = false;
					GetComponent<MeshRenderer>().Visible = true;
					InvulCurrent = InvulPeriod;
				}

			}
			#endregion
			vec3 pos = holeyTransform.Position;

			//Movement Related stuff
			PS.GetLinearVelocity(this.ID, out vec3 currVelocity);

			if (pos.y < OutofMapPos.y)
            {
                isDead = true;
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
				RespawnPlayer = true;
				Debug.Log(RespawnPoint.ToString());
				//Debug.Log("Respawn");
			}

			dirVec = new vec3(0, 0, 0);

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
					dirVec += CS.GetMainCameraForwardVec();
					lastPlayerDirection = 0;
				}

				if (InputSystem.GetKeyDown(InputKeys.K))
				{
					dirVec -= CS.GetMainCameraForwardVec();
					lastPlayerDirection = 180;
				}

				if (InputSystem.GetKeyDown(InputKeys.J))
				{
					dirVec += CS.GetMainCameraRightVec();
					lastPlayerDirection = 90;
				}

				if (InputSystem.GetKeyDown(InputKeys.L))
				{
					dirVec -= CS.GetMainCameraRightVec();
					lastPlayerDirection = 270;
				}

				if (InputSystem.GetKeyDown(InputKeys.I))
				{
					if (InputSystem.GetKeyDown(InputKeys.L))
					{
						lastPlayerDirection = 315;
					}

					if (InputSystem.GetKeyDown(InputKeys.J))
					{
						lastPlayerDirection = 45;
					}
				}

				if (InputSystem.GetKeyDown(InputKeys.K))
				{
					if (InputSystem.GetKeyDown(InputKeys.L))
					{
						lastPlayerDirection = 225;
					}

					if (InputSystem.GetKeyDown(InputKeys.J))
					{
						lastPlayerDirection = 135;
					}
				}

				if (InputSystem.GetKeyTrigger(InputKeys.Enter))
				{
					
					isWalking = false;

					if (isGrounded)
					{
						// Boosted Jump
						if (isBoostedJump)
						{
							vec3 maxHeight = new vec3(0, 150, 0);
							Jump(maxHeight);
							if (ECSManager.IsValidEntity(jumpSFX))
							{
								AudioSystem.Play(jumpSFX);
							}
						}
						else
						{
							vec3 maxHeight = new vec3(0, 70, 0);
							Jump(maxHeight);
							if (ECSManager.IsValidEntity(jumpSFX))
							{
								AudioSystem.Play(jumpSFX);
							}
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

			if (ECSManager.IsValidEntity(walkingSFX))
			{
				if (isWalking && walkingSFXPlayed == false)
				{
					AudioSystem.Play(walkingSFX);
					walkingSFXPlayed = true;
				}

				if (!isWalking || !isGrounded)
				{
					AudioSystem.Stop(walkingSFX);
					walkingSFXPlayed = false;
				}
			}


			#endregion

			#region Swap

			// Check if can swap ability
			if (InputSystem.GetKeyTrigger(InputKeys.Backslash))
			{
				MyPowerManager.SwapPowerUps();
				MyPowerUpUI.UpdateUI(MyPowerManager.powerUps);
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
					if (isScaled)
					{
						if (ECSManager.IsValidEntity(changesizeSFX))
						{
							AudioSystem.Play(changesizeSFX);
						}
					}
					else
					{
						if (ECSManager.IsValidEntity(normalsizeSFX))
						{
							AudioSystem.Play(normalsizeSFX);
						}
					}
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
				//if(GetComponent<MeshRenderer>().Mesh != "6ee6fad4e6ecaab8")
				//	GetComponent<MeshRenderer>().Mesh = "6ee6fad4e6ecaab8";
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
				//Change to cactus
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
			if (dirVec != new vec3())
				dirVec = dirVec.Normalized;

			playerDirection = lastPlayerDirection + (int)CS.GetMainCameraRotation().y;
			playerDirection = (playerDirection % 360);

            if (dirVec != vec3.Zero)
            {
                if (isGrounded)
                {
                    if (Math.Sqrt(currVelocity.x * currVelocity.x + currVelocity.z * currVelocity.z) < maxVelocity)
                    {
                        finalVelocity = currVelocity + (dirVec * acceleration * Time.deltaTime);
                        PS.SetLinearVelocity(this.ID, finalVelocity);
                    }
                    else
                    {
                        vec3 tmp = dirVec * maxVelocity;
                        finalVelocity = new vec3(tmp.x, currVelocity.y, tmp.z);
                        PS.SetLinearVelocity(this.ID, finalVelocity);
                    }
                }
                else if (!isGrounded)
                {
                    //change to be air max velocirty instead
                    if (Math.Sqrt(currVelocity.x * currVelocity.x + currVelocity.z * currVelocity.z) < maxAirVelocity)
                    {
                        finalVelocity = currVelocity + (dirVec * acceleration * Time.deltaTime);
                        PS.SetLinearVelocity(this.ID, finalVelocity);
                    }
                    else
                    {
                        vec3 tmp = dirVec * maxAirVelocity;
                        finalVelocity = new vec3(tmp.x, currVelocity.y, tmp.z);
                        PS.SetLinearVelocity(this.ID, finalVelocity);
                    }
                }
            }

            holeyTransform.Rotation = new vec3(0, playerDirection, 0);

			isGrounded = false;

			if(RespawnPlayer)
            {
                if (RespawnTimer > 0)
                {
                    RespawnTimer -= Time.deltaTime;
                }
                else
                {
                    Respawn(); 
					isDead = false;
                    RespawnPlayer = false;
					RespawnTimer = 1.0f;
                }
            }
		}
		private void Jump(vec3 JumpHeight)
		{
			PS.AddForce(this.ID, JumpHeight, ForceMode.VelocityChange);
		}

		private void OnCollisionStay(System.UInt64 otherID)
		{
			isGrounded = false;

			Entity other = new Entity(otherID);
			// Make it loose one of it's powerups
			if (other.CompareTag("FallingObstacle") || other.CompareTag("RollingObstacle"))
			{
				TakeDamage();
			}
			// Check is activated jumppad
			if (other.CompareTag("JumpPad"))
            {
                isGrounded = true;
                if (other.GetComponent<JumpPad>().isActivated)
				{
					isBoostedJump = true;
				}
            }
            if (other.CompareTag("Ground") || other.CompareTag("Platform")
                || other.CompareTag("Blue") || other.CompareTag("BlueCollider")
				|| other.CompareTag("LeftCactus") || other.CompareTag("RightCactus"))
            {
				isGrounded = true;
			}
		}
		private void OnCollisionExit(System.UInt64 otherID)
		{
			Entity other = new Entity(otherID);
			if (EngineGetTag(otherID) == "Red")
			{
				PS.GetLinearVelocity(this.ID, out vec3 output);
				if (output.y > maxJumpHeight)
					output.y = maxJumpHeight;
				PS.SetLinearVelocity(this.ID, output);
			}
			// No longer boosted if leave jumppad
			else if (other.CompareTag("JumpPad"))
            {
                isBoostedJump = false;
				isGrounded = false;
            }
            if (other.CompareTag("Ground") || other.CompareTag("Platform")
                || other.CompareTag("Red") || other.CompareTag("RedCollider"))
            {
                isGrounded = false;
            }
        }

		public void UpdateDisplay()
		{
			MyPowerUpUI.UpdateUI(MyPowerManager.powerUps);
		}

		public void TakeDamage()
		{
			if (Invulnerability) return;
			if (MyPowerManager.powerUps.Count > 0)
			{
				MyPowerManager.LoseMain();
				isScaled = false;
			}
			else
            {
                RespawnPlayer = true;
                isDead = true;
            }
			Invulnerability = true;
		}

		public void ResetToInitialPos()
		{
			holeyTransform.Position = InitialPosition;
		}

		// activate this function using a coroutine to time the spawning of the player
		private void Respawn() 
		{
			holeyTransform.Position = RespawnPoint;
            Invulnerability = true;
        }

		public void SetRespawnPoint(vec3 position)
        {
            RespawnPoint = position;
        }

        public bool GetIsDead()
        {
            return isDead;
        }
	}
}
