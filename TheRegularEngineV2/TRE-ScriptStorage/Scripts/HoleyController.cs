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
	using PRS = PersistentSystem;

	public class HoleyController : Entity
	{
		public Entity mainCamera;
		public PauseMenu MyPauseMenu;
		public PowerUpUI MyPowerUpUI;
		public PowerUpManager MyPowerManager;

        /// <summary>Check if player is on top of another player</summary>
        public bool isOnPlayer = false;

        /// <summary>Check if player has boosted jump</summary>
        public bool isBoostedJump = false;

		/// <summary>Check if player is on ground</summary>
		public bool isGrounded = true;

		/// <summary>Direction vector</summary>
		private vec3 dirVec;

		/// <summary>Max Velocity vector</summary>
		private const float maxVelocity = 30f;

		/// <summary>Acceleration</summary>
		private const float acceleration = 700f;

		/// <summary>Final Velocity</summary>
		private vec3 finalVelocity = vec3.Zero;

		/// <summary>Original Scale</summary>
		private vec3 oriScale;

		//Check if player is walking
		public bool isWalking = false;

		//check if player used super power
		public bool mainBlueberry = false;  // Scaling
		public bool mainStrawberry = false; // Shape
		public bool isScaled = false;

		// Controllable
		public bool isControllable = true;
		private bool isControllerConnected = false;
		private bool lastControllerConnected = false;
		public bool changeUI = false;
		public float uncontrollableTimer = 0f;
		private Entity CharacterUI;

		public int ControllerPreset = 0;
		public int KeyboardPreset = 1;

		// the controls/keys that THIS player (Holey) will use
		#region Player Controls
		InputKeys playerUpKey = InputKeys.I;
		InputKeys playerDownKey = InputKeys.K;
		InputKeys playerLeftKey = InputKeys.J;
		InputKeys playerRightKey = InputKeys.L;
		InputKeys playerJumpKey = InputKeys.Enter;
		InputKeys playerSwapKey = InputKeys.Backslash;
		InputKeys playerDropKey = InputKeys.RightShift;
		InputKeys playerAbilityKey = InputKeys.Backspace;
		#endregion

		// Controller Preset
		IS.Button jump = IS.Button.A;
		IS.Button swap = IS.Button.Y;
		IS.Button drop = IS.Button.B;
		IS.Button ability = IS.Button.X;

		private const int ControllerNumber = 1;

		#region Collider Variables
		public const float defaultRadius = 2f;
		public const float blueberrysuperRadius = 2.4f;
		public const float strawberrysuperRadius = 2.4f;
		public float currentRadius = 2f;
		public const float defaultHeight = 1f;
		public const float blueberrysuperHeight = 3.6f;
		public const float strawberrysuperHeight = 1.2f;
		public float currentHeight = 1f;
		public float currOffset = 3f;
		#endregion

		#region Player Transform Variables
		private Transform holeyTransform;
		// private vec3 defaultXform = new vec3(75f, 75f, 75f);
		private vec3 blueberryscaledXform = new vec3(0.040f, 0.040f, 0.040f);
		private vec3 strawberryscaledXform = new vec3(0.040f, 0.040f, 0.040f);
		private vec3 currentXform = new vec3(0.75f, 0.75f, 0.75f);
		#endregion

		public int turnDirection = 0;
		private int playerDirection = 0;
		private int lastPlayerDirection = 0;

		private const float lerpSpeed = 5f;

		#region Jump Variables
		private const float maxJumpHeight = 70f;
		//Check if player is jumping at all
		public bool isJumping = false;
		//how long you hold the jump button to reach max jump height
		public const float maxJumpButtomTime = 0.5f;
		public float currentJumpTime;
		public bool jumpCancelled = false;
		//how long after the player walks off the ground can he still jump
		private const float coyoteTime = 0.2f;
		public float coyoteTimeCounter;
		//if player press space within this buffer time, they will still be able to jump even if they havent landed
		private const float jumpBufferTime = 0.25f;
		public float jumpBufferCounter;
		// private float jumpHeight = 25f;
		#endregion

		#region Audio Variables
		private ulong walkingSFX;
		private ulong jumpSFX;
		private ulong changesizeSFX;
		private ulong normalsizeSFX;
		private ulong fallingMaracaSFX;
		// private ulong fallingHatSFX;
		private ulong fallSFX;
		private ulong cheeringSFX;
		private ulong hurtSFX;
		private ulong landingSFX;
		private ulong startCheerSFX;

		private bool walkingSFXPlayed = false;
		private bool fallingSFXPlayed = false;
		private bool landingSFXPlayed = false;
		private bool startcheerPlayed = false;

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
		private const float InvulPeriod = 1.0f;
		private float InvulBlinkCurrent = 0.1f;
		private const float InvulBlinkPeriod = 0.1f;
		#endregion

		//Moley Reference
		private Entity moley_ref;

		//Holey particles
		private Entity holey_dust;
		// Cheats
		public bool keepInventory = false;
		public bool creativeMode = false;

		//Materials and Meshes
		//Default
		private const string defaultWalkingMesh = "mole_walk.fbx";
		private const string defaultJumpingMesh = "mole_jump.fbx";
		private const string defaultIdleMesh = "mole_idle.fbx";
		private const string defaultAnimationMaterial = "BlueCharacter_Animation.material";
		private const string testmat = "Blue_Holey.material";

		//Blueberry
		// private const string blueberryMaterial = "BlueCharacter_Animation.material";
		private const string blueberryMesh = "Holey_Blueberry.fbx";

		//Strawberry
		private const string strawberryMaterial = "Holey_Strawberry.material";
		private const string strawberryMesh = "Holey_Strawberry.fbx";

		//VFX
		private Entity mImpactedVFX;
		private float mImpactedVFXTimer = 1f;
		private bool mIsImpacted = false;

		public void Start()
		{
			#region UI Variables
			MyPauseMenu = ECSManager.FindEntityByName("PauseMenu").GetComponent<PauseMenu>();
			MyPowerUpUI = ECSManager.FindEntityByName("RightCharacter_HUD").GetComponent<PowerUpUI>();
			MyPowerManager = parenting.GetChildFromName("Power Manager").GetComponent<PowerUpManager>();
			MyPowerManager.MyPowerUpUI = MyPowerUpUI;
			MyPowerUpUI.UpdateUI(MyPowerManager.powerUps);
			CharacterUI = ECSManager.FindEntityByName("RightCharacter_HUD");
			#endregion

			#region Player Transform and Physics Variables
			holeyTransform = GetComponent<Transform>();
			holeyTransform.Rotation = vec3.Zero;

			PS.ConstrainRotationX(this.ID, true);
			PS.ConstrainRotationY(this.ID, true);
			PS.ConstrainRotationZ(this.ID, true);
			#endregion

			#region Respawn Variables
			InitialPosition = holeyTransform.Position;
			OutofMapPos = holeyTransform.Position;
			OutofMapPos.y = holeyTransform.Position.y - 300.0f;

			RespawnPoint = holeyTransform.Position;
			RespawnPoint.y += 10.0f;
			#endregion

			#region Sound Variables
			walkingSFX = ECSManager.FindIDFromName("SFX_HoleyFootsteps");
			jumpSFX = ECSManager.FindIDFromName("SFX_HoleyJump");
			changesizeSFX = ECSManager.FindIDFromName("SFX_Tall");
			normalsizeSFX = ECSManager.FindIDFromName("SFX_NormalSize");
			fallingMaracaSFX = ECSManager.FindIDFromName("SFX_FallingMaraca");
			// fallingHatSFX = ECSManager.FindIDFromName("SFX_FallingHat");
			fallSFX = ECSManager.FindIDFromName("SFX_HoleyFall");
			cheeringSFX = ECSManager.FindIDFromName("SFX_Holey_BoostedJump");
			hurtSFX = ECSManager.FindIDFromName("SFX_HoleyHurt1");
			landingSFX = ECSManager.FindIDFromName("SFX_HoleyLand");
			walkingSFXVolume = AS.GetVolume(walkingSFX);
			startCheerSFX = ECSManager.FindIDFromName("SFX_Holey_Cheer1");
			#endregion

			mainCamera = ECSManager.FindEntityByName("Main Camera");
			moley_ref = ECSManager.FindEntityByName("Moley");
			oriScale = holeyTransform.Scale;

			holey_dust = ECSManager.FindEntityByName("Holey_Dust");

			// Presistent Controls
			if (int.TryParse(PRS.GetValue("HoleyKB"), out int result))
			{
				KeyboardPreset = result;
				SetKeyboardPreset(result);
			}
			else
			{
				PRS.SetValue("HoleyKB", "1");
			}

			if (int.TryParse(PRS.GetValue("HoleyController"), out result))
			{
				ControllerPreset = result;
				SetControllerPreset(result);
			}
			else
			{
				PRS.SetValue("HoleyController", "0");
			}

			mImpactedVFX = ECSManager.FindEntityByName("ImpactedVFXHoley");
		}

		public void Update()
		{
			#region UI
			// check if controller connected
			isControllerConnected = IS.GetControllerConnected(ControllerNumber);
			if (lastControllerConnected != isControllerConnected)
			{
				lastControllerConnected = isControllerConnected;
				changeUI = true;
			}

			if (changeUI && isControllerConnected && CharacterUI != null)
			{
				changeUI = false;
                switch (ControllerPreset)
                {
                    case 0:
                        CharacterUI.GetComponent<SpriteRenderer>().Texture = "CharacterUI_Left_Controller.png";
                        break;
                    case 1:
                        CharacterUI.GetComponent<SpriteRenderer>().Texture = "CharacterUI_Left_Controller2.png";
                        break;
                    case 2:
                        CharacterUI.GetComponent<SpriteRenderer>().Texture = "CharacterUI_Left_Controller2.png";
                        break;
					case 3:
						CharacterUI.GetComponent<SpriteRenderer>().Texture = "CharacterUI_Left_Controller.png";
						break;
                }
			}

			if (changeUI && !isControllerConnected && CharacterUI != null)
			{
				changeUI = false;
                switch (KeyboardPreset)
                {
					case 0:
						CharacterUI.GetComponent<SpriteRenderer>().Texture = "CharacterUI_Left.png";
                        break;
                    case 1:
                        CharacterUI.GetComponent<SpriteRenderer>().Texture = "CharacterUI_Left_KB2.png";
						break;
					case 2:
                        CharacterUI.GetComponent<SpriteRenderer>().Texture = "CharacterUI_Left_KB3.png";
                        break;
                }
				
			}
			#endregion

			CheckControllability();

			HandleInvulnerability();

			vec3 pos = holeyTransform.Position;

			//Movement Related stuff
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

			playerDirection = lastPlayerDirection + turnDirection;
			playerDirection %= 360;

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
			IsInsideCirclePush(moley_ref.GetComponent<Transform>().Position);

			holeyTransform.Rotation = new vec3(0, playerDirection, 0);

			//isGrounded = false;

			if (RespawnPlayer)
			{
				if (RespawnTimer > 0 /* && isDead*/)
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

			if (mIsImpacted)
			{
				// Always look at the main camera
				if (mainCamera != null)
				{
					// Rotate Character to look at target
					vec2 rotAxis = MathF.GetLookAtAxis(mImpactedVFX.transform.Position, mainCamera.transform.Position);
					mImpactedVFX.transform.Rotation = new vec3(rotAxis.x, rotAxis.y, 0);
				}

				mImpactedVFXTimer -= Time.GetDeltaTime();
				if (mImpactedVFXTimer < 0)
				{
					mIsImpacted = false;
					mImpactedVFXTimer = 1f;
					SpriteSystem.SetSprite3DVisibility(mImpactedVFX.ID, false);
				}
			}
		}

		private void Jump(vec3 JumpHeight)
		{
			PS.AddForce(this.ID, JumpHeight, ForceMode.VelocityChange);
		}

		private void SetImpactedVFX()
		{
			vec3 HoleyPos = new vec3(this.GetComponent<Transform>().Position);
			HoleyPos += new vec3(0, 4, 0);
			mImpactedVFX.GetComponent<Transform>().Position = HoleyPos;
			SpriteSystem.SetSprite3DVisibility(mImpactedVFX.ID, true);
			mIsImpacted = true;
		}

		public void OnCollisionStay(System.UInt64 otherID)
		{
			//isGrounded = false;

			Entity other = new Entity(otherID);
			// Make it loose one of it's powerups
			if (other.CompareTag("FallingObstacle") || other.CompareTag("RollingObstacle"))
			{
				// Ignore damage for fallingObstacle under the following conditions
				PS.GetLinearVelocity(otherID, out vec3 speed);
				// Falling object is no longer falling
				if (other.CompareTag("FallingObstacle") && other.GetComponent<FallingObj>().isGrounded) return;

				TakeDamage();
				SetImpactedVFX();

				if (other.name == "FallingRock" && ECSManager.IsValidEntity(hurtSFX))
				{
					AS.Play(hurtSFX);
				}
				else if (other.name == "RollingObstacle" && ECSManager.IsValidEntity(hurtSFX))
				{
					AS.Play(hurtSFX);
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

			if (other.CompareTag("Ground") || other.CompareTag("Platform")
				|| other.CompareTag("Blue") || other.CompareTag("RedCollider")
				|| other.CompareTag("LeftCactus") || other.CompareTag("RightCactus"))
			{
				isGrounded = true;
			}
		}

		public void OnCollisionExit(System.UInt64 otherID)
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
			if (other.CompareTag("JumpPad"))
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
				if (!moley_ref.GetComponent<MoleyController>().GetIsDead() && isDead)
				{
					RespawnPlayer = true;
					if (MyPowerManager.powerUps.Count > 0 && !keepInventory)
					{
						MyPowerManager.LoseMain();
						isScaled = false;
					}
				}
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
				if (!MyPauseMenu.isPaused && isControllable && !isControllerConnected)
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
						}

						//check if space is held down and jump time is not over
						if (currentJumpTime > maxJumpButtomTime)
						{
							isJumping = false;
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

						vec3 maxHeight = new vec3(0, 70, 0);
						// Boosted Jump
						if (isBoostedJump)
						{
							maxHeight = new vec3(0, 140, 0);
						}

						Jump(maxHeight);

						if (ECSManager.IsValidEntity(jumpSFX) && ECSManager.IsValidEntity(cheeringSFX))
						{
							if (!startcheerPlayed && ECSManager.IsValidEntity(startCheerSFX))
							{
								AS.Play(startCheerSFX);
								startcheerPlayed = true;
							}

							if (startcheerPlayed)
							{
								int cheer = Random.Range(1, 5);//25% chance to play cheering sound
								if (cheer == 1)
								{
									AS.Play(startCheerSFX);
								}
							}

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
				else if (!MyPauseMenu.isPaused && isControllable && isControllerConnected)
				{
					if (ControllerPreset == 0 || ControllerPreset == 2)
					{
						float x = IS.GetControllerStickX(ControllerNumber, false); // false for left thumbstick
						float y = IS.GetControllerStickY(ControllerNumber, false); // false for left thumbstick

						// calculate the direction vector
						isWalking = x != 0 || y != 0;

						// calculate the angle of the direction vector
						if (isWalking)
						{
							lastPlayerDirection = (int)(Math.Atan2(y, x) * 180 / Math.PI - 90 + 360) % 360;

							// handle the dirVec
							if (y > 0)
								dirVec += CS.GetMainCameraForwardVec();
							if (y < 0)
								dirVec -= CS.GetMainCameraForwardVec();
							if (x > 0)
								dirVec -= CS.GetMainCameraRightVec();
							if (x < 0)
								dirVec += CS.GetMainCameraRightVec();
						}
					}
					else
					{
						if (IS.GetControllerButtonPress(ControllerNumber, IS.Button.DPadUp))
						{
							dirVec += CS.GetMainCameraForwardVec();
							lastPlayerDirection = 0;
							isWalking = true;
						}

						if (IS.GetControllerButtonPress(ControllerNumber, IS.Button.DPadDown))
						{
							dirVec -= CS.GetMainCameraForwardVec();
							lastPlayerDirection = 180;
							isWalking = true;
						}

						if (IS.GetControllerButtonPress(ControllerNumber, IS.Button.DPadLeft))
						{
							dirVec += CS.GetMainCameraRightVec();
							lastPlayerDirection = 90;
							isWalking = true;
						}

						if (IS.GetControllerButtonPress(ControllerNumber, IS.Button.DPadRight))
						{
							dirVec -= CS.GetMainCameraRightVec();
							lastPlayerDirection = 270;
							isWalking = true;
						}

						if (IS.GetControllerButtonPress(ControllerNumber, IS.Button.DPadUp))
						{
							if (IS.GetControllerButtonPress(ControllerNumber, IS.Button.DPadRight))
								lastPlayerDirection = 315;

							if (IS.GetControllerButtonPress(ControllerNumber, IS.Button.DPadLeft))
								lastPlayerDirection = 45;
						}

						if (IS.GetControllerButtonPress(ControllerNumber, IS.Button.DPadDown))
						{
							if (IS.GetControllerButtonPress(ControllerNumber, IS.Button.DPadRight))
								lastPlayerDirection = 225;

							if (IS.GetControllerButtonPress(ControllerNumber, IS.Button.DPadLeft))
								lastPlayerDirection = 135;
						}

						if (!IS.GetControllerButtonPress(ControllerNumber, IS.Button.DPadUp) &&
							!IS.GetControllerButtonPress(ControllerNumber, IS.Button.DPadDown) &&
							!IS.GetControllerButtonPress(ControllerNumber, IS.Button.DPadLeft) &&
							!IS.GetControllerButtonPress(ControllerNumber, IS.Button.DPadRight))
						{
							isWalking = false;
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
					if (IS.GetControllerButtonTriggered(ControllerNumber, jump))
					{
						// jumpHeight += Time.deltaTime;
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
						if (IS.GetControllerButtonReleased(ControllerNumber, jump))
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
						if (IS.GetControllerButtonReleased(ControllerNumber, jump))
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
							if (!startcheerPlayed && ECSManager.IsValidEntity(startCheerSFX))
							{
								AS.Play(startCheerSFX);
								startcheerPlayed = true;
							}

							if (startcheerPlayed)
							{
								int cheer = Random.Range(1, 5);//25% chance to play cheering sound
								if (cheer == 1)
								{
									AS.Play(startCheerSFX);
								}
							}

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
			else if (!isGrounded)
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

			if (IS.GetControllerButtonTriggered(ControllerNumber, swap))
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

			if (IS.GetControllerButtonTriggered(ControllerNumber, drop))
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
			if (isControllable && IS.GetKeyPress(playerAbilityKey))
			{
				if (mainBlueberry || mainStrawberry)
				{
					isScaled = !isScaled;

					// Shift it up to prevent falling
					if (isScaled)
					{
						//vec3 newPos = transform.Position;
						//newPos.y += 1.5f;
						//transform.Position = newPos;
						PS.SetLinearVelocity(ID, vec3.Zero);

						// Shift other mole up too if it is colliding
						Entity headCollider = ECSManager.FindEntityByName("Holey's Head Collider");
						headCollider.GetComponent<PlayerHeadCollider>().SetToPlayer();
						Entity Moley = ECSManager.FindEntityByName("Moley");
						if (PS.IsCollisionStay(headCollider.ID, Moley.ID))
						{
							vec3 moleyPos = vec3.Zero;
							moleyPos.y = currOffset + GetComponent<CapsuleCollider>().HalfHeight * 2f + Moley.GetComponent<CapsuleCollider>().HalfHeight + 3.5f;
							Moley.transform.Position += moleyPos;
							PS.SetLinearVelocity(Moley.ID, vec3.Zero);
						}
						PS.SetLinearVelocity(ID, vec3.Zero);
					}

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

			if (isControllable && IS.GetControllerButtonTriggered(ControllerNumber, ability))
			{
				if (mainBlueberry || mainStrawberry)
				{
					isScaled = !isScaled;

					// Shift it up to prevent falling
					if (isScaled)
					{
						//vec3 newPos = transform.Position;
						//newPos.y += 1.5f;
						//transform.Position = newPos;
						PS.SetLinearVelocity(ID, vec3.Zero);

						// Shift other mole up too if it is colliding
						Entity headCollider = ECSManager.FindEntityByName("Holey's Head Collider");
						headCollider.GetComponent<PlayerHeadCollider>().SetToPlayer();
						Entity Moley = ECSManager.FindEntityByName("Moley");
						if (PS.IsCollisionStay(headCollider.ID, Moley.ID))
						{
							vec3 moleyPos = vec3.Zero;
							moleyPos.y = currOffset + GetComponent<CapsuleCollider>().HalfHeight * 2f + Moley.GetComponent<CapsuleCollider>().HalfHeight + 3.5f;
							Moley.transform.Position += moleyPos;
							PS.SetLinearVelocity(Moley.ID, vec3.Zero);
						}
						PS.SetLinearVelocity(ID, vec3.Zero);
					}

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
				dirVec.y = 0;
				if (dirVec != vec3.Zero && isGrounded)
				{
					dirVec = dirVec.Normalized;

					//Walking state
					GetComponent<MeshRenderer>().Mesh = defaultWalkingMesh;
					GetComponent<MeshRenderer>().AnimMaterial = defaultAnimationMaterial;
					if (HasComponent<Animation>())
						GetComponent<Animation>().AnimationSpeed = 0.5f;
				}
				else if (!isGrounded)
				{
					dirVec = dirVec.NormalizedSafe;
					GetComponent<MeshRenderer>().Mesh = defaultJumpingMesh;
					GetComponent<MeshRenderer>().AnimMaterial = defaultAnimationMaterial;
					if (HasComponent<Animation>())
						GetComponent<Animation>().AnimationSpeed = 0.5f;
				}
				else
				{
					GetComponent<MeshRenderer>().Mesh = defaultIdleMesh;
					GetComponent<MeshRenderer>().AnimMaterial = defaultAnimationMaterial;
					if (HasComponent<Animation>())
						GetComponent<Animation>().AnimationSpeed = 0.25f;
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
				//Tall model
				if (MS.IsCurrentMesh(this.ID, blueberryMesh) == false)
				{
					GetComponent<MeshRenderer>().Mesh = blueberryMesh;
					GetComponent<MeshRenderer>().Material = testmat;
					if (HasComponent<Animation>())
						RemoveComponent<Animation>();
				}
				currentHeight = MathF.Lerp(currentHeight, blueberrysuperHeight, lerpSpeed * Time.deltaTime);
				currentRadius = MathF.Lerp(currentRadius, blueberrysuperRadius, lerpSpeed * Time.deltaTime);
				currOffset = MathF.Lerp(currOffset, 5.8f, lerpSpeed * Time.deltaTime);
				currentXform = blueberryscaledXform;

				PS.ResizeCapsuleCollider(this.ID, currentRadius, currentHeight);
				PS.UpdateColliderOffset(this.ID, new vec3(0, currOffset, 0));
				TS.SetScaling(this.ID, currentXform);
			}
			else if (mainStrawberry)
			{
				//Cactus Model
				if (MS.IsCurrentMesh(this.ID, strawberryMesh) == false)
				{
					GetComponent<MeshRenderer>().Mesh = strawberryMesh;
					GetComponent<MeshRenderer>().Material = strawberryMaterial;
					if (HasComponent<Animation>())
						RemoveComponent<Animation>();
				}

				currentHeight = MathF.Lerp(currentHeight, strawberrysuperHeight, lerpSpeed * Time.deltaTime);
				currentRadius = MathF.Lerp(currentRadius, strawberrysuperRadius, lerpSpeed * Time.deltaTime);
				PS.ResizeCapsuleCollider(this.ID, currentRadius, currentHeight);

				currentXform.x = MathF.Lerp(currentXform.x, strawberryscaledXform.x, lerpSpeed * Time.deltaTime);
				currentXform.y = MathF.Lerp(currentXform.y, strawberryscaledXform.y, lerpSpeed * Time.deltaTime);
				currentXform.z = MathF.Lerp(currentXform.z, strawberryscaledXform.z, lerpSpeed * Time.deltaTime);
				TS.SetScaling(this.ID, currentXform);
			}
		}

		private void HandleParticles(ref vec3 currVelocity)
		{
			//Debug.Log("particle is active: " + PS3D.GetActive(holey_dust.ID));
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
			holey_dust.GetComponent<Transform>().Position = pos;

			PS3D.SetVelocity(holey_dust.ID, new vec3(particleVel.x, 0.10f, particleVel.z));
			//turn off the particles if the player is dead/ not moving/ not grounded
			if (isDead || !isGrounded || !isWalking)
			{
				PS3D.SetActive(holey_dust.ID, false);
			}
			else
			{
				PS3D.SetActive(holey_dust.ID, true);
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
				holeyTransform.Scale = new vec3(holeyTransform.Scale.x, 0.01f, holeyTransform.Scale.z);
			}
			Invulnerability = true;

			ECSManager.FindEntityByName("Moley").GetComponent<MoleyController>().IsActivated = true;
		}

		public void ResetToInitialPos()
		{
			holeyTransform.Position = InitialPosition;
		}

		// activate this function using a coroutine to time the spawning of the player
		private void Respawn()
		{
			holeyTransform.Position = RespawnPoint;
			holeyTransform.Scale = oriScale;
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

			PS.SetLinearVelocity(moley_ref.ID, new vec3(playerToOther.x, 0, playerToOther.y));
		}

		private void CheckControllability()
		{
			// Confirmation pop up
			bool confirmationPopUp = false;
			if (Scene.GetSceneName() == "MainMenu" && ECSManager.FindEntityByName("QuitConfirmationPopup") != null && ECSManager.FindEntityByName("QuitConfirmationPopup").GetComponent<SpriteRenderer>().isVisible)
			{
				confirmationPopUp = true;
			}

			// Camera panning at the start
			bool cameraTransiting = false;
			if (ECSManager.FindEntityByName("CameraManager") != null &&
				(
				(Scene.GetSceneName() == "Tutorial" && !ECSManager.FindEntityByName("CameraManager").GetComponent<TutorialCameraManager>().preTransitions.preTransitioned) ||
				(Scene.GetSceneName() == "Level_1" && !ECSManager.FindEntityByName("CameraManager").GetComponent<Level_1CameraManager>().preTransitions.preTransitioned) ||
				(Scene.GetSceneName() == "Level_2" && !ECSManager.FindEntityByName("CameraManager").GetComponent<Level_2CameraManager>().preTransitions.preTransitioned))
				)
			{
				cameraTransiting = true;
			}

			// Reduce uncontrollable timer if > 0
			if (uncontrollableTimer > 0f) uncontrollableTimer -= Time.deltaTime;
			if (uncontrollableTimer < 0f) uncontrollableTimer = 0f;

			// Logic to handle isControllable
			if (Scene.IsTransiting() || cameraTransiting || confirmationPopUp || uncontrollableTimer > 0f)
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

		public void SetControllerPreset(int preset)
		{
			switch (preset)
			{
				case 0:
					jump = IS.Button.A;
					swap = IS.Button.B;
					ability = IS.Button.X;
					drop = IS.Button.Y;
					break;
				case 1:
					jump = IS.Button.B;
					swap = IS.Button.A;
					ability = IS.Button.Y;
					drop = IS.Button.X;
					break;
				case 2:
                    jump = IS.Button.B;
                    swap = IS.Button.A;
                    ability = IS.Button.Y;
                    drop = IS.Button.X;
                    break;
				case 3:
                    jump = IS.Button.A;
                    swap = IS.Button.B;
                    ability = IS.Button.X;
                    drop = IS.Button.Y;
                    break;
			}

		}

		public void SetKeyboardPreset(int preset)
		{
			switch (preset)
			{
				case 0:
					playerUpKey = InputKeys.W;
					playerDownKey = InputKeys.S;
					playerLeftKey = InputKeys.A;
					playerRightKey = InputKeys.D;
					playerJumpKey = InputKeys.Space;
					playerSwapKey = InputKeys.Q;
					playerAbilityKey = InputKeys.E;
					playerDropKey = InputKeys.LeftShift;
					break;
				case 1:
					playerUpKey = InputKeys.I;
					playerDownKey = InputKeys.K;
					playerLeftKey = InputKeys.J;
					playerRightKey = InputKeys.L;
					playerJumpKey = InputKeys.Enter;
					playerSwapKey = InputKeys.Backslash;
					playerAbilityKey = InputKeys.Backspace;
					playerDropKey = InputKeys.RightShift;
					break;
				case 2:
					playerUpKey = InputKeys.Up;
					playerDownKey = InputKeys.Down;
					playerLeftKey = InputKeys.Left;
					playerRightKey = InputKeys.Right;
					playerJumpKey = InputKeys.Slash;
					playerSwapKey = InputKeys.Comma;
					playerAbilityKey = InputKeys.Period;
					playerDropKey = InputKeys.M;
					break;
			}

		}
	}
}
