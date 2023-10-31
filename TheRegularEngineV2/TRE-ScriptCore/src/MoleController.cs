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
        //Maxium height the player can jump
        private Vector3 maxHeight = new Vector3(0, 10000, 0);
        //direction vector
        private Vector3 dirVec;
        //Movement Vector
        Vector3 movementVector = Vector3.zero;
        //Max Velocity vector
        private float maxVelocity = 20f;
        //Acceleration
        private float acceleration = 300f;
        //Deceleration
        private float deceleration = -10f;
        //velocity in the air
        private float airVelocity = 8f;
        //force direction
        private Vector3 forceDirection = Vector3.zero;
        //final velocity
        private Vector3 finalVelocity = Vector3.zero;

        //Wake up the mole
        private bool isAwake = false;


		//check if player used super power
		private bool isScaled = false;
		//Default scale
		private float defaultScale = 5;
		//Increase character scale
		private float superScale = 1;
        private Vector3 fat = new Vector3(3f, 2f, 3f);
		//Return width back to 0
		private Vector3 thin = new Vector3(0.01f, 0.01f, 0.01f);

		private Vector3 playerDirection = new Vector3(0,0,1);

		//For camera controller
		private Entity Trigger_A;
		private Entity Trigger_B;
		private Entity Trigger_C;
		private Entity Trigger_D;
		private Entity Trigger_E;
		private Entity Trigger_F;
		private Entity Trigger_G;

		public bool regionA;
		public bool regionB;
		public bool regionC;
		public bool regionD;
		public bool regionE;
		public bool regionF;
		public bool regionG;

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

			TransformSystem.SetRotation(this.ID, new Vector3(0, 0, 0));
		}

		public void Update()
		{
            // Move The Test Object 
            TransformSystem.GetPosition(this.ID, out Vector3 pos);
            PhysicsSystem.ConstrainRotationX(this.ID, true);
			PhysicsSystem.ConstrainRotationY(this.ID, true);
			PhysicsSystem.ConstrainRotationZ(this.ID, true);


            //Movement Related stuff
            PhysicsSystem.GetLinearVelocity(this.ID, out Vector3 currVelocity);

            dirVec = new Vector3(0, 0, 0);

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

			if (InputSystem.GetKeyDown(InputKeys.Space))
            {
				if (isGrounded)
				{
					Vector3 maxHeight = new Vector3(currVelocity.x, 1000, currVelocity.z);
					Jump(maxHeight);
				}
			}

			if (InputSystem.GetKeyDown(InputKeys.E))
			{
				if (!isScaled)
				{
					//for fat boi
					PhysicsSystem.ResizeBoxCollider(this.ID, fat);
					//tall boi
					//PhysicsSystem.ResizeCapsuleCollider(this.ID, defaultScale, superScale);
					isScaled = true;
				}
				else if (isScaled)
				{
					//for fat boi
                    PhysicsSystem.ResizeBoxCollider(this.ID, thin);
					//for tall boi
                    //PhysicsSystem.ResizeCapsuleCollider(this.ID, 3, 2);
					isScaled = false;
				}
			}

			dirVec.Normalize();

            if (dirVec != Vector3.zero)
            {
                //Debug.Log(dirVec.x + " " + dirVec.z);
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
                    //Debug.Log("Max velocity is:" + currVelocity.x + currVelocity.y + currVelocity.z);
                }
                //Debug.Log("current velocity is:" + currVelocity.x + currVelocity.y + currVelocity.z);
            }

			TransformSystem.SetRotation(this.ID, playerDirection);

			regionA = PhysicsSystem.IsTriggerEnter(this.ID, Trigger_A.ID) || PhysicsSystem.IsTriggerStay(this.ID, Trigger_A.ID);
			regionB = PhysicsSystem.IsTriggerEnter(this.ID, Trigger_B.ID) || PhysicsSystem.IsTriggerStay(this.ID, Trigger_B.ID);
			regionC = PhysicsSystem.IsTriggerEnter(this.ID, Trigger_C.ID) || PhysicsSystem.IsTriggerStay(this.ID, Trigger_C.ID);
			regionD = PhysicsSystem.IsTriggerEnter(this.ID, Trigger_D.ID) || PhysicsSystem.IsTriggerStay(this.ID, Trigger_D.ID);
			regionE = PhysicsSystem.IsTriggerEnter(this.ID, Trigger_E.ID) || PhysicsSystem.IsTriggerStay(this.ID, Trigger_E.ID);
			regionF = PhysicsSystem.IsTriggerEnter(this.ID, Trigger_F.ID) || PhysicsSystem.IsTriggerStay(this.ID, Trigger_F.ID);
			regionG = PhysicsSystem.IsTriggerEnter(this.ID, Trigger_G.ID) || PhysicsSystem.IsTriggerStay(this.ID, Trigger_G.ID);
		}
		private void Jump(Vector3 JumpHeight)
		{
			//PhysicsSystem.SetLinearVelocity(this.ID, JumpHeight);
			PhysicsSystem.AddForce(this.ID, JumpHeight, ForceMode.Impulse);
		}

		public static float lerp(float start, float end, float t)
		{
			if (t > 1)
				t = 1;
			else if (t < 0)
				t = 0;
			return start + (end - start) * t;
		}

		private void OnCollisionStay(System.UInt64 otherID)
		{
			isGrounded = false;

			Entity other = new Entity(otherID);
			if(PhysicsSystem.IsCollisionStay(this.ID, otherID))
			{
				if(EngineGetTag(otherID) == "Ground")
				{
					isGrounded = true;
				}
				else
				{
					isGrounded = false;
				}
			}
		}
	}
}
