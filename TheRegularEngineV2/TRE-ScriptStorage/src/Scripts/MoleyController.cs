using GlmSharp;
using System;

namespace TRE
{
	using AS = AudioSystem;
	using CS = CameraSystem;
	using IS = InputSystem;
	using MS = MeshRendererSystem;
	using PS = PhysicsSystem;
	using TS = TransformSystem;
	using PS3D = ParticleSystem3D;

	public class MoleyController : Entity
	{
		public PauseMenu MyPauseMenu;
		public PowerUpUI MyPowerUpUI;
		public PowerUpManager MyPowerManager;

		//Check if player is boosted jump
		public bool isBoostedJump = false;
		//Check if player is on the ground
		public bool isGrounded = true;
		//direction vector
		private vec3 dirVec;
		//Max Velocity vector
		private float maxVelocity = 30f;
		//Acceleration
		private float acceleration = 700f;
		//final velocity
		private vec3 finalVelocity = vec3.Zero;
		//Original Scale
		private vec3 oriScale;

		//Check if player is walking
		public bool isWalking = false;

		//check if player used super power
		public bool mainBlueberry = false;  // Scaling
		public bool mainStrawberry = false; // Shape
		public bool isScaled = false;

		// Controllable
		public bool isControllable = true;
		private bool IsControllerConnected = false;
		private bool changeUI = false;
		private bool lastControllerConnected = false;
        private Entity CharacterUI;

		// the controls/keys that THIS player (Moley) will use
		#region Player Controls
		const InputKeys playerUpKey = InputKeys.W;
		const InputKeys playerDownKey = InputKeys.S;
		const InputKeys playerLeftKey = InputKeys.A;
		const InputKeys playerRightKey = InputKeys.D;
		const InputKeys playerJumpKey = InputKeys.Space;
		const InputKeys playerSwapKey = InputKeys.Q;
		const InputKeys playerDropKey = InputKeys.LeftShift;
		const InputKeys playerAbilityKey = InputKeys.E;
		#endregion

        private int ControllerNumber = 0;

		#region Collider Variables
		private float defaultRadius = 2f;
		private float blueberrysuperRadius = 2f;
		private float strawberrysuperRadius = 2f;
		public float currentRadius = 2f;
		private float defaultHeight = 1f;
		private float blueberrysuperHeight = 1f;
		private float strawberrysuperHeight = 2f;
		public float currentHeight = 1f;
		public float currOffset = 3f;
		#endregion

		#region Player Transform Variables
		private Transform moleyTransform;
		private vec3 defaultXform = new vec3(0.75f, 0.75f, 0.75f);
		private vec3 blueberryscaledXform = new vec3(0.4f, 0.4f, 0.4f);
		private vec3 strawberryscaledXform = new vec3(0.4f, 0.4f, 0.4f);
		private vec3 currentXform = new vec3(0.75f, 0.75f, 0.75f);
		#endregion

		public int turnDirection = 0;
		private int playerDirection = 0;
		private int lastPlayerDirection = 0;

		private float lerpSpeed = 5f;

		#region Jump Variables
		private float maxJumpHeight = 70f;
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
		private float jumpHeight = 25f;
		#endregion

		#region Audio Variables
		private ulong walkingSFX;
		//private ulong walkingSFX1;
		//private ulong walkingSFX2;
		//private ulong walkingSFX3;
		//private ulong walkingSFX4;
		//private ulong walkingSFX5;
		//private ulong walkingSFX6;
		private ulong jumpSFX;
		private ulong changesizeSFX;
		private ulong normalsizeSFX;
		private ulong fallingMaracaSFX;
		private ulong fallingHatSFX;
		private ulong fallSFX;
		private ulong cheeringSFX;
		private ulong hurtSFX;
		private ulong landingSFX;

		private bool walkingSFXPlayed = false;
		private bool fallingSFXPlayed = false;
		private bool landingSFXPlayed = false;

		private float walkingSFXVolume;
		#endregion

		#region Respawn Variables
		private vec3 RespawnPoint = vec3.Zero;
		private bool RespawnPlayer = false;
		private float RespawnTimer = 1.5f;
		public bool isDead = false;
		private vec3 InitialPosition = vec3.Zero;
		private vec3 OutofMapPos = vec3.Zero;
		private bool DroppingOutOfMap = false;
		#endregion

		#region Invulnerability Variables
		private bool Invulnerability = false;
		private float InvulCurrent = 1.0f;
		private float InvulPeriod = 1.0f;
		private float InvulBlinkCurrent = 0.1f;
		private float InvulBlinkPeriod = 0.1f;
		#endregion

		//Holey Reference
		private Entity holey_ref;

		//Moley Particles
		private Entity moley_dust;

		//these variables are enclusive to Moley
		private Entity UIPopup2;
		private bool IsActivated = false;
		private bool HasBeenTriggeredBefore = false;

		//For camera controller
		//private Entity Key;
		//private Entity FinalPlatform;

		// Cheats
		public bool keepInventory = false;
		public bool creativeMode = false;

		//Materials and Meshes
		//Default
		private const string defaultMaterial = "Red_Moley.material";
        private const string defaultWalkingMesh = "mole_walk.fbx";
        private const string defaultJumpingMesh = "mole_jump.fbx";
        private const string defaultIdleMesh = "mole_idle.fbx";
        private const string defaultAnimationMaterial = "RedCharacter_Animation.material";

		//Blueberry
        private const string blueberryMaterial = "Moley_Blueberry.material";
		private const string blueberryMesh = "Moley_Blueberry.fbx";

        //Strawberry
        private const string strawberryMaterial = "Moley_Strawberry.material";
		private const string strawberryMesh = "Moley_Strawberry.fbx";
        public void Start()
		{
			#region UI Variables
			MyPauseMenu = ECSManager.FindEntityByName("PauseMenu").GetComponent<PauseMenu>();
			MyPowerUpUI = ECSManager.FindEntityByName("LeftCharacter_HUD").GetComponent<PowerUpUI>();
			MyPowerManager = parenting.GetChildFromName("Power Manager").GetComponent<PowerUpManager>();
			MyPowerManager.MyPowerUpUI = MyPowerUpUI;
			MyPowerUpUI.UpdateUI(MyPowerManager.powerUps);
			CharacterUI = ECSManager.FindEntityByName("LeftCharacter_HUD");
			#endregion

			UIPopup2 = ECSManager.FindEntityByName("PopupUI2");
			IsActivated = false;
			HasBeenTriggeredBefore = false;

			#region Player Transform and Physics Variables
			moleyTransform = GetComponent<Transform>();
			TS.SetRotation(this.ID, vec3.Zero);
			PS.ConstrainRotationX(this.ID, true);
			PS.ConstrainRotationY(this.ID, true);
			PS.ConstrainRotationZ(this.ID, true);
			#endregion

			#region Respawn Variables
			TS.GetPosition(this.ID, out vec3 InitialPos);
			InitialPosition = InitialPos;
			OutofMapPos = InitialPos;
			OutofMapPos.y = InitialPos.y - 300.0f;

			RespawnPoint = moleyTransform.Position;
			RespawnPoint.y += 10.0f;
			#endregion

			#region Sound Variables
			walkingSFX = ECSManager.FindIDFromName("SFX_MoleyFootsteps");
			//walkingSFX1 = ECSManager.FindIDFromName("SFX_Footsteps1");
			//walkingSFX2 = ECSManager.FindIDFromName("SFX_Footsteps2");
			//walkingSFX3 = ECSManager.FindIDFromName("SFX_Footsteps3");
			//walkingSFX4 = ECSManager.FindIDFromName("SFX_Footsteps4");
			//walkingSFX5 = ECSManager.FindIDFromName("SFX_Footsteps5");
			//walkingSFX6 = ECSManager.FindIDFromName("SFX_Footsteps6");
			jumpSFX = ECSManager.FindIDFromName("SFX_MoleyJump");
			changesizeSFX = ECSManager.FindIDFromName("SFX_Fat");
			normalsizeSFX = ECSManager.FindIDFromName("SFX_NormalSize");
			fallingMaracaSFX = ECSManager.FindIDFromName("SFX_FallingMaraca");
			fallingHatSFX = ECSManager.FindIDFromName("SFX_FallingHat");
			fallSFX = ECSManager.FindIDFromName("SFX_MoleyFall");
			cheeringSFX = ECSManager.FindIDFromName("SFX_Moley_BoostedJump");
			hurtSFX = ECSManager.FindIDFromName("SFX_MoleyHurt1");
			landingSFX = ECSManager.FindIDFromName("SFX_MoleyLand");
			walkingSFXVolume = AS.GetVolume(walkingSFX);
			#endregion

			holey_ref = ECSManager.FindEntityByName("Holey");
			oriScale = moleyTransform.Scale;

			moley_dust = ECSManager.FindEntityByName("Moley_Dust");

			//Init all the meshes and materials
			GetComponent<MeshRenderer>().Mesh = blueberryMesh;
			GetComponent<MeshRenderer>().Material = blueberryMaterial;
			//GetComponent<MeshRenderer>().Mesh = strawberryMesh;
			//GetComponent<MeshRenderer>().Material = strawberryMaterial;
			GetComponent<MeshRenderer>().Mesh = defaultWalkingMesh;
			GetComponent<MeshRenderer>().Material = defaultMaterial;
			GetComponent<MeshRenderer>().AnimMaterial = defaultAnimationMaterial;
		}

		public void Update()
		{
            // check if controller connected
            IsControllerConnected = IS.GetControllerConnected(ControllerNumber);
            if (lastControllerConnected != IsControllerConnected)
            {
                lastControllerConnected = IsControllerConnected;
                changeUI = true;
            }

            if (changeUI && IsControllerConnected && CharacterUI != null)
            {
                changeUI = false;
                CharacterUI.GetComponent<SpriteRenderer>().Texture = "CharacterUI_Right_Controller.png";
            }

            if (changeUI && !IsControllerConnected && CharacterUI != null)
            {
                changeUI = false;
                CharacterUI.GetComponent<SpriteRenderer>().Texture = "CharacterUI_Right.png";
            }


            CheckControllability();

			HandleInvulnerability();

			//Transform variables
			TS.GetPosition(this.ID, out vec3 pos);
			//TransformSystem.SetRotation(this.ID, new vec3(0, 0, 0));

			//Movement variables
			PS.GetLinearVelocity(this.ID, out vec3 currVelocity);

			HandleRespawn(pos);

			dirVec = vec3.Zero;

			HandleMovement(ref currVelocity);

			HandleAudio();

			HandleSwap();

			HandleDrop();

			HandleAbilities();

			HandleParticles(ref currVelocity);

			//Do NOT REMOVE THIS for some reason it stops the mole when its tall from flying idk dont ask me
			dirVec.y = 0;
			dirVec = dirVec.NormalizedSafe;

			#region CHEATS
			// Close Game
			//if (InputSystem.GetKeyHold(InputKeys.Escape))
			//{
			//	Game.CloseGame();
			//}
			#endregion

			playerDirection = (int)lastPlayerDirection + turnDirection;
			playerDirection = playerDirection % 360;

			// Only set velocity if not respawning
			if (!RespawnPlayer)
			{
				if (Math.Sqrt(currVelocity.x * currVelocity.x + currVelocity.z * currVelocity.z) <= maxVelocity)
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

			// here?
			IsInsideCirclePush(holey_ref.GetComponent<Transform>().Position);

			TS.SetRotation(this.ID, new vec3(0, playerDirection, 0));

			//isGrounded = false;

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
					fallingSFXPlayed = false;
				}
			}

			#region UI Popup Region
			if (!HasBeenTriggeredBefore)
			{
				if (IsActivated)
				{
					UIPopup2.GetComponent<SpriteRenderer>().isVisible = true;
				}

				if (IsActivated && IS.GetKeyHold(InputKeys.Space))
				{
					UIPopup2.GetComponent<SpriteRenderer>().isVisible = false;
					HasBeenTriggeredBefore = true;
				}
			}
			#endregion
		}

		private void Jump(vec3 JumpHeight)
		{
			//PhysicsSystem.SetLinearVelocity(this.ID, JumpHeight);
			PS.AddForce(this.ID, JumpHeight, ForceMode.VelocityChange);
		}

		private void OnCollisionStay(System.UInt64 otherID)
		{
			//isGrounded = false;

			Entity other = new Entity(otherID);
			// Make it loose one of it's powerups
			if (other.CompareTag("FallingObstacle") || other.CompareTag("RollingObstacle"))
			{
				TakeDamage();

				if (other.ID == ECSManager.FindIDFromName("FallingMaracca"))
				{
					if (ECSManager.IsValidEntity(fallingMaracaSFX))
						AS.Play(fallingMaracaSFX);
				}
				//else if (other.ID == ECSManager.FindIDFromName("FallingHat"))
				//{
				//	if (ECSManager.IsValidEntity(fallingHatSFX))
				//		AS.Play(fallingHatSFX);
				//}
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

		private void HandleInvulnerability()
		{
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
		}

		private void HandleRespawn(vec3 playerPosition)
		{
			if (playerPosition.y < OutofMapPos.y)
			{
				PS.GetLinearVelocity(this.ID, out vec3 currVelocity);
				if (currVelocity.x > 0 || currVelocity.z > 0)
				{
					currVelocity.x = 0;
					currVelocity.z = 0;
					PS.SetLinearVelocity(this.ID, currVelocity);
				}

				isDead = true;
				DroppingOutOfMap = true;
			}
			else
			{
				DroppingOutOfMap = false;
			}

			if (playerPosition.y < (InitialPosition.y - 50.0f))
			{
				if (!holey_ref.GetComponent<HoleyController>().GetIsDead() && isDead)
				{
					RespawnPlayer = true;
					if (MyPowerManager.powerUps.Count > 0 && !keepInventory)
					{
						MyPowerManager.LoseMain();
						isScaled = false;
					}
				}
				// else do not respawn player since both are dead
			}
		}

		private void HandleMovement(ref vec3 currVelocity)
		{
			// Ignores if dead
			if (isDead) return;

			if (DroppingOutOfMap)
			{
				dirVec.x = 0.0f;
				dirVec.z = 0.0f;
			}
			else if (DroppingOutOfMap == false)
			{
				// KB Connected
				if (!MyPauseMenu.isPaused && isControllable && !IsControllerConnected)
				{
					if (IS.GetKeyHold(playerUpKey))
					{
						dirVec += CS.GetMainCameraForwardVec();
						lastPlayerDirection = 0;
						isWalking = true;
					}

					if (IS.GetKeyHold(playerDownKey))
					{
						dirVec -= CS.GetMainCameraForwardVec();
						lastPlayerDirection = 180;
						isWalking = true;
					}

					if (IS.GetKeyHold(playerLeftKey))
					{
						dirVec += CS.GetMainCameraRightVec();
						lastPlayerDirection = 90;
						isWalking = true;
					}

					if (IS.GetKeyHold(playerRightKey))
					{
						dirVec -= CS.GetMainCameraRightVec();
						lastPlayerDirection = 270;
						isWalking = true;
					}

					if (IS.GetKeyHold(playerUpKey))
					{
						if (IS.GetKeyHold(playerRightKey))
							lastPlayerDirection = 315;

						if (IS.GetKeyHold(playerLeftKey))
							lastPlayerDirection = 45;
					}

					if (IS.GetKeyHold(playerDownKey))
					{
						if (IS.GetKeyHold(playerRightKey))
							lastPlayerDirection = 225;

						if (IS.GetKeyHold(playerLeftKey))
							lastPlayerDirection = 135;
					}

					if (!IS.GetKeyHold(playerUpKey) && !IS.GetKeyHold(playerDownKey) && !IS.GetKeyHold(playerLeftKey) &&
						!IS.GetKeyHold(playerRightKey))
					{
						isWalking = false;
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
					if (IS.GetKeyPress(playerJumpKey))
					{
						jumpHeight += Time.deltaTime;
						jumpBufferCounter = jumpBufferTime;
					}
					//count down the buffer time
					else
					{
						jumpBufferCounter -= Time.deltaTime;
					}
					//check if player is jumping
					if (isJumping)
					{
						//check if space is released then cancel jump
						if (IS.GetKeyRelease(playerJumpKey))
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
						if (IS.GetKeyRelease(playerJumpKey))
						{
							isJumping = false;
						}
					}
					//jump buffer time and coyote time is still active
					if (coyoteTimeCounter > 0f && jumpBufferCounter > 0f && isControllable)
					{
						isWalking = false;
						//if something break comment this line below out
						currVelocity.y = 0;

						vec3 maxHeight = new vec3(0, 70, 0);
						// Boosted Jump
						if (isBoostedJump)
						{
							maxHeight = new vec3(0, 120, 0);
						}

						Jump(maxHeight);

						if (ECSManager.IsValidEntity(jumpSFX) && ECSManager.IsValidEntity(cheeringSFX))
						{
							if (isBoostedJump)
								AS.Play(cheeringSFX);
							else
								AS.Play(jumpSFX);
						}

						isJumping = true;
						jumpCancelled = false;
						currentJumpTime = 0;
						jumpBufferCounter = 0;
					}
				}

				// controller is connected
				else if (!MyPauseMenu.isPaused && isControllable && IsControllerConnected)
				{
					float x = IS.GetControllerStickX(ControllerNumber, false); // false for left thumbstick
					float y = IS.GetControllerStickY(ControllerNumber, false); // false for left thumbstick

					// calculate the direction vector
					isWalking = x != 0 || y != 0;

					// calculate the angle of the direction vector
					if (isWalking)
					{
						lastPlayerDirection = (int)(Math.Atan2(y, x) * 180/Math.PI - 90 + 360) % 360;
						

						// handle the dirVec
						if(y > 0)
							dirVec += CS.GetMainCameraForwardVec();
						if(y < 0)
                            dirVec -= CS.GetMainCameraForwardVec();
						if(x > 0)
							dirVec -= CS.GetMainCameraRightVec();
						if(x < 0)
							dirVec += CS.GetMainCameraRightVec();
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
					if (IS.GetControllerButtonTriggered(ControllerNumber, IS.Button.A))
					{
						jumpHeight += Time.deltaTime;
						jumpBufferCounter = jumpBufferTime;
					}
					//count down the buffer time
					else
					{
						jumpBufferCounter -= Time.deltaTime;
					}
					//check if player is jumping
					if (isJumping)
					{
						//check if space is released then cancel jump
						if (IS.GetControllerButtonReleased(ControllerNumber, IS.Button.A))
						{
							jumpCancelled = true;
							coyoteTimeCounter = 0f;
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
						if (IS.GetControllerButtonReleased(ControllerNumber, IS.Button.A))
						{
							isJumping = false;
						}
					}
					//jump buffer time and coyote time is still active
					if (coyoteTimeCounter > 0f && jumpBufferCounter > 0f && isControllable)
					{
						isWalking = false;
						//if something break comment this line below out
						currVelocity.y = 0;

						vec3 maxHeight = new vec3(0, 70, 0);
						// Boosted Jump
						if (isBoostedJump)
						{
							maxHeight = new vec3(0, 120, 0);
						}

						Jump(maxHeight);

						if (ECSManager.IsValidEntity(jumpSFX) && ECSManager.IsValidEntity(cheeringSFX))
						{
							if (isBoostedJump)
								AS.Play(cheeringSFX);
							else
								AS.Play(jumpSFX);
						}

						isJumping = true;
						jumpCancelled = false;
						currentJumpTime = 0;
						jumpBufferCounter = 0;
					}
				}
			}
		}

		private void HandleAudio()
		{
			//bool isMovementKeyHeld = IS.GetKeyHold(playerUpKey) || IS.GetKeyHold(playerDownKey) ||
			//	IS.GetKeyHold(playerLeftKey) || IS.GetKeyHold(playerRightKey);

			//// if the player holds any movement key, they are walking
			//isWalking = isMovementKeyHeld;

			if (ECSManager.IsValidEntity(walkingSFX))
			{
				if (isWalking && walkingSFXPlayed == false)
				{
					AS.SetVolume(walkingSFX, VolumeVariation(walkingSFXVolume, 0.2f));
					AS.Play(walkingSFX);
					walkingSFXPlayed = true;
				}

				if (!isWalking || !isGrounded)
				{
					AS.Stop(walkingSFX);
					walkingSFXPlayed = false;
				}
			}

			if (DroppingOutOfMap)
			{
				if (ECSManager.IsValidEntity(fallSFX) && !fallingSFXPlayed)
				{
					fallingSFXPlayed = true;
					AS.Play(fallSFX);
				}
			}

			if (isGrounded && landingSFXPlayed == false)
			{
				AS.Play(landingSFX);
				landingSFXPlayed = true;
			}
			else if(!isGrounded)
			{
				AS.Stop(landingSFX);
				landingSFXPlayed = false;
			}
		}

		private void HandleSwap()
		{
			// Check if can swap ability
			if (IS.GetKeyPress(playerSwapKey))
			{
				MyPowerManager.SwapPowerUps();
				MyPowerUpUI.UpdateUI(MyPowerManager.powerUps);
				isScaled = false;
			}

			if (IS.GetControllerButtonTriggered(ControllerNumber, InputSystem.Button.Y))
			{
				MyPowerManager.SwapPowerUps();
				MyPowerUpUI.UpdateUI(MyPowerManager.powerUps);
				isScaled = false;
			}
		}

		private void HandleDrop()
		{
			// Check if can trigger ability
			if (IS.GetKeyPress(playerDropKey))
			{
				MyPowerManager.DropMain();
				isScaled = false;
			}

			if (IS.GetControllerButtonTriggered(ControllerNumber, InputSystem.Button.B))
			{
				MyPowerManager.DropMain();
				isScaled = false;
			}
		}

		private void HandleAbilities()
		{
			if (isDead) return;

			mainBlueberry = MyPowerManager.powerUps.Count > 0 && MyPowerManager.powerUps[0].CompareTag("Blueberry");
			mainStrawberry = MyPowerManager.powerUps.Count > 0 && MyPowerManager.powerUps[0].CompareTag("Strawberry");

			if (isScaled && !mainBlueberry && !mainStrawberry)
			{
				isScaled = false;
			}

			// Check if can trigger ability
			if (IS.GetKeyPress(playerAbilityKey))
			{
				if (mainBlueberry || mainStrawberry)
				{
					isScaled = !isScaled;

					if (isScaled)
					{
						if (ECSManager.IsValidEntity(changesizeSFX))
							AS.Play(changesizeSFX);
					}
					else
					{
						if (ECSManager.IsValidEntity(normalsizeSFX))
							AS.Play(normalsizeSFX);
					}
				}
			}

			if (IS.GetControllerButtonTriggered(ControllerNumber, InputSystem.Button.X))
			{
				if (mainBlueberry || mainStrawberry)
				{
					isScaled = !isScaled;

					if (isScaled)
					{
						if (ECSManager.IsValidEntity(changesizeSFX))
							AS.Play(changesizeSFX);
					}
					else
					{
						if (ECSManager.IsValidEntity(normalsizeSFX))
							AS.Play(normalsizeSFX);
					}
				}
			}

			if (isScaled == false || (!mainBlueberry && !mainStrawberry))
			{
				//Default model
				GetComponent<MeshRenderer>().Material = defaultMaterial;
				dirVec.y = 0;
				if (dirVec != vec3.Zero && isGrounded)
				{
					//Walking state
					GetComponent<MeshRenderer>().Mesh = defaultWalkingMesh;
					GetComponent<MeshRenderer>().AnimMaterial = defaultAnimationMaterial;
					if (HasComponent<Animation>())
						GetComponent<Animation>().AnimationSpeed = 0.5f;
				}
				else if (!isGrounded)
				{
					GetComponent<MeshRenderer>().Mesh = defaultJumpingMesh;
					GetComponent<MeshRenderer>().AnimMaterial = defaultAnimationMaterial;
					if (HasComponent<Animation>())
						GetComponent<Animation>().AnimationSpeed = 0.5f;
				}
				else
				{
					//Red Animation Material
					GetComponent<MeshRenderer>().Mesh = defaultIdleMesh;
					GetComponent<MeshRenderer>().AnimMaterial = defaultAnimationMaterial;
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
				TS.SetScaling(this.ID, currentXform);
			}
			else if (mainBlueberry)
			{
				//Fat model
				if (MS.IsCurrentMesh(this.ID, blueberryMesh) == false)
				{
					GetComponent<MeshRenderer>().Mesh = blueberryMesh;
					GetComponent<MeshRenderer>().Material = blueberryMaterial;
				}
				currentHeight = MathF.Lerp(currentHeight, blueberrysuperHeight, lerpSpeed * Time.deltaTime);
				currentRadius = MathF.Lerp(currentRadius, blueberrysuperRadius, lerpSpeed * Time.deltaTime);
				currOffset = MathF.Lerp(currOffset, -1.2f, lerpSpeed * Time.deltaTime);
				currentXform = blueberryscaledXform;

				PS.UpdateColliderOffset(this.ID, new vec3(0, currOffset, 0));
				PS.ResizeCapsuleCollider(this.ID, currentRadius, currentHeight);
				TS.SetScaling(this.ID, currentXform);
			}
			else if (mainStrawberry)
			{
				//flat model
				if (MS.IsCurrentMesh(this.ID, strawberryMesh) == false)
				{
					GetComponent<MeshRenderer>().Mesh = strawberryMesh;
					GetComponent<MeshRenderer>().Material = strawberryMaterial;
				}
				currentHeight = MathF.Lerp(currentHeight, strawberrysuperHeight, lerpSpeed * Time.deltaTime);
				currentRadius = MathF.Lerp(currentRadius, strawberrysuperRadius, lerpSpeed * Time.deltaTime);
				currOffset = MathF.Lerp(currOffset, -3f, lerpSpeed * Time.deltaTime);
				currentXform.x = MathF.Lerp(currentXform.x, strawberryscaledXform.x, lerpSpeed * Time.deltaTime);
				currentXform.y = MathF.Lerp(currentXform.y, strawberryscaledXform.y, lerpSpeed * Time.deltaTime);
				currentXform.z = MathF.Lerp(currentXform.z, strawberryscaledXform.z, lerpSpeed * Time.deltaTime);

				PS.UpdateColliderOffset(this.ID, new vec3(0, currOffset, 0));
				PS.ResizeCapsuleCollider(this.ID, currentRadius, currentHeight);
				TS.SetScaling(this.ID, currentXform);
			}
		}

		private void HandleParticles(ref vec3 currVelocity)
		{
			//Debug.Log("particle is active: " + PS3D.GetActive(moley_dust.ID));
            vec3 pos = this.GetComponent<Transform>().Position;
			vec3 particleVel;
            if (currVelocity != vec3.Zero)
			{
				particleVel = currVelocity.NormalizedSafe;
				particleVel *= -1;
            }
			else
			{
				particleVel = vec3.Zero;
            }
            //pos.y += 1f;
            pos.x += particleVel.x * 2;
            pos.z += particleVel.z * 2;
            moley_dust.GetComponent<Transform>().Position = pos;

            PS3D.SetVelocity(moley_dust.ID, new vec3(particleVel.x, 0.10f, particleVel.z));
			//turn off the particles if the player is dead/ not moving/ not grounded
			if (isDead || !isGrounded || !isWalking)
			{
                PS3D.SetActive(moley_dust.ID, false);
            }
			else
			{
				PS3D.SetActive(moley_dust.ID, true);
			}
		}


        public void UpdateDisplay()
		{
			MyPowerUpUI.UpdateUI(MyPowerManager.powerUps);
		}

		public void TakeDamage()
		{
			if (Invulnerability || creativeMode) return;
			if (MyPowerManager.powerUps.Count > 0 && !keepInventory)
			{
				MyPowerManager.LoseMain();
				isScaled = false;
			}
			else
			{
				RespawnPlayer = true;
				isDead = true;
				moleyTransform.Scale = new vec3(moleyTransform.Scale.x, 0.01f, moleyTransform.Scale.z);
			}
			Invulnerability = true;

			// Commenting out for now until IsActivated is cfm not needed
			//if (Invulnerability)
			//{
			//	IsActivated = true;
			//	return;
			//}

			//if (MyPowerManager.powerUps.Count > 0)
			//{
			//	MyPowerManager.LoseMain();
			//	isScaled = false;
			//}
			//else
			//{
			//	isDead = true;
			//	RespawnPlayer = true;
			//}
			//Invulnerability = true;
		}

		public void ResetToInitialPos()
		{
			TS.SetPosition(this.ID, InitialPosition);
		}

		// activate this function using a coroutine to time the spawning of the player
		private void Respawn()
		{
			moleyTransform.Position = RespawnPoint;
			//moleyTransform.Scale = oriScale;
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

		// There is a circular/small cylindrical volume that
		// make both players repel each other.
		private void IsInsideCirclePush(vec3 otherPlayerPos)
		{
			const float yDiffThreshold = 2.0f;
			const float circleRadius = 5.0f;

			float yDiff = Math.Abs(transform.Position.y - otherPlayerPos.y);
			if (yDiff >= yDiffThreshold)
			{
				return;
			}

			vec2 playerPlanePos = new vec2(transform.Position.x, transform.Position.z);
			vec2 otherPlanePos = new vec2(otherPlayerPos.x, otherPlayerPos.z);
			vec2 playerToOther = playerPlanePos - otherPlanePos;

			if (playerToOther.Length >= circleRadius)
			{
				return;
			}

			// normalize the vector
			playerToOther = playerToOther.Normalized;

			// flip it
			playerToOther *= -1;

			// give it a lil' scale
			playerToOther *= 20;

			PS.SetLinearVelocity(holey_ref.ID, new vec3(playerToOther.x, 0, playerToOther.y));
		}

		private void CheckControllability()
		{
			// Camera panning at the start
			bool cameraTransiting = false;
			if (Scene.GetSceneName() == "Tutorial" && ECSManager.FindEntityByName("CameraManager") != null && !ECSManager.FindEntityByName("CameraManager").GetComponent<TutorialCameraManager>().preTransitions.preTransitioned)
			{
				cameraTransiting = true;
            }


            // Logic to handle isControllable
            if (Scene.IsTransiting() || cameraTransiting)
			{
				isControllable = false;
			}
			else
			{
				isControllable = true;
            }
		}
		private float VolumeVariation(float volume, float variation)
		{
			float newVolume = volume + Random.Range(-variation, variation);
			if (newVolume < 0)
			{
				newVolume = 0;
			}
			else if (newVolume > 1)
			{
				newVolume = 1;
			}
			return newVolume;
		}
	}
}
