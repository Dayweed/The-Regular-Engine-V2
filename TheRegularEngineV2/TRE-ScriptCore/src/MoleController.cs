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
		private Vector3 maxHeight = new Vector3(0, 2000, 0);
		//direction vector
		private Vector3 dirVec;
		//Movement Vector
		Vector3 movementVector = Vector3.zero;
		//Wake up the mole
		private bool isAwake = false;


		//check if player used super power
		private bool isScaled = false;
		//Default scale
		private float defaultScale = 5;
		//Increase character scale
		private float superScale = 1;
		//For now the floor collision
		public Entity Plane_collider;
		private Entity Trigger_Start;
		private Entity Trigger_1;
		private Entity Trigger_2;

		private CameraController cameraController;

		public float elapsedTime = 0.0f;

		public void Start()
		{
			Plane_collider = ECSManager.FindEntityByName("Plane collider");
			Debug.Log("My ID is " + this.ID);
			Debug.Log("Plane ID is " + Plane_collider.ID);

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

			dirVec = new Vector3(0, 0, 0);

			if (InputSystem.GetKeyDown(InputKeys.W))
			{
				dirVec.z += 1;
			}

			if (InputSystem.GetKeyDown(InputKeys.S))
			{
				dirVec.z += -1;
			}

			if (InputSystem.GetKeyDown(InputKeys.A))
			{
				dirVec.x += 1;
			}

			if (InputSystem.GetKeyDown(InputKeys.D))
			{
				dirVec.x += -1;
			}

			if (InputSystem.GetKeyDown(InputKeys.Space))
			{
				// if the player JUST starts to touch the ground OR has been chilling on the ground for a while
				isGrounded = PhysicsSystem.IsCollisionEnter(this.ID, Plane_collider.ID) || PhysicsSystem.IsCollisionStay(this.ID, Plane_collider.ID);

				if (isGrounded)
					Jump(maxHeight);
			}

			if (InputSystem.GetKeyDown(InputKeys.E))
			{
				if (!isScaled)
				{
					superScale = lerp(1, 5, 0.1f);
					Debug.Log("Super Scale: " + superScale);
					PhysicsSystem.ResizeCapsuleCollider(this.ID, superScale, defaultScale);
					superScale = 1;
					isScaled = true;
				}
				else if (isScaled)
				{

					defaultScale = lerp(5, 1, 0.1f);
					defaultScale -= 0.5f * Time.deltaTime;
					PhysicsSystem.ResizeCapsuleCollider(this.ID, defaultScale, defaultScale);
					defaultScale = 5;
					isScaled = false;
				}
			}

			dirVec.Normalize();

			movementVector = dirVec * 10;
			PhysicsSystem.GetLinearVelocity(this.ID, out Vector3 output);
			PhysicsSystem.AddForce(this.ID, movementVector, ForceMode.VelocityChange);

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
		//private void OnTriggerStay(System.UInt64 otherID)
		//{
		//	Entity other = new Entity(otherID);
		//	Core.Log("Triggered with " + ECSManager.FindNameFromID(other.ID));
		//}
	}
}
