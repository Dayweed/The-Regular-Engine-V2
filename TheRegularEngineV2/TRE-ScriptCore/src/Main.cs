using System;
using System.Reflection;
using System.Runtime.CompilerServices;

namespace TRE
{
	public class Main
	{
		//create the test object and temp object not too sure if the temp object is linked in some 
		public Entity Temp = new Entity("Temp");
		public Entity Test = new Entity("Test");

		public Main()
		{
			Temp.id = ECSManager.CreateEntity(Temp.name);
			Console.WriteLine("Hello World from C#!");

			Test.id = ECSManager.FindIDFromName(Test.name);
			Console.WriteLine("Test ID: " + Test.id);

			Console.WriteLine("Entity: " + Entity.GetActive(Test.id));
            Entity.SetActive(Test.id, false);
            Console.WriteLine("Entity: " + Entity.GetActive(Test.id));
            Entity.SetActive(Test.id, true);
        }

		public void Update()
		{
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
