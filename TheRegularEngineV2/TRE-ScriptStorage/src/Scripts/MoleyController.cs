using System;
using System.Collections.Generic;
using System.Reflection;
using System.Runtime.CompilerServices;
using static System.Runtime.CompilerServices.RuntimeHelpers;
using System.Threading;
using GlmSharp;

namespace TRE
{
	using PS = PhysicsSystem;
	using CS = CameraSystem;

	public class MoleyController : Entity
	{
		public PowerUpUI MyPowerUpUI;
		public PowerUpManager MyPowerManager;

		//Check if player is boosted jump
		private bool isBoostedJump = false;
		//check if player is on the ground (for now , just a plane)
		private bool isGrounded = true;
		//direction vector
		private vec3 dirVec;
		//Max Velocity vector
		private float maxVelocity = 30f;
		//Acceleration
		private float acceleration = 700f;
		//final velocity
		private vec3 finalVelocity = vec3.Zero;
		//maxJumpHeight
		private float maxJumpHeight = 70f;
		//Check if player is walking
		private bool isWalking = false;
		private bool walkingSFXPlayed = false;

		//check if player used super power
		public bool mainBlueberry = false;  // Scaling
		public bool mainStrawberry = false; // Shape
		private bool isScaled = false;

		//Box Collider
		private float defaultRadius = 2f;
		private float blueberrysuperRadius = 4f;
		private float strawberrysuperRadius = 1.5f;
		private float currentRadius = 2f;
		private float defaultHeight = 1f;
		private float blueberrysuperHeight = 0.1f;
		private float strawberrysuperHeight = 0.1f;
		private float currentHeight = 1f;

		//Player Scallings
		private vec3 defaultXform = new vec3(0.75f, 0.75f, 0.75f);
		private vec3 blueberryscaledXform = new vec3(2f, 1f, 2f);
		private vec3 strawberryscaledXform = new vec3(2f, 1f, 0.5f);
		private vec3 currentXform = new vec3(0.75f, 0.75f, 0.75f);

		private int playerDirection = 0;
		private int lastPlayerDirection = 0;

		private float lerpSpeed = 5f;

		//For camera controller
		private Entity Key;
		private Entity FinalPlatform;

		public float elapsedTime = 0.0f;

		private vec3 InitialPosition = new vec3(0.0f, 0.0f, 0.0f);
		private vec3 OutofMapPos = new vec3(0.0f, 0.0f, 0.0f);
		private bool DroppingOutOfMap = false;

		private bool Invulnerability = false;
		private float InvulCurrent = 1.0f;
		private float InvulPeriod = 1.0f;
		private float InvulBlinkCurrent = 0.1f;
		private float InvulBlinkPeriod = 0.1f;

        public void Start()
		{
			MyPowerUpUI = ECSManager.FindEntityByName("LeftCharacter_HUD").GetComponent<PowerUpUI>();
            MyPowerManager = parenting.GetChildFromName("Power Manager").GetComponent<PowerUpManager>();
			MyPowerManager.MyPowerUpUI = MyPowerUpUI;
			MyPowerUpUI.UpdateUI(MyPowerManager.powerUps);

            Key = ECSManager.FindEntityByName("Key");
			Debug.Log("Key ID is " + Key.ID);

			FinalPlatform = ECSManager.FindEntityByName("Final_Platform");
			Debug.Log("FinalPlatform ID is " + FinalPlatform.ID);

			TransformSystem.SetRotation(this.ID, new vec3(0, 0, 0));
			PS.ConstrainRotationX(this.ID, true);
			PS.ConstrainRotationY(this.ID, true);
			PS.ConstrainRotationZ(this.ID, true);

			TransformSystem.GetPosition(this.ID, out vec3 InitialPos);
			InitialPosition = InitialPos;
			OutofMapPos = InitialPos;
			OutofMapPos.y = InitialPos.y - 5.0f;
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

            // Move The Test Object 
            TransformSystem.GetPosition(this.ID, out vec3 pos);
			TransformSystem.SetRotation(this.ID, new vec3(0, 0, 0));

			//Movement Related stuff
			PS.GetLinearVelocity(this.ID, out vec3 currVelocity);

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
				ResetToInitialPos();
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
				if (InputSystem.GetKeyDown(InputKeys.W))
				{
					dirVec += CS.GetMainCameraForwardVec();
					lastPlayerDirection = 0;
				}

				if (InputSystem.GetKeyDown(InputKeys.S))
				{
					dirVec -= CS.GetMainCameraForwardVec();
					lastPlayerDirection = 180;
				}

				if (InputSystem.GetKeyDown(InputKeys.A))
				{
					dirVec += CS.GetMainCameraRightVec();
					lastPlayerDirection = 90;
				}

				if (InputSystem.GetKeyDown(InputKeys.D))
				{
					dirVec -= CS.GetMainCameraRightVec();
					lastPlayerDirection = 270;
				}

				if (InputSystem.GetKeyDown(InputKeys.W))
				{
					if (InputSystem.GetKeyDown(InputKeys.D))
					{
						lastPlayerDirection = 315;
					}
					if (InputSystem.GetKeyDown(InputKeys.A))
					{
						lastPlayerDirection = 45;
					}
				}

				if (InputSystem.GetKeyDown(InputKeys.S))
				{
					if (InputSystem.GetKeyDown(InputKeys.D))
					{
						lastPlayerDirection = 225;
					}
					if (InputSystem.GetKeyDown(InputKeys.A))
					{
						lastPlayerDirection = 135;
					}
				}

				if (InputSystem.GetKeyTrigger(InputKeys.Space))
				{
					if (ECSManager.IsValidEntity(6503599471310675157))
					{
						AudioSystem.Play(6503599471310675157);
					}
					isWalking = false;

					if (isGrounded)
					{
						// Boosted Jump
						if (isBoostedJump)
						{
                            vec3 maxHeight = new vec3(0, 150, 0);
							Jump(maxHeight);
						}
						else
						{
                            vec3 maxHeight = new vec3(0, 70, 0);
							Jump(maxHeight);
						}
					}
				}
			}
			#endregion

			#region Audio
			if (InputSystem.GetKeyDown(InputKeys.W) || InputSystem.GetKeyDown(InputKeys.S) ||
			InputSystem.GetKeyDown(InputKeys.A) || InputSystem.GetKeyDown(InputKeys.D))
			{
				isWalking = true;
			}

			if (!(InputSystem.GetKeyDown(InputKeys.W) || InputSystem.GetKeyDown(InputKeys.S) ||
				InputSystem.GetKeyDown(InputKeys.A) || InputSystem.GetKeyDown(InputKeys.D)))
			{
				isWalking = false;
			}

			if (ECSManager.IsValidEntity(15348080909718226430))
			{
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
			}


			//Debug.Log("Audio:" + AudioSystem.GetIsPlaying(15348080909718226430));
			//Debug.Log("isWalking: " + isWalking);

			#endregion

			#region Swap
			// Check if can swap ability
			if (InputSystem.GetKeyTrigger(InputKeys.Q))
			{
				MyPowerManager.SwapPowerUps();
                MyPowerUpUI.UpdateUI(MyPowerManager.powerUps);
                isScaled = false;
			}
			#endregion

			#region Drop
			// Check if can trigger ability
			if (InputSystem.GetKeyTrigger(InputKeys.LeftShift))
			{
				MyPowerManager.DropMain();
                isScaled = false;
			}
			#endregion

			#region Ability
			mainBlueberry = MyPowerManager.powerUps.Count > 0 && MyPowerManager.powerUps[0].CompareTag("Blueberry");
			mainStrawberry = MyPowerManager.powerUps.Count > 0 && MyPowerManager.powerUps[0].CompareTag("Strawberry");

			if (isScaled && !mainBlueberry && !mainStrawberry)
			{
				isScaled = false;
			}

			// Check if can trigger ability
			if (InputSystem.GetKeyTrigger(InputKeys.E))
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
				//for fat boi
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
				//for fat boi
				currentHeight = MathF.Lerp(currentHeight, strawberrysuperHeight, lerpSpeed * Time.deltaTime);
				currentRadius = MathF.Lerp(currentRadius, strawberrysuperRadius, lerpSpeed * Time.deltaTime);
				PS.ResizeCapsuleCollider(this.ID, currentRadius, currentHeight);
				currentXform.x = MathF.Lerp(currentXform.x, strawberryscaledXform.x, lerpSpeed * Time.deltaTime);
				currentXform.y = MathF.Lerp(currentXform.y, strawberryscaledXform.y, lerpSpeed * Time.deltaTime);
				currentXform.z = MathF.Lerp(currentXform.z, strawberryscaledXform.z, lerpSpeed * Time.deltaTime);
				TransformSystem.SetScaling(this.ID, currentXform);
			}
			#endregion

			#region CHEATS
			// Close Game
			if (InputSystem.GetKeyDown(InputKeys.Escape))
			{
				Game.CloseGame();
			}
			#endregion

			dirVec.y = 0;
            if (dirVec != new vec3())
                dirVec = dirVec.Normalized;

            playerDirection = lastPlayerDirection + (int)CS.GetMainCameraRotation().y;
			playerDirection = (playerDirection % 360);

			if (dirVec != vec3.Zero)
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

			TransformSystem.SetRotation(this.ID, new vec3(0, playerDirection, 0));

			if (Key.ID != 0 && FinalPlatform.ID != 0)
			{
				if (PS.IsTriggerEnter(this.ID, Key.ID))
				{
					Key.SetActive(false);
					TransformSystem.SetPosition(FinalPlatform.ID, new vec3(100, 9, -302));
					Debug.Log("Key Collected");
				}
			}
		}

		private void Jump(vec3 JumpHeight)
		{
			//PhysicsSystem.SetLinearVelocity(this.ID, JumpHeight);
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
				if (other.GetComponent<JumpPad>().isActivated) isBoostedJump = true;
			}
			if (PS.IsCollisionStay(this.ID, otherID))
			{
				if (EngineGetTag(otherID) == "Ground" || EngineGetTag(otherID) == "JumpPad" || EngineGetTag(otherID) == "Platform"
					|| EngineGetTag(otherID) == "Blue" || EngineGetTag(otherID) == "BlueCollider")
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
				if (EngineGetTag(otherID) == "Blue")
				{
					PS.GetLinearVelocity(this.ID, out vec3 output);
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
                ResetToInitialPos();
            }
			Invulnerability = true;
        }

        public void ResetToInitialPos()
        {
            TransformSystem.SetPosition(this.ID, InitialPosition);
        }
    }
}
