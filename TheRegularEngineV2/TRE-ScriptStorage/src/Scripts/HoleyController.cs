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

	public class HoleyController : Entity
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
		private bool walkingSFXPlayed = false;
		private bool fallingSFXPlayed = false;

		//check if player used super power
		public bool mainBlueberry = false;  // Scaling
		public bool mainStrawberry = false; // Shape
		public bool isScaled = false;

		// Controllable
		public bool isControllable = true;

		// the controls/keys that THIS player (Holey) will use
		#region Player Controls
		const InputKeys playerUpKey = InputKeys.I;
		const InputKeys playerDownKey = InputKeys.K;
		const InputKeys playerLeftKey = InputKeys.J;
		const InputKeys playerRightKey = InputKeys.L;
		const InputKeys playerJumpKey = InputKeys.Enter;
		const InputKeys playerSwapKey = InputKeys.Backslash;
		const InputKeys playerDropKey = InputKeys.RightShift;
		const InputKeys playerAbilityKey = InputKeys.Backspace;
		#endregion

		#region Collider Variables
		public float defaultRadius = 2f;
		public float blueberrysuperRadius = 2.4f;
		public float strawberrysuperRadius = 2.4f;
		public float currentRadius = 2f;
		public float defaultHeight = 1f;
		public float blueberrysuperHeight = 3.6f;
		public float strawberrysuperHeight = 1.2f;
		public float currentHeight = 1f;
		public float currOffset = 3f;
		#endregion

		#region Player Transform Variables
		private Transform holeyTransform;
		public vec3 defaultXform = new vec3(75f, 75f, 75f);
		public vec3 blueberryscaledXform = new vec3(0.040f, 0.040f, 0.040f);
		public vec3 strawberryscaledXform = new vec3(0.040f, 0.040f, 0.040f);
		public vec3 currentXform = new vec3(0.75f, 0.75f, 0.75f);
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
		private ulong jumpSFX;
		private ulong changesizeSFX;
		private ulong normalsizeSFX;
		private ulong fallingMaracaSFX;
		private ulong fallingHatSFX;
		private ulong fallSFX;
		private ulong cheeringSFX;
		private ulong hurtSFX;
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

		//Moley Reference
		private Entity moley_ref;

		// Cheats
		public bool keepInventory = false;
		public bool creativeMode = false;

		public void Start()
		{
			#region UI Variables
			MyPauseMenu = ECSManager.FindEntityByName("PauseMenu").GetComponent<PauseMenu>();
			MyPowerUpUI = ECSManager.FindEntityByName("RightCharacter_HUD").GetComponent<PowerUpUI>();
			MyPowerManager = parenting.GetChildFromName("Power Manager").GetComponent<PowerUpManager>();
			MyPowerManager.MyPowerUpUI = MyPowerUpUI;
			MyPowerUpUI.UpdateUI(MyPowerManager.powerUps);
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
			OutofMapPos.y = holeyTransform.Position.y - 200.0f;

			RespawnPoint = holeyTransform.Position;
			RespawnPoint.y += 10.0f;
			#endregion

			#region Sound Variables
			walkingSFX = ECSManager.FindIDFromName("SFX_HoleyFootsteps");
			jumpSFX = ECSManager.FindIDFromName("SFX_HoleyJump");
			changesizeSFX = ECSManager.FindIDFromName("SFX_Tall");
			normalsizeSFX = ECSManager.FindIDFromName("SFX_NormalSize");
			fallingMaracaSFX = ECSManager.FindIDFromName("SFX_FallingMaraca");
			fallingHatSFX = ECSManager.FindIDFromName("SFX_FallingHat");
			fallSFX = ECSManager.FindIDFromName("SFX_HoleyFall");
			cheeringSFX = ECSManager.FindIDFromName("SFX_Holey_BoostedJump");
			hurtSFX = ECSManager.FindIDFromName("SFX_HoleyHurt1");
			#endregion

			moley_ref = ECSManager.FindEntityByName("Moley");

			oriScale = holeyTransform.Scale;
		}

		public void Update()
		{
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

			//Do NOT REMOVE THIS for some reason it stops the mole when its tall from flying idk dont ask me
			dirVec.y = 0;
			dirVec = dirVec.NormalizedSafe;

			playerDirection = lastPlayerDirection + turnDirection;
			playerDirection = playerDirection % 360;


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
		}

		private void Jump(vec3 JumpHeight)
		{
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
				else if (other.ID == ECSManager.FindIDFromName("FallingHat"))
				{
					if (ECSManager.IsValidEntity(fallingHatSFX))
						AS.Play(fallingHatSFX);
				}
				else
				{

				}
				//else if (other.ID == ECSManager.FindIDFromName("RollingObjectRender"))
				//{
				//	if (ECSManager.IsValidEntity(hurtSFX))
				//		AudioSystem.Play(hurtSFX);
				//}

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

				//Debug.Log("Out of map");
			}
			else
			{
				DroppingOutOfMap = false;
				//Debug.Log("Not out of map");
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
				if (!MyPauseMenu.isPaused && isControllable)
				{
					if (IS.GetKeyHold(playerUpKey))
					{
						dirVec += CS.GetMainCameraForwardVec();
						lastPlayerDirection = 0;
					}

					if (IS.GetKeyHold(playerDownKey))
					{
						dirVec -= CS.GetMainCameraForwardVec();
						lastPlayerDirection = 180;
					}

					if (IS.GetKeyHold(playerLeftKey))
					{
						dirVec += CS.GetMainCameraRightVec();
						lastPlayerDirection = 90;
					}

					if (IS.GetKeyHold(playerRightKey))
					{
						dirVec -= CS.GetMainCameraRightVec();
						lastPlayerDirection = 270;
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

					if (jumpCancelled && isJumping && currVelocity.y > 0)
					{
						currVelocity.y = 0;
					}

					if (isGrounded)
					{
						coyoteTimeCounter = coyoteTime;
					}
					else
					{
						coyoteTimeCounter -= Time.deltaTime;
					}

					if (IS.GetKeyPress(playerJumpKey))
					{
						jumpBufferCounter = jumpBufferTime;
					}
					else
					{
						jumpBufferCounter -= Time.deltaTime;
					}

					if (isJumping)
					{
						if (IS.GetKeyRelease(playerJumpKey))
						{
							//Debug.Log("cancelled jump");
							jumpCancelled = true;
							coyoteTimeCounter = 0f;
						}
						if (currentJumpTime > maxJumpButtomTime)
						{
							//Debug.Log("maxed out jump");
							isJumping = false;
						}
						currentJumpTime += Time.deltaTime;
					}
					else
					{
						if (IS.GetKeyRelease(playerJumpKey))
						{
							isJumping = false;
						}
					}

					if (coyoteTimeCounter > 0f && jumpBufferCounter > 0f && isControllable && !IsInsideCircleNoJump(moley_ref.GetComponent<Transform>().Position))
					{
						isWalking = false;

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
			bool isMovementKeyHeld = IS.GetKeyHold(playerUpKey) || IS.GetKeyHold(playerDownKey) ||
				IS.GetKeyHold(playerLeftKey) || IS.GetKeyHold(playerRightKey);

			// if the player holds any movement key, they are walking
			isWalking = isMovementKeyHeld;

			if (ECSManager.IsValidEntity(walkingSFX))
			{
				if (isWalking && walkingSFXPlayed == false)
				{
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
		}

		private void HandleDrop()
		{
			// Check if can trigger ability
			if (IS.GetKeyPress(playerDropKey))
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

			if (IS.GetKeyPress(playerAbilityKey))
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
							vec3 moleyPos = Moley.transform.Position;
							moleyPos.y += currOffset + GetComponent<CapsuleCollider>().HalfHeight * 2f + Moley.GetComponent<CapsuleCollider>().HalfHeight + 3.5f;
							Moley.transform.Position = moleyPos;
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
					GetComponent<MeshRenderer>().Mesh = "mole_walk.fbx";
					GetComponent<MeshRenderer>().AnimMaterial = "BlueCharacter_Animation.material";
					if (HasComponent<Animation>())
						GetComponent<Animation>().AnimationSpeed = 0.5f;
				}
				else if (!isGrounded)
				{
					dirVec = dirVec.NormalizedSafe;
					GetComponent<MeshRenderer>().Mesh = "mole_jump.fbx";
					GetComponent<MeshRenderer>().AnimMaterial = "BlueCharacter_Animation.material";
					if (HasComponent<Animation>())
						GetComponent<Animation>().AnimationSpeed = 0.5f;
				}
				else
				{
					GetComponent<MeshRenderer>().Mesh = "mole_idle.fbx";
					GetComponent<MeshRenderer>().AnimMaterial = "BlueCharacter_Animation.material";
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
				if (MS.IsCurrentMesh(this.ID, "Holey_Blueberry.fbx") == false)
				{
					GetComponent<MeshRenderer>().Mesh = "Holey_Blueberry.fbx";
					GetComponent<MeshRenderer>().Material = "BlueCharacter_Animation.material";
					if (HasComponent<Animation>())
						RemoveComponent<Animation>();
				}
				currentHeight = MathF.Lerp(currentHeight, blueberrysuperHeight, 0.5f * lerpSpeed * Time.deltaTime);
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
				if (MS.IsCurrentMesh(this.ID, "Holey_Strawberry.fbx") == false)
				{
					GetComponent<MeshRenderer>().Mesh = "Holey_Strawberry.fbx";
					GetComponent<MeshRenderer>().Material = "Holey_Strawberry.material";
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
		// when both players are in, neither can jump.
		private bool IsInsideCircleNoJump(vec3 otherPlayerPos)
		{
			// Debug.Log("Moley Pos: " + otherPlayerPos.ToString());
			const float yDiffThreshold = 2.0f;
			const float circleRadius = 5.0f;

			float yDiff = Math.Abs(transform.Position.y - otherPlayerPos.y);
			if (yDiff >= yDiffThreshold)
			{
				// Debug.Log("different y heights, ok");
				return false;
			}

			vec2 playerPlanePos = new vec2(transform.Position.x, transform.Position.z);
			vec2 otherPlanePos = new vec2(otherPlayerPos.x, otherPlayerPos.z);

			float planeDist = (playerPlanePos - otherPlanePos).Length;
			if (planeDist >= circleRadius)
			{
				// Debug.Log("same y, far enough away, ok");
				return false;
			}
			return true;
		}
	}
}
