using System;
using System.Reflection;
using System.Runtime.CompilerServices;


namespace TRE
{

    public enum Components
    {
        Mesh = 0,
        Camera = 1,
        Audio = 2,
    }

    public struct Vector3
    {
        public float x, y, z;
        public Vector3(float x, float y, float z)
        {
            this.x = x; this.y = y; this.z = z;
        }
    }

    public struct Entity
    {
        private string _id;
        private string _name;

        public Entity(string id, string name)
        {
            _id = id;
            _name = name;
        }
        public string GetId()
        {
            return _id;
        }
        public string GetName()
        {
            return _name;
        }

    }

    public class ECSManager
    {
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern static string CreateEntity(string name);
        
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern static void AddComponent(string entityID, Components component);
        
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern static void RemoveComponent(string entityID, Components component);
    }

    public class Demo
    {
        public Demo()
        {
            SpawnObject();
        }


        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern static string SpawnObject();

        
    }

    public class TransformSystem
    {
        public TransformSystem()
        {
        }

        public void transformDemo(string id)
        {
            Vector3 test = new Vector3(0, 0, 0);
            GetRotation(id, out test);

            test.x += 1;

            SetRotation(id, test);
        }

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern static void GetPosition(string id, out Vector3 output);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern static void SetPosition(string id, Vector3 position);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern static void GetRotation(string id, out Vector3 output);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern static void SetRotation(string id, Vector3 rotation);


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

