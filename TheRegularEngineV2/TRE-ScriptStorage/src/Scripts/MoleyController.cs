using System;
using System.Collections.Generic;
using System.Globalization;
using System.Reflection;
using System.Runtime.CompilerServices;
using static System.Runtime.CompilerServices.RuntimeHelpers;
using System.Threading;
using GlmSharp;
using System.Diagnostics;

namespace TRE
{
	using PS = PhysicsSystem;
	using CS = CameraSystem;
	using MS = MeshRendererSystem;

	public class MoleyController : Entity
	{
		public PowerUpUI MyPowerUpUI;
		public PowerUpManager MyPowerManager;

		//Check if player is boosted jump
		public bool isBoostedJump = false;
		//check if player is on the ground (for now , just a plane)
		public bool isGrounded = true;
		//direction vector
		private vec3 dirVec;
		//Max Velocity vector
		private float maxVelocity = 30f;
		//Acceleration
		private float acceleration = 700f;
		//final velocity
		private vec3 finalVelocity = vec3.Zero;

		//Check if player is walking
		private bool isWalking = false;
		private bool walkingSFXPlayed = false;

		//check if player used super power
		public bool mainBlueberry = false;  // Scaling
		public bool mainStrawberry = false; // Shape
		public bool isScaled = false;

        #region Collider Variables
        private float defaultRadius = 2f;
		private float blueberrysuperRadius = 4f;
		private float strawberrysuperRadius = 4f;
		public float currentRadius = 2f;
		private float defaultHeight = 1f;
		private float blueberrysuperHeight = 2.4f;
		private float strawberrysuperHeight = 4.8f;
		public float currentHeight = 1f;
		public float currOffset = 3f;
        #endregion
        #region Player Transform Variables
        private Transform moleyTransform;
        private vec3 defaultXform = new vec3(0.75f, 0.75f, 0.75f);
		private vec3 blueberryscaledXform = new vec3(40f, 40f, 40f);
		private vec3 strawberryscaledXform = new vec3(0.5f, 0.5f, 0.5f);
		private vec3 currentXform = new vec3(0.75f, 0.75f, 0.75f);
        #endregion

        private int playerDirection = 0;
		private int lastPlayerDirection = 0;

		private float lerpSpeed = 5f;

        #region Jump Variables
        //Check if player is jumping at all
        public bool isJumping = false;
		//how long you hold the jump button to reach max jump height
		public float maxJumpButtomTime = 0.5f;
		public float currentJumpTime;
		public bool jumpCancelled = false;
		//how long after the player walks off the ground can he still jump
		private float coyoteTime = 0.2f;
		public float coyoteTimeCounter;
		//if player press space within this buffer time, they will still be able to jump even if they havent landed
		private float jumpBufferTime = 0.25f;
		public float jumpBufferCounter;
        private float maxJumpHeight = 70f;
        private float jumpHeight = 25f;
        #endregion

        //For camera controller
        //private Entity Key;
        //private Entity FinalPlatform;

        #region Audio Variables
        private ulong walkingSFX;
		private ulong jumpSFX;
		private ulong changesizeSFX;
		private ulong normalsizeSFX;
		private ulong fallingMaracaSFX;
		private ulong fallingHatSFX;
        #endregion

        public float elapsedTime = 0.0f;

        #region Respawn Variables
        private vec3 RespawnPoint = new vec3(0, 0, 0);
		private bool RespawnPlayer = false;
		private float RespawnTimer = 1.5f;
        public bool isDead = false;
        private vec3 InitialPosition = new vec3(0.0f, 0.0f, 0.0f);
        private vec3 OutofMapPos = new vec3(0.0f, 0.0f, 0.0f);
        private bool DroppingOutOfMap = false;
        #endregion

        #region Invulnerability Variables
        private bool Invulnerability = false;
		private float InvulCurrent = 1.0f;
		private float InvulPeriod = 1.0f;
		private float InvulBlinkCurrent = 0.1f;
		private float InvulBlinkPeriod = 0.1f;
        #endregion

		private Entity UIPopup2;
		private bool IsActivated = false;
		private bool HasBeenTriggeredBefore = false;

		//reference to holey
		private Entity holey_ref;

		public void Start()
		{
            #region UI variables
            MyPowerUpUI = ECSManager.FindEntityByName("LeftCharacter_HUD").GetComponent<PowerUpUI>();
			MyPowerManager = parenting.GetChildFromName("Power Manager").GetComponent<PowerUpManager>();
			MyPowerManager.MyPowerUpUI = MyPowerUpUI;
			MyPowerUpUI.UpdateUI(MyPowerManager.powerUps);

            UIPopup2 = ECSManager.FindEntityByName("PopupUI2");
            IsActivated = false;
            HasBeenTriggeredBefore = false;
            #endregion

            #region player Transform and Physics variables
            TransformSystem.SetRotation(this.ID, new vec3(0, 0, 0));
			PS.ConstrainRotationX(this.ID, true);
			PS.ConstrainRotationY(this.ID, true);
			PS.ConstrainRotationZ(this.ID, true);
            #endregion

            #region Respawn Variables
            TransformSystem.GetPosition(this.ID, out vec3 InitialPos);
			InitialPosition = InitialPos;
			OutofMapPos = InitialPos;
			OutofMapPos.y = InitialPos.y - 50.0f;
			moleyTransform = GetComponent<Transform>();

            RespawnPoint = moleyTransform.Position;
            RespawnPoint.y += 10.0f;
            #endregion

            #region Sound Variables
            walkingSFX = ECSManager.FindIDFromName("SFX_MoleyFootsteps");
			jumpSFX = ECSManager.FindIDFromName("SFX_MoleyJump");
			changesizeSFX = ECSManager.FindIDFromName("SFX_Fat");
			normalsizeSFX = ECSManager.FindIDFromName("SFX_NormalSize");
			fallingMaracaSFX = ECSManager.FindIDFromName("SFX_FallingMaraca");
			fallingHatSFX = ECSManager.FindIDFromName("SFX_FallingHat");
            #endregion

			holey_ref = ECSManager.FindEntityByName("Holey");
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

			//Transform variables
			TransformSystem.GetPosition(this.ID, out vec3 pos);
			TransformSystem.SetRotation(this.ID, new vec3(0, 0, 0));

			//Movement variables
			PS.GetLinearVelocity(this.ID, out vec3 currVelocity);

            #region respawn mechanics
            if (pos.y < OutofMapPos.y)
			{
				isDead = true;
				DroppingOutOfMap = true;
			}
			else
			{
				DroppingOutOfMap = false;
			}

			if (pos.y < (InitialPosition.y - 50.0f))
			{
				if (!holey_ref.GetComponent<HoleyController>().GetIsDead() && isDead)
				{
					RespawnPlayer = true;
					if (MyPowerManager.powerUps.Count > 0)
					{
						MyPowerManager.LoseMain();
						isScaled = false;
					}
				}
				// else do not respawn player since both are dead
			}
            #endregion
            #region Movement
			dirVec = vec3.Zero;

            if (DroppingOutOfMap)
			{
				dirVec.x = 0.0f;
				dirVec.z = 0.0f;
			}
			else if (DroppingOutOfMap == false)
			{
				if (InputSystem.GetKeyHold(InputKeys.W))
				{
					dirVec += CS.GetMainCameraForwardVec();
					lastPlayerDirection = 0;
				}
				if (InputSystem.GetKeyHold(InputKeys.S))
				{
					dirVec -= CS.GetMainCameraForwardVec();
					lastPlayerDirection = 180;
				}
				if (InputSystem.GetKeyHold(InputKeys.A))
				{
					dirVec += CS.GetMainCameraRightVec();
					lastPlayerDirection = 90;
				}
				if (InputSystem.GetKeyHold(InputKeys.D))
				{
					dirVec -= CS.GetMainCameraRightVec();
					lastPlayerDirection = 270;
				}
				if (InputSystem.GetKeyHold(InputKeys.W))
				{
					if (InputSystem.GetKeyHold(InputKeys.D))
					{
						lastPlayerDirection = 315;
					}
					if (InputSystem.GetKeyHold(InputKeys.A))
					{
						lastPlayerDirection = 45;
					}
				}

				if (InputSystem.GetKeyHold(InputKeys.S))
				{
					if (InputSystem.GetKeyHold(InputKeys.D))
					{
						lastPlayerDirection = 225;
					}
					if (InputSystem.GetKeyHold(InputKeys.A))
					{
						lastPlayerDirection = 135;
					}
				}

				//When the space bar is released, the player will stop mid jump
				if (jumpCancelled && isJumping && currVelocity.y > 0)
				{
					currVelocity.y = 0;
				}
				//check if player is on the ground then reset coyote time
				if (isGrounded)
				{
					coyoteTimeCounter = coyoteTime;
				}
				//check if player is not on the ground then reduce coyote time
				else
				{
					coyoteTimeCounter -= Time.deltaTime;
				}
				//check if space is pressed within the buffer time
				if (InputSystem.GetKeyPress(InputKeys.Space))
                {
                    jumpBufferCounter = jumpBufferTime;
					//Debug.Log("Jump Pressed");
                }
				//count down the buffer time
                else
                {
                    jumpBufferCounter -= Time.deltaTime;
                    //Debug.Log("Jump Buffer Time 2: " + jumpBufferCounter);
                }
				//check if player is jumping
				if (isJumping)
				{
					//check if space is released then cancel jump
					if (InputSystem.GetKeyRelease(InputKeys.Space))
					{
						jumpCancelled = true;
						coyoteTimeCounter = 0f;
						//Debug.Log("Jump Cancelled");
					}
					//check if space is held down and jump time is not over
					if (currentJumpTime > maxJumpButtomTime)
					{
						isJumping = false;

						//Debug.Log("Jump ran out");
                    }
					currentJumpTime += Time.deltaTime;
				}
				//check if player is on the ground and space is not released
				else
				{
                    if (InputSystem.GetKeyRelease(InputKeys.Space))
                    {
						isJumping = false;
                    }
                }
				//jump buffer time and coyote time is still active
                if (coyoteTimeCounter > 0f && jumpBufferCounter > 0f)
				{
					isWalking = false;

					vec3 maxHeight = new vec3(0, 70, 0);
					// Boosted Jump
					if (isBoostedJump)
					{
						maxHeight = new vec3(0, 150, 0);
					}

					Jump(maxHeight);
					if (ECSManager.IsValidEntity(jumpSFX))
					{
						AudioSystem.Play(jumpSFX);
					}

					isJumping = true;
					jumpCancelled = false;
					currentJumpTime = 0;
					jumpBufferCounter = 0;
                }
			}
			#endregion

			#region Audio
			if (InputSystem.GetKeyHold(InputKeys.W) || InputSystem.GetKeyHold(InputKeys.S) ||
			InputSystem.GetKeyHold(InputKeys.A) || InputSystem.GetKeyHold(InputKeys.D))
			{
				isWalking = true;
			}

			if (!(InputSystem.GetKeyHold(InputKeys.W) || InputSystem.GetKeyHold(InputKeys.S) ||
				InputSystem.GetKeyHold(InputKeys.A) || InputSystem.GetKeyHold(InputKeys.D)))
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
			if (InputSystem.GetKeyPress(InputKeys.Q))
			{
				MyPowerManager.SwapPowerUps();
				MyPowerUpUI.UpdateUI(MyPowerManager.powerUps);
				isScaled = false;
			}
			#endregion

			#region Drop
			// Check if can trigger ability
			if (InputSystem.GetKeyPress(InputKeys.LeftShift))
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
			if (InputSystem.GetKeyPress(InputKeys.E))
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
				//Default model
				GetComponent<MeshRenderer>().Material = "Red_Moley.material";
				dirVec.y = 0;
				if (dirVec != vec3.Zero && isGrounded)
				{
					//Walking state
					GetComponent<MeshRenderer>().Mesh = "mole_walk.fbx";
					GetComponent<MeshRenderer>().AnimMaterial = "RedCharacter_Animation.material";
					if (HasComponent<Animation>())
						GetComponent<Animation>().AnimationSpeed = 0.5f;
				}
				else if (!isGrounded)
				{
					GetComponent<MeshRenderer>().Mesh = "mole_jump.fbx";
					GetComponent<MeshRenderer>().AnimMaterial = "RedCharacter_Animation.material";
					if (HasComponent<Animation>())
						GetComponent<Animation>().AnimationSpeed = 0.5f;
				}
				else
				{
					//Red Animation Material
					GetComponent<MeshRenderer>().Mesh = "mole_idle.fbx";
					GetComponent<MeshRenderer>().AnimMaterial = "RedCharacter_Animation.material";
					if (HasComponent<Animation>())
						GetComponent<Animation>().AnimationSpeed = 0.25f;

					//TransformSystem.SetScaling(this.ID, new vec3(0.03f, 0.03f, 0.03f));
				}
				currentHeight = MathF.Lerp(currentHeight, defaultHeight, lerpSpeed * Time.deltaTime);
				currentRadius = MathF.Lerp(currentRadius, defaultRadius, lerpSpeed * Time.deltaTime);
				currOffset = MathF.Lerp(currOffset, 3, lerpSpeed * Time.deltaTime);
				currentXform = new vec3(0.03f, 0.03f, 0.03f);

				PS.ResizeCapsuleCollider(this.ID, currentRadius, currentHeight);
				PS.UpdateColliderOffset(this.ID, new vec3(0, currOffset, 0));
				TransformSystem.SetScaling(this.ID, currentXform);
			}
			else if (mainBlueberry)
			{
				//Fat model
				if (MS.IsCurrentMesh(this.ID, "Moley_Blueberry.fbx") == false)
				{
					GetComponent<MeshRenderer>().Mesh = "Moley_Blueberry.fbx";
					GetComponent<MeshRenderer>().Material = "Moley_Blueberry.material";
				}
				currentHeight = MathF.Lerp(currentHeight, blueberrysuperHeight, lerpSpeed * Time.deltaTime);
				currentRadius = MathF.Lerp(currentRadius, blueberrysuperRadius, lerpSpeed * Time.deltaTime);
				currOffset = MathF.Lerp(currOffset, 1, lerpSpeed * Time.deltaTime);
				currentXform = blueberryscaledXform;

				PS.UpdateColliderOffset(this.ID, new vec3(0, currOffset, 0));
				PS.ResizeCapsuleCollider(this.ID, currentRadius, currentHeight);
				TransformSystem.SetScaling(this.ID, currentXform);
			}
			else if (mainStrawberry)
			{
				//flat model
				if (MS.IsCurrentMesh(this.ID, "Moley_Strawberry.fbx") == false)
				{
					GetComponent<MeshRenderer>().Mesh = "Moley_Strawberry.fbx";
					GetComponent<MeshRenderer>().Material = "Moley_Strawberry.material";
				}
				currentHeight = MathF.Lerp(currentHeight, strawberrysuperHeight, lerpSpeed * Time.deltaTime);
				currentRadius = MathF.Lerp(currentRadius, strawberrysuperRadius, lerpSpeed * Time.deltaTime);
				currentXform.x = MathF.Lerp(currentXform.x, strawberryscaledXform.x, lerpSpeed * Time.deltaTime);
				currentXform.y = MathF.Lerp(currentXform.y, strawberryscaledXform.y, lerpSpeed * Time.deltaTime);
				currentXform.z = MathF.Lerp(currentXform.z, strawberryscaledXform.z, lerpSpeed * Time.deltaTime);

				PS.ResizeCapsuleCollider(this.ID, currentRadius, currentHeight);
				TransformSystem.SetScaling(this.ID, currentXform);
			}
			#endregion

			//Do NOT REMOVE THIS for some reason it stops the mole when its tall from flying idk dont ask me
			dirVec.y = 0;
			dirVec = dirVec.NormalizedSafe;

			#region CHEATS
			// Close Game
			if (InputSystem.GetKeyHold(InputKeys.Escape))
			{
				Game.CloseGame();
			}
			#endregion


			playerDirection = (int)lastPlayerDirection + (int)CS.GetMainCameraRotation().y;
			playerDirection = (playerDirection % 360);

			/*else if (dirVec.x == 0 && dirVec.z == 0)
			{
				// If no input, slow down
				finalVelocity = currVelocity * 0.9f;
				PS.SetLinearVelocity(this.ID, finalVelocity);
			}*/

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

			TransformSystem.SetRotation(this.ID, new vec3(0, playerDirection, 0));

			isGrounded = false;

			if (RespawnPlayer)
			{
				if (RespawnTimer > 0 && isDead)
				{
					RespawnTimer -= Time.deltaTime;
				}
				else
				{
					Respawn();
					PS.SetLinearVelocity(ID, vec3.Zero);
					isDead = false;
					RespawnPlayer = false;
					RespawnTimer = 1.5f;
				}
			}

			if (!HasBeenTriggeredBefore)
			{
				if (IsActivated)
				{
					UIPopup2.GetComponent<SpriteRenderer>().isVisible = true;
				}

				if (IsActivated && InputSystem.GetKeyHold(InputKeys.Space))
				{
					UIPopup2.GetComponent<SpriteRenderer>().isVisible = false;
					HasBeenTriggeredBefore = true;
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

				if (other.ID == ECSManager.FindIDFromName("FallingMaracca"))
				{
					if (ECSManager.IsValidEntity(fallingMaracaSFX))
						AudioSystem.Play(fallingMaracaSFX);
				}
				else if (other.ID == ECSManager.FindIDFromName("FallingHat"))
				{
					if (ECSManager.IsValidEntity(fallingHatSFX))
						AudioSystem.Play(fallingHatSFX);
				}
				else
				{
					//rollingobstaclesfx
				}
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
			//For jumping
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
			if (EngineGetTag(otherID) == "Blue")
			{
				PS.GetLinearVelocity(this.ID, out vec3 output);
				if (output.y > maxJumpHeight)
					output.y = maxJumpHeight;
				PS.SetLinearVelocity(this.ID, output);
			}
			// No longer boosted if leave jumppad
			if (other.CompareTag("JumpPad"))
			{
				isBoostedJump = false;
				isGrounded = false;
			}
			if (other.CompareTag("Ground") || other.CompareTag("Platform")
				|| other.CompareTag("Blue") || other.CompareTag("BlueCollider"))
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
			if (Invulnerability)
			{
				IsActivated = true;
				return;
			}

			if (MyPowerManager.powerUps.Count > 0)
			{
				MyPowerManager.LoseMain();
				isScaled = false;
			}
			else
			{
				isDead = true;
				RespawnPlayer = true;
			}
			Invulnerability = true;
		}

		public void ResetToInitialPos()
		{
			TransformSystem.SetPosition(this.ID, InitialPosition);
		}

		private void Respawn()
		{
			moleyTransform.Position = RespawnPoint;
			Invulnerability = true;
		}

		public void SetRespawnPoint(vec3 pos)
		{
			RespawnPoint = pos;
		}

		public bool GetIsDead()
		{
			return isDead;
		}
	}
}
