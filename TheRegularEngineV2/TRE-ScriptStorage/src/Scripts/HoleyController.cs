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
	using CS = CameraSystem;
	using PS = PhysicsSystem;
	using TS = TransformSystem;
	using MS = MeshRendererSystem;

	class HoleyController : Entity
	{
		public PowerUpUI MyPowerUpUI;
		public PowerUpManager MyPowerManager;
		public PauseMenu MyPauseMenu;

		//Check if player is boosted jump
		public bool isBoostedJump = false;
		//Check if player is on the ground
		public bool isGrounded = true;
		//direction vector
		private vec3 dirVec;
		//Max velocity
		private float maxVelocity = 30f;
		//Max Air Velocity vector
		//private float maxAirVelocity = 25f;
		//Acceleration
		private float acceleration = 700f;
		//final velocity
		private vec3 finalVelocity = vec3.Zero;
		//maxJumpHeight
		private float maxJumpHeight = 70f;
        //Check if player is walking
        public bool isWalking = false;
		private bool walkingSFXPlayed = false;

		private float lerpSpeed = 5f;

		//Jump Variables
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

		//Capsule Collider
		public bool mainBlueberry = false;  // Scaling
		public bool mainStrawberry = false; // Shape
		public bool isScaled = false;
		public float defaultRadius = 2f;
		public float defaultHeight = 1f;

		public float blueberrysuperRadius = 2.4f;
		public float blueberrysuperHeight = 3.6f;

		public float strawberrysuperRadius = 2.4f;
		public float strawberrysuperHeight = 1.2f;

		public float currentRadius = 2f;
		public float currentHeight = 1f;
		public float currOffset = 3f;

		//Transform Scale
		public vec3 defaultXform = new vec3(75f, 75f, 75f);
		public vec3 blueberryscaledXform = new vec3(0.040f, 0.0354f, 0.040f);
		public vec3 strawberryscaledXform = new vec3(0.025f, 0.025f, 0.025f);
		public vec3 currentXform = new vec3(0.75f, 0.75f, 0.75f);

		public int turnDirection = 0;
		private int playerDirection = 0;
		private int lastPlayerDirection = 0;

		//Respawn variables
		private vec3 RespawnPoint = new vec3(0, 0, 0);
		private bool RespawnPlayer = false;
		private float RespawnTimer = 1.5f;

		// Is Dead
		public bool isDead = false;

		// Controllable
		public bool isControllable = true;

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
		private ulong fallingMaracaSFX;
		private ulong fallingHatSFX;
		private ulong fallSFX;

		//Transfrom Component
		private Transform holeyTransform;

		//Moley Reference 
		private Entity moley_ref;

		public void Start()
		{
			MyPauseMenu = ECSManager.FindEntityByName("PauseMenu").GetComponent<PauseMenu>();
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
			OutofMapPos.y = holeyTransform.Position.y - 200.0f;

			walkingSFX = ECSManager.FindIDFromName("SFX_HoleyFootsteps");
			jumpSFX = ECSManager.FindIDFromName("SFX_HoleyJump");
			changesizeSFX = ECSManager.FindIDFromName("SFX_Tall");
			normalsizeSFX = ECSManager.FindIDFromName("SFX_NormalSize");
			fallingMaracaSFX = ECSManager.FindIDFromName("SFX_FallingMaraca");
			fallingHatSFX = ECSManager.FindIDFromName("SFX_FallingHat");
			fallSFX = ECSManager.FindIDFromName("SFX_HoleyFall");

			RespawnPoint = holeyTransform.Position;
			RespawnPoint.y += 10.0f;

			moley_ref = ECSManager.FindEntityByName("Moley");
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

				if (ECSManager.IsValidEntity(fallSFX))
				{
					AudioSystem.Play(fallSFX);
				}
				//Debug.Log("Out of map");
			}
			else
			{
				DroppingOutOfMap = false;
				//Debug.Log("Not out of map");
			}

			if (pos.y < (InitialPosition.y - 50.0f))
			{
				if (!moley_ref.GetComponent<MoleyController>().GetIsDead() && isDead)
				{
					RespawnPlayer = true;
					if (MyPowerManager.powerUps.Count > 0)
					{
						MyPowerManager.LoseMain();
						isScaled = false;
					}

				}
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
				if (!MyPauseMenu.isPaused && isControllable)
				{

					if (InputSystem.GetKeyHold(InputKeys.I))
					{
						dirVec += CS.GetMainCameraForwardVec();
						lastPlayerDirection = 0;
					}

					if (InputSystem.GetKeyHold(InputKeys.K))
					{
						dirVec -= CS.GetMainCameraForwardVec();
						lastPlayerDirection = 180;
					}

					if (InputSystem.GetKeyHold(InputKeys.J))
					{
						dirVec += CS.GetMainCameraRightVec();
						lastPlayerDirection = 90;
					}

					if (InputSystem.GetKeyHold(InputKeys.L))
					{
						dirVec -= CS.GetMainCameraRightVec();
						lastPlayerDirection = 270;
					}

					if (InputSystem.GetKeyHold(InputKeys.I))
					{
						if (InputSystem.GetKeyHold(InputKeys.L))
						{
							lastPlayerDirection = 315;
						}

						if (InputSystem.GetKeyHold(InputKeys.J))
						{
							lastPlayerDirection = 45;
						}
					}

					if (InputSystem.GetKeyHold(InputKeys.K))
					{
						if (InputSystem.GetKeyHold(InputKeys.L))
						{
							lastPlayerDirection = 225;
						}

						if (InputSystem.GetKeyHold(InputKeys.J))
						{
							lastPlayerDirection = 135;
						}
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

					if (InputSystem.GetKeyPress(InputKeys.Enter))
					{
						jumpBufferCounter = jumpBufferTime;
					}
					else
					{
						jumpBufferCounter -= Time.deltaTime;
					}

					if (isJumping)
					{
						if (InputSystem.GetKeyRelease(InputKeys.Enter))
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
						if (InputSystem.GetKeyRelease(InputKeys.Enter))
						{
							isJumping = false;
						}
					}

					if (coyoteTimeCounter > 0f && jumpBufferCounter > 0f && isControllable)
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

			}

			#endregion

			#region Audio

			if (InputSystem.GetKeyHold(InputKeys.I) || InputSystem.GetKeyHold(InputKeys.K) ||
				InputSystem.GetKeyHold(InputKeys.J) || InputSystem.GetKeyHold(InputKeys.L))
			{
				isWalking = true;
			}

			if (!(InputSystem.GetKeyHold(InputKeys.I) || InputSystem.GetKeyHold(InputKeys.J) ||
				  InputSystem.GetKeyHold(InputKeys.K) || InputSystem.GetKeyHold(InputKeys.L)))
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
			if (InputSystem.GetKeyPress(InputKeys.Backslash))
			{
				MyPowerManager.SwapPowerUps();
				MyPowerUpUI.UpdateUI(MyPowerManager.powerUps);
				isScaled = false;
			}

			#endregion

			#region Drop

			// Check if can trigger ability
			if (InputSystem.GetKeyPress(InputKeys.RightShift))
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

			if (InputSystem.GetKeyPress(InputKeys.Backspace))
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
						PhysicsSystem.SetLinearVelocity(ID, vec3.Zero);

						// Shift other mole up too if it is colliding
						Entity headCollider = ECSManager.FindEntityByName("Holey's Head Collider");
						headCollider.GetComponent<PlayerHeadCollider>().SetToPlayer();
						Entity Moley = ECSManager.FindEntityByName("Moley");
						if (PhysicsSystem.IsCollisionStay(headCollider.ID, Moley.ID))
						{
							vec3 moleyPos = Moley.transform.Position;
							moleyPos.y += currOffset + GetComponent<CapsuleCollider>().HalfHeight * 2f + Moley.GetComponent<CapsuleCollider>().HalfHeight + 3.5f;
							Moley.transform.Position = moleyPos;
							PhysicsSystem.SetLinearVelocity(Moley.ID, vec3.Zero);
						}
						PhysicsSystem.SetLinearVelocity(ID, vec3.Zero);
					}

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
				//default model
				dirVec.y = 0;
				if (dirVec != new vec3() && isGrounded)
				{
					dirVec = dirVec.Normalized;

					//Walking animation
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

			#endregion
			//Do NOT REMOVE THIS for some reason it stops the mole when its tall from flying idk dont ask me
			dirVec.y = 0;
			dirVec = dirVec.NormalizedSafe;

			playerDirection = lastPlayerDirection + turnDirection;
			playerDirection = (playerDirection % 360);


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

			isGrounded = false;

			if (RespawnPlayer)
			{
				if (RespawnTimer > 0)
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