using System;
using System.Reflection;
using System.Runtime.CompilerServices;

namespace TRE
{
	public class Main
	{
		// Scripting Initialization
		HumanCentipede humanCentipedo = new HumanCentipede();
		RandomizeFallingObjLocation testingRandoFall = new RandomizeFallingObjLocation();

        //create the test object and temp object not too sure if the temp object is linked in some 
        public Entity Temp = new Entity("Temp");
		public Entity Test = new Entity("Test");

		public Main()
		{
			Temp.id = ECSManager.CreateEntity(Temp.name);
			Console.WriteLine("Hello World from C#!");

			Test.id = ECSManager.FindIDFromName(Test.name);
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
			testingRandoFall.Start();
        }

		public void Update()
		{
            // Script calling
            //humanCentipedo.Update();
			testingRandoFall.Update();

			// Move The Test Object 
			TransformSystem.GetPosition(Test.id, out Vector3 pos);
			//
			Vector3 tmp = new Vector3(0, 0, 0);

			if (InputSystem.GetKeyDown(InputKeys.W))
			{
				tmp.x = 60;
				PhysicsSystem.AddForce(Test.id, tmp);
			}

			if (InputSystem.GetKeyDown(InputKeys.S))
			{
				tmp.x = -60;
				PhysicsSystem.AddForce(Test.id, tmp);
			}

			if (InputSystem.GetKeyDown(InputKeys.Space))
			{
				Console.WriteLine("Space Pressed!");
			}
		}
	}
}
