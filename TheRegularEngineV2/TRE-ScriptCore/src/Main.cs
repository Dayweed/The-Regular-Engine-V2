using System;
using System.Runtime.CompilerServices;


namespace TRE
{

    public enum Components
    {
        Mesh = 0,
        Camera = 1,
        Audio = 2,
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
    public class Main
    {
        public Main()
        {
            Console.WriteLine("Hello World from C#! ABC");
        }

        public void Test()
        {
            string id = ECSManager.CreateEntity("ScriptingTest");
            Entity Temp = new Entity(id, "ScriptingTest");
            Console.WriteLine($"{Temp.GetName()} has been created with ID: {Temp.GetId()}");
            ECSManager.AddComponent(Temp.GetId(), Components.Mesh);
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
}

