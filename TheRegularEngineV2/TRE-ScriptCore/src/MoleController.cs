using System;
using System.Collections.Generic;
using System.Reflection;
using System.Runtime.CompilerServices;
using static System.Runtime.CompilerServices.RuntimeHelpers;
using System.Threading;
namespace TRE
{
	public class MoleController : Entity
	{
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
		private bool isScaled = false;
		//Box Collider
		private Vector3 current = new Vector3(0.01f, 0.01f, 0.01f);
		private Vector3 thin = new Vector3(0.01f, 0.01f, 0.01f);
		private Vector3 fat = new Vector3(6f, 5f, 5.5f);
		//Player Scallings
		private Vector3 defaultXform = new Vector3(0.75f, 0.75f, 0.75f);
		private Vector3 scaledXform = new Vector3(2f, 1f, 2f);

		private Vector3 playerDirection = new Vector3(0,0,1);

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

		public void Start()
		{
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
			PhysicsSystem.ConstrainRotationX(this.ID, true);
			PhysicsSystem.ConstrainRotationY(this.ID, true);
			PhysicsSystem.ConstrainRotationZ(this.ID, true);
		}

		public void Update()
		{
			// Move The Test Object 
			TransformSystem.GetPosition(this.ID, out Vector3 pos);

			//Movement Related stuff
			PhysicsSystem.GetLinearVelocity(this.ID, out Vector3 currVelocity);

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

			if(InputSystem.GetKeyDown(InputKeys.W))
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
            #endregion

            #region Ability
            if (InputSystem.GetKeyTrigger(InputKeys.E))
			{
				isScaled = !isScaled;
			}

			if (isScaled == false)
			{
				//for fat boi
				current = MathF.Vec3Lerp(current, thin, 0.2f);
				PhysicsSystem.ResizeBoxCollider(this.ID, current);
				TransformSystem.SetScaling(this.ID, defaultXform);
			}
			else
			{
				//for fat boi
				current = MathF.Vec3Lerp(current, fat, 0.2f);
				PhysicsSystem.ResizeBoxCollider(this.ID, current);
				TransformSystem.SetScaling(this.ID, scaledXform);
			}
            #endregion

            dirVec.Normalize();

			if (dirVec != Vector3.zero)
			{
				if (Math.Sqrt(currVelocity.x * currVelocity.x + currVelocity.z * currVelocity.z) < maxVelocity)
				{
					finalVelocity = currVelocity + (dirVec * acceleration * Time.GetDeltaTime());
					PhysicsSystem.SetLinearVelocity(this.ID, finalVelocity);
				}
				else
				{
					Vector3 tmp = dirVec * maxVelocity;
					finalVelocity = new Vector3(tmp.x, currVelocity.y, tmp.z);
					PhysicsSystem.SetLinearVelocity(this.ID, finalVelocity);
				}
			}

			TransformSystem.SetRotation(this.ID, playerDirection);

			regionA = PhysicsSystem.IsTriggerEnter(this.ID, Trigger_A.ID) || PhysicsSystem.IsTriggerStay(this.ID, Trigger_A.ID);
			regionB = PhysicsSystem.IsTriggerEnter(this.ID, Trigger_B.ID) || PhysicsSystem.IsTriggerStay(this.ID, Trigger_B.ID);
			regionC = PhysicsSystem.IsTriggerEnter(this.ID, Trigger_C.ID) || PhysicsSystem.IsTriggerStay(this.ID, Trigger_C.ID);
			regionD = PhysicsSystem.IsTriggerEnter(this.ID, Trigger_D.ID) || PhysicsSystem.IsTriggerStay(this.ID, Trigger_D.ID);
			regionE = PhysicsSystem.IsTriggerEnter(this.ID, Trigger_E.ID) || PhysicsSystem.IsTriggerStay(this.ID, Trigger_E.ID);
			regionF = PhysicsSystem.IsTriggerEnter(this.ID, Trigger_F.ID) || PhysicsSystem.IsTriggerStay(this.ID, Trigger_F.ID);
			regionG = PhysicsSystem.IsTriggerEnter(this.ID, Trigger_G.ID) || PhysicsSystem.IsTriggerStay(this.ID, Trigger_G.ID);
			regionH = PhysicsSystem.IsTriggerEnter(this.ID, Trigger_H.ID) || PhysicsSystem.IsTriggerStay(this.ID, Trigger_H.ID);

			if(PhysicsSystem.IsTriggerEnter(this.ID, Key.ID))
			{
				Key.SetActive(false);
				TransformSystem.SetPosition(FinalPlatform.ID, new Vector3(100, 9 ,-302));
				Debug.Log("Key Collected");
			}
		}
		private void Jump(Vector3 JumpHeight)
		{
			//PhysicsSystem.SetLinearVelocity(this.ID, JumpHeight);
			PhysicsSystem.AddForce(this.ID, JumpHeight, ForceMode.VelocityChange);
		}

		private void OnCollisionStay(System.UInt64 otherID)
		{
			isGrounded = false;

			Entity other = new Entity(otherID);
			if(PhysicsSystem.IsCollisionStay(this.ID, otherID))
			{
				if(EngineGetTag(otherID) == "Ground" || EngineGetTag(otherID) == "Player")
				{
					isGrounded = true;
				}
				else
				{
					isGrounded = false;
				}
			}
			if(PhysicsSystem.IsCollisionExit(this.ID, otherID))
			{
				if(EngineGetTag(otherID) == "Player")
				{
					PhysicsSystem.GetLinearVelocity(this.ID, out Vector3 output);
					if (output.y > maxVelocity)
						output.y = maxVelocity;
					PhysicsSystem.SetLinearVelocity(this.ID, output);
				}
			}
		}
	}
}
