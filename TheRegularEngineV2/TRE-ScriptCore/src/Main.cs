using System;
using System.Reflection;
using System.Runtime.CompilerServices;


namespace TRE
{

    public class Demo
    {
        public Demo()
        {
            SpawnObject();
        }


        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern static string SpawnObject();

        
    }

    public class Main
    {
        public Entity Temp;
        public Main()
        {
            Console.WriteLine("Hello World from C#!");
        }

        public void Update()
        {
            // This is the update loop for the main class;
            if (GetTestGUID() != "")
            {
                TransformSystem transformSystem = new TransformSystem();
                transformSystem.transformDemo(GetTestGUID());
            }
        }

        public void Test()
        {
            string id = ECSManager.CreateEntity("ScriptingTest");
            Temp = new Entity(id, "ScriptingTest");
            //Console.WriteLine($"{Temp.GetName()} has been created with ID: {Temp.GetId()}");
            ECSManager.AddComponent(Temp.GetId(), Components.Mesh);
        }

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern static string GetTestGUID();
    }

}

