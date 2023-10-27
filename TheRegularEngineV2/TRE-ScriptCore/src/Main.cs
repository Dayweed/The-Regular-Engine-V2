using System;
using System.Reflection;
using System.Runtime.CompilerServices;

namespace TRE
{
	using PS = PhysicsSystem;

	public class Main
	{
		// Scripting Initialization
		HumanCentipede humanCentipedo = new HumanCentipede();
		RandomizeFallingObjLocation testingRandoFall = new RandomizeFallingObjLocation();

        //create the test object and temp object not too sure if the temp object is linked in some 
        public Entity Temp = new Entity("Temp");
		public Entity Test = new Entity("Test");
        public Entity Plane_collider = new Entity("Plane collider");
        private bool isJumping = false;
        private bool isGrounded = true;
        private Vector3 maxJumpHeight = new Vector3(0, 20, 0);
        private Vector3 maxHeight = new Vector3(0, 0, 0);

        public Main()
		{
			//Temp.id = ECSManager.CreateEntity(Temp.name);
			//Console.WriteLine("Hello World from C#!");

			Test.id = ECSManager.FindIDFromName(Test.name);
            Plane_collider.id = ECSManager.FindIDFromName(Plane_collider.name);
			Console.WriteLine("Test ID: " + Test.id);

			Console.WriteLine("Entity: " + Test.GetActive());
            Test.SetActive(false);
            Console.WriteLine("Entity: " + Test.GetActive());
            Test.SetActive(true);
            Console.WriteLine("Entity: " + Test.GetActive());

            //Console.WriteLine("Tag: " + Test.GetTag());
            Test.SetTag("UwU");
            //Console.WriteLine("Tag: " + Test.GetTag());
            Console.WriteLine("Is UwU? " + Test.CompareTag("UwU"));
            Console.WriteLine("Is OwO? " + Test.CompareTag("OwO"));
        }

		public void Start()
		{
			humanCentipedo.Start();
			testingRandoFall.Start();
        }

		public void Update()
		{
            // Script calling
            humanCentipedo.Update();
			//testingRandoFall.Update();

			// Move The Test Object 
			TransformSystem.GetPosition(Test.id, out Vector3 pos);
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
                dirVec.x += -1;
            }

            if (InputSystem.GetKeyDown(InputKeys.D))
            {
                dirVec.x += 1;
            }

            if (InputSystem.GetKeyDown(InputKeys.Space))
            {
                // if the player JUST starts to touch the ground OR has been chilling on the ground for a while
                isGrounded = PS.IsCollisionEnter(Test.id, Plane_collider.id) || PS.IsCollisionStay(Test.id, Plane_collider.id);
                maxHeight = pos + maxJumpHeight; // this was not it, chief... :(

               if (isGrounded)
                    Jump(new Vector3(0, 35, 0)); // ah, much better (for forcemode.velchange)
                    // Jump(new Vector3(0, 13000, 0)); // uh oh beeeg number (for forcemode.force)
            }

            dirVec.Normalize();

            Vector3 tmp = dirVec * 60;

            PS.AddForce(Test.id, tmp, PS.ForceMode.Force);
        }

        private void Jump(Vector3 maxHeight)
        {
            PS.AddForce(Test.id, maxHeight, PS.ForceMode.VelocityChange);
        }
	}
}
