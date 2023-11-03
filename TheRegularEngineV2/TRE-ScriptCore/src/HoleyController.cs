using System;
using System.Collections.Generic;
using System.Reflection;
using System.Runtime.CompilerServices;
using static System.Runtime.CompilerServices.RuntimeHelpers;
using System.Threading;
namespace TRE
{
	using PS = PhysicsSystem;

	public class HoleyController : Entity
	{
		public PowerUpManager MyPowerManager;

        //Check if player is boosted jump
        private bool isBoostedJump = false;
        //check if player is on the ground (for now , just a plane)
        private bool isGrounded = true;
		//direction vector
		private Vector3 dirVec;
		//Max Velocity vector
		private float maxVelocity = 15f;
		//Acceleration
		private float acceleration = 300f;
		//final velocity
		private Vector3 finalVelocity = Vector3.zero;

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
		private Vector3 defaultXform = new Vector3(0.75f, 0.75f, 0.75f);
		private Vector3 blueberryscaledXform = new Vector3(2f, 1f, 2f);
		private Vector3 strawberryscaledXform = new Vector3(2f, 1f, 0.5f);
		private Vector3 currentXform = new Vector3(0.75f, 0.75f, 0.75f);

		private Vector3 playerDirection = new Vector3(0, 0, 1);

		private float lerpSpeed = 0.05f;

		//For camera controller
		private Entity Key;
		private Entity FinalPlatform;

		public float elapsedTime = 0.0f;

		public void Start()
		{
			MyPowerManager = parenting.GetChildFromName("Power Manager").GetComponent<PowerUpManager>();

			Key = ECSManager.FindEntityByName("Key");
			Debug.Log("Key ID is " + Key.ID);

			FinalPlatform = ECSManager.FindEntityByName("Final_Platform");
			Debug.Log("FinalPlatform ID is " + FinalPlatform.ID);

			TransformSystem.SetRotation(this.ID, new Vector3(0, 0, 0));
			PS.ConstrainRotationX(this.ID, true);
			PS.ConstrainRotationY(this.ID, true);
			PS.ConstrainRotationZ(this.ID, true);
		}

		public void Update()
		{
			// Move The Test Object 
			TransformSystem.GetPosition(this.ID, out Vector3 pos);

			//Movement Related stuff
			PS.GetLinearVelocity(this.ID, out Vector3 currVelocity);

			dirVec = new Vector3(0, 0, 0);
			#region Movement
			if (InputSystem.GetKeyDown(InputKeys.W))
			{
				dirVec.z += -1;
				playerDirection.y = 180;
			}

			if (InputSystem.GetKeyDown(InputKeys.S))
			{
				dirVec.z += 1;
				playerDirection.y = 0;
			}

			if (InputSystem.GetKeyDown(InputKeys.A))
			{
				dirVec.x += -1;
				playerDirection.y = 270;
			}

			if (InputSystem.GetKeyDown(InputKeys.D))
			{
				dirVec.x += 1;
				playerDirection.y = 90;
			}

			if (InputSystem.GetKeyDown(InputKeys.W))
			{
				if (InputSystem.GetKeyDown(InputKeys.D))
				{
					playerDirection.y = 135;
				}
				if (InputSystem.GetKeyDown(InputKeys.A))
				{
					playerDirection.y = 225;
				}
			}

			if (InputSystem.GetKeyDown(InputKeys.S))
			{
				if (InputSystem.GetKeyDown(InputKeys.D))
				{
					playerDirection.y = 45;
				}
				if (InputSystem.GetKeyDown(InputKeys.A))
				{
					playerDirection.y = 315;
				}
			}

			if (InputSystem.GetKeyTrigger(InputKeys.Space))
			{
				if (isGrounded)
				{
					// Boosted Jump
					if (isBoostedJump)
					{
						Vector3 maxHeight = new Vector3(0, 70, 0);
						Jump(maxHeight);
					}
					else
					{
						Vector3 maxHeight = new Vector3(0, 35, 0);
						Jump(maxHeight);
					}
				}
            }
            #endregion

            #region Swap
            // Check if can swap ability
            if (InputSystem.GetKeyTrigger(InputKeys.Q))
            {
                MyPowerManager.SwapPowerUps();
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
				currentHeight = MathF.Lerp(currentHeight, defaultHeight, lerpSpeed);
				currentRadius = MathF.Lerp(currentRadius, defaultRadius, lerpSpeed);
				PS.ResizeCapsuleCollider(this.ID, currentRadius, currentHeight);
                currentXform.x = MathF.Lerp(currentXform.x, defaultXform.x, lerpSpeed);
                currentXform.y = MathF.Lerp(currentXform.y, defaultXform.y, lerpSpeed);
                currentXform.z = MathF.Lerp(currentXform.z, defaultXform.z, lerpSpeed);
                TransformSystem.SetScaling(this.ID, currentXform);
            }
			else if (mainBlueberry)
			{
				//for fat boi
				currentHeight = MathF.Lerp(currentHeight, blueberrysuperHeight, lerpSpeed);
				currentRadius = MathF.Lerp(currentRadius, blueberrysuperRadius, lerpSpeed);
				PS.ResizeCapsuleCollider(this.ID, currentRadius, currentHeight);
                currentXform.x = MathF.Lerp(currentXform.x, blueberryscaledXform.x, lerpSpeed);
                currentXform.y = MathF.Lerp(currentXform.y, blueberryscaledXform.y, lerpSpeed);
                currentXform.z = MathF.Lerp(currentXform.z, blueberryscaledXform.z, lerpSpeed);
                TransformSystem.SetScaling(this.ID, currentXform);
            }
			else if (mainStrawberry)
			{
				//for fat boi
				currentHeight = MathF.Lerp(currentHeight, strawberrysuperHeight, lerpSpeed);
				currentRadius = MathF.Lerp(currentRadius, strawberrysuperRadius, lerpSpeed);
				PS.ResizeCapsuleCollider(this.ID, currentRadius, currentHeight);
                currentXform.x = MathF.Lerp(currentXform.x, strawberryscaledXform.x, lerpSpeed);
                currentXform.y = MathF.Lerp(currentXform.y, strawberryscaledXform.y, lerpSpeed);
                currentXform.z = MathF.Lerp(currentXform.z, strawberryscaledXform.z, lerpSpeed);
                TransformSystem.SetScaling(this.ID, currentXform);
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

			

			if (PS.IsTriggerEnter(this.ID, Key.ID))
			{
				Key.SetActive(false);
				TransformSystem.SetPosition(FinalPlatform.ID, new Vector3(100, 9, -302));
				Debug.Log("Key Collected");
			}
		}

		private void Jump(Vector3 JumpHeight)
		{
			//PhysicsSystem.SetLinearVelocity(this.ID, JumpHeight);
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
				if (EngineGetTag(otherID) == "Ground" || EngineGetTag(otherID) == "JumpPad" || EngineGetTag(otherID) == "Blue")
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
					PS.GetLinearVelocity(this.ID, out Vector3 output);
					if (output.y > maxVelocity)
						output.y = maxVelocity;
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
