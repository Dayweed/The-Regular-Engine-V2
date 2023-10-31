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
        private Vector3 maxHeight = new Vector3(0, 1000, 0);
        //direction vector
        private Vector3 dirVec;
        //Movement Vector
        Vector3 movementVector = Vector3.zero;
        //Max Velocity vector
        private float maxVelocity = 5f;
        //Acceleration
        private float acceleration = 100f;
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
        private float defaultScale = 1;
        //Increase character scale
        private float superScale = 5;
        //Increase player width
        private Vector3 fat = new Vector3(3f, 2f, 3f);
		//Return width back to 0
		private Vector3 thin = new Vector3(0.01f, 0.01f, 0.01f);
        //For now the floor collision
        public Entity Collider;
        private Entity Trigger_Start;
        private Entity Trigger_1;
        private Entity Trigger_2;

        private CameraController cameraController;

		public float elapsedTime = 0.0f;

		public void Start()
		{
			Trigger_Start = ECSManager.FindEntityByName("Trigger_Start");
			Debug.Log("Trigger_Start ID is " + Trigger_Start.ID);

			Trigger_1 = ECSManager.FindEntityByName("Trigger_1");
			Debug.Log("Trigger_1 ID is " + Trigger_1.ID);

			Trigger_2 = ECSManager.FindEntityByName("Trigger_2");
			Debug.Log("Trigger_2 ID is " + Trigger_2.ID);

			cameraController = ECSManager.FindEntityByName("Main Camera").GetComponent<CameraController>();
		}

		public void Update()
		{
            // Move The Test Object 
            TransformSystem.GetPosition(this.ID, out Vector3 pos);
            PhysicsSystem.ConstrainRotationX(this.ID, true);
            PhysicsSystem.ConstrainRotationZ(this.ID, true);

            //Movement Related stuff
            PhysicsSystem.GetLinearVelocity(this.ID, out Vector3 currVelocity);

            dirVec = new Vector3(0, 0, 0);

			if (InputSystem.GetKeyDown(InputKeys.W))
			{
				dirVec.z += -1;
			}

			if (InputSystem.GetKeyDown(InputKeys.S))
			{
				dirVec.z += 1;
			}

			if (InputSystem.GetKeyDown(InputKeys.A))
			{
				dirVec.x += -1;
			}

			if (InputSystem.GetKeyDown(InputKeys.D))
			{
				dirVec.x += 1;
			}

			if (InputSystem.GetKeyDown(InputKeys.Space))
            {
				if (isGrounded)
					Jump(maxHeight);
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
                Debug.Log("current velocity is:" + currVelocity.x + currVelocity.y + currVelocity.z);
            }

            cameraController.regionStart = PhysicsSystem.IsTriggerEnter(this.ID, Trigger_Start.ID) || PhysicsSystem.IsTriggerStay(this.ID, Trigger_Start.ID); ;
			cameraController.region1 = PhysicsSystem.IsTriggerEnter(this.ID, Trigger_1.ID) || PhysicsSystem.IsTriggerStay(this.ID, Trigger_1.ID);
			cameraController.region2 = PhysicsSystem.IsTriggerEnter(this.ID, Trigger_2.ID) || PhysicsSystem.IsTriggerStay(this.ID, Trigger_2.ID);
		}
		private void Jump(Vector3 JumpHeight)
		{
			PhysicsSystem.AddForce(this.ID, JumpHeight, ForceMode.Acceleration);
		}

		public static float lerp(float start, float end, float t)
		{
			if (t > 1)
				t = 1;
			else if (t < 0)
				t = 0;
			return start + (end - start) * t;
		}

		private void OnTriggerStay(System.UInt64 otherID)
		{
			Entity other = new Entity(otherID);
            if (PhysicsSystem.IsCollisionStay(this.ID, otherID) == true)
                isGrounded = true;
			else
				isGrounded = false;

        }
	}
}
