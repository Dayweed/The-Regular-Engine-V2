using System;
using System.Collections.Generic;
using System.Reflection;
using System.Runtime.CompilerServices;
using static System.Runtime.CompilerServices.RuntimeHelpers;
using System.Threading;
namespace TRE
{
	using PS = PhysicsSystem;
    using AS = Audio;

    public class HoleyController : Entity
	{
		public PowerUpManager MyPowerManager;

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
		private float superRadius = 3.5f;
		private float currentRadius = 2f;
		private float defaultHeight = 1f;
		private float superHeight = 0.1f;
		private float currentHeight = 1f;
		//Player Scallings
		private Vector3 defaultXform = new Vector3(0.75f, 0.75f, 0.75f);
		private Vector3 scaledXform = new Vector3(2f, 1f, 2f);

		private Vector3 playerDirection = new Vector3(0, 0, 1);

		private float lerpSpeed = 0.05f;

		//For camera controller
		private Entity Trigger_A;
		private Entity Trigger_B;
		private Entity Trigger_C;
		private Entity Trigger_D;
		private Entity Trigger_E;
		private Entity Trigger_F;
		private Entity Trigger_G;
		private Entity Trigger_H;
		private Entity Key;
		private Entity FinalPlatform;

		public bool regionA;
		public bool regionB;
		public bool regionC;
		public bool regionD;
		public bool regionE;
		public bool regionF;
		public bool regionG;
		public bool regionH;

		public float elapsedTime = 0.0f;

        private bool isMoving = false;

        public void Start()
		{
			MyPowerManager = parenting.GetChildFromName("Power Manager").GetComponent<PowerUpManager>();

			Trigger_A = ECSManager.FindEntityByName("Trigger_A");
			Debug.Log("Trigger_A ID is " + Trigger_A.ID);

			Trigger_B = ECSManager.FindEntityByName("Trigger_B");
			Debug.Log("Trigger_B ID is " + Trigger_B.ID);

			Trigger_C = ECSManager.FindEntityByName("Trigger_C");
			Debug.Log("Trigger_C ID is " + Trigger_C.ID);

			Trigger_D = ECSManager.FindEntityByName("Trigger_D");
			Debug.Log("Trigger_D ID is " + Trigger_D.ID);

			Trigger_E = ECSManager.FindEntityByName("Trigger_E");
			Debug.Log("Trigger_E ID is " + Trigger_E.ID);

			Trigger_F = ECSManager.FindEntityByName("Trigger_F");
			Debug.Log("Trigger_F ID is " + Trigger_F.ID);

			Trigger_G = ECSManager.FindEntityByName("Trigger_G");
			Debug.Log("Trigger_G ID is " + Trigger_G.ID);

			Trigger_H = ECSManager.FindEntityByName("Trigger_H");
			Debug.Log("Trigger_H ID is " + Trigger_H.ID);

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
				isMoving = true;
			}

			if (InputSystem.GetKeyDown(InputKeys.S))
			{
				dirVec.z += 1;
				playerDirection.y = 0;
                isMoving = true;
            }

			if (InputSystem.GetKeyDown(InputKeys.A))
			{
				dirVec.x += -1;
				playerDirection.y = 270;
                isMoving = true;
            }

			if (InputSystem.GetKeyDown(InputKeys.D))
			{
				dirVec.x += 1;
				playerDirection.y = 90;
                isMoving = true;
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
					Vector3 maxHeight = new Vector3(0, 35, 0);
					Jump(maxHeight);
				}
			}

            //SFX
            if (isMoving && AS.GetIsPlaying(this.ID) == false)
            {
                //AS.SetPlay(this.ID);
            }
            else if (AS.GetIsPlaying(this.ID))
            {
                //AS.StopAudio(this.ID);
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
				if (mainBlueberry)
				{
					isScaled = !isScaled;
				}
			}

			if ((isScaled == false || !mainBlueberry))
			{
				currentHeight = MathF.Lerp(currentHeight, defaultHeight, lerpSpeed);
				currentRadius = MathF.Lerp(currentRadius, defaultRadius, lerpSpeed);
				PS.ResizeCapsuleCollider(this.ID, currentRadius, currentHeight);
				TransformSystem.SetScaling(this.ID, defaultXform);
			}
			else
			{
				//for fat boi
				currentHeight = MathF.Lerp(currentHeight, superHeight, lerpSpeed);
				currentRadius = MathF.Lerp(currentRadius, superRadius, lerpSpeed);
				PS.ResizeCapsuleCollider(this.ID, currentRadius, currentHeight);
				TransformSystem.SetScaling(this.ID, scaledXform);
			}
			#endregion

			#region Drop
			// Check if can trigger ability
			if (InputSystem.GetKeyTrigger(InputKeys.LeftShift))
			{
				MyPowerManager.DropMain();
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

			regionA = IsInsideTrigger(Trigger_A);
			regionB = IsInsideTrigger(Trigger_B);
			regionC = IsInsideTrigger(Trigger_C);
			regionD = IsInsideTrigger(Trigger_D);
			regionE = IsInsideTrigger(Trigger_E);
			regionF = IsInsideTrigger(Trigger_F);
			regionG = IsInsideTrigger(Trigger_G);
			regionH = IsInsideTrigger(Trigger_H);

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
			if (PS.IsCollisionStay(this.ID, otherID))
			{
				if (EngineGetTag(otherID) == "Ground" || EngineGetTag(otherID) == "Blue")
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
				if (EngineGetTag(otherID) == "Player")
				{
					PS.GetLinearVelocity(this.ID, out Vector3 output);
					if (output.y > maxVelocity)
						output.y = maxVelocity;
					PS.SetLinearVelocity(this.ID, output);
				}
			}
		}

		private bool IsInsideTrigger(Entity entity)
		{
			return PS.IsTriggerEnter(this.ID, entity.ID) || PS.IsTriggerStay(this.ID, entity.ID);
		}
	}
}
