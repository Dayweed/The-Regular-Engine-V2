using System;
using System.Reflection;
using System.Runtime.CompilerServices;


namespace TRE
{

    public class Main
    {

        public Entity Temp = new Entity("Temp");
        public Entity Test = new Entity("Test");
        
        public Main()
        {
            Temp.id = ECSManager.CreateEntity(Temp.name);
            Console.WriteLine("Hello World from C#!");

            Test.id = ECSManager.FindIDFromName(Test.name);
            Console.WriteLine("Test ID: " + Test.id);
        }

        public void Update()
        {
            // Move The Test Object 
             TransformSystem.GetPosition(Test.id, out Vector3 pos);

             pos.x += 0.1f;
             pos.y += 0.1f;
             pos.z += 0.1f;

             TransformSystem.SetPosition(Test.id, pos);

             if (InputSystem.GetKeyDown(InputKeys.Space))
             {
                 Console.WriteLine("Space Pressed!");
             }

        }


        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern static string GetTestGUID();
    }

}

