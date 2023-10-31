using System;
using System.Reflection;
using System.Runtime.CompilerServices;

namespace TRE
{
	using PS = PhysicsSystem;

	public class Main
	{
		// Scripting Initialization
		//HumanCentipede humanCentipedo = new HumanCentipede();
		//RandomizeFallingObjLocation testingRandoFall = new RandomizeFallingObjLocation();

		/* Commenting out uneeded code to prevent Console Panel spam...
		//create the test object and temp object not too sure if the temp object is linked in some 
		public Entity Temp = new Entity(new long(), "Temp");
		public Entity Test = new Entity(new long(), "Test");
		public Entity Plane_collider = new Entity(new long(), "Plane collider");

		//check if player is on the ground (for now , just a plane)
		private bool isGrounded = true;
		//Maxium height the player can jump
		private Vector3 maxHeight = new Vector3(0, 240, 0);


		//check if player used super power
		private bool isScaled = false;
		private float defaultScale = 1;
		private float superScale = 50;
		*/

		public Main()
		{
			//Temp.id = ECSManager.CreateEntity(Temp.name);
			//Console.WriteLine("Hello World from C#!");

			/* Commenting out uneeded code to prevent Console Panel spam...
			Test.ID = ECSManager.FindIDFromName(Test.name);
			Plane_collider.ID = ECSManager.FindIDFromName(Plane_collider.name);
			Console.WriteLine("Test ID: " + Test.ID);


			//Console.WriteLine("Tag: " + Test.GetTag());
			Test.SetTag("UwU");
			//Console.WriteLine("Tag: " + Test.GetTag());
			Console.WriteLine("Is UwU? " + Test.CompareTag("UwU"));
			Console.WriteLine("Is OwO? " + Test.CompareTag("OwO"));
			*/
		}

		public void Start()
		{
			//humanCentipedo.Start();
			//testingRandoFall.Start();
		}

		public void Update()
		{
			// Script calling
			//humanCentipedo.Update();
			//testingRandoFall.Update();
			
			/* Commenting out uneeded code to prevent Console Panel spam...
			// Move The Test Object 
			TransformSystem.GetPosition(Test.ID, out Vector3 pos);
			//

			Vector3 dirVec = new Vector3(0, 0, 0);

			if (InputSystem.GetKeyDown(InputKeys.W))
			{
				dirVec.z += 1;
				//PS.AddForce(Mole.id, dirVec);
			}

			if (InputSystem.GetKeyDown(InputKeys.S))
			{
				dirVec.z += -1;
				//PS.AddForce(Mole.id, dirVec);
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
				isGrounded = PS.IsCollisionEnter(Test.ID, Plane_collider.ID) || PS.IsCollisionStay(Test.ID, Plane_collider.ID);

				if (isGrounded)
				{
					Jump(maxHeight); // uh oh beeeg number (for forcemode.force)
					// Jump(new Vector3(0, 35, 0)); // ah, much better (for forcemode.velchange)
				}
			}

			if (InputSystem.GetKeyDown(InputKeys.E))
			{
				if (!isScaled)
				{
					PS.ResizeSphereCollider(Test.ID, superScale);
					isScaled = true;
				}
				else if (isScaled)
				{
					PS.ResizeSphereCollider(Test.ID, defaultScale);
					isScaled = false;
				}
			}

			dirVec.Normalize();

			Vector3 tmp = dirVec * 60;

			PS.AddForce(Test.ID, tmp, ForceMode.Force);
			*/
		}

		private void Jump(Vector3 JumpHeight)
		{
			/* Commenting out uneeded code to prevent Console Panel spam...
			PhysicsSystem.AddForce(Test.ID, JumpHeight, ForceMode.Force);
			// PS.AddForce(Test.id, 35, PS.ForceMode.VelocityChange);
			*/
		}
	}

	public class Testing : Entity
	{
		public float test = 0.0f;
		public bool test2 = false;
		void OnCreate()
		{
			Console.WriteLine("Testing OnCreate");
			Console.WriteLine($"Testing OnCreate ID - {ID}");
		}

		void Update()
		{
			Console.WriteLine("Testing Update");
			Console.WriteLine($"Testing Update ID - {ID}");
			Console.WriteLine($"Testing Update Position - {transform.Position.x} , {transform.Position.y} ");
		}
	}

}
