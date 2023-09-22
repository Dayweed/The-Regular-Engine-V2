using System;
using System.Runtime.CompilerServices;


namespace TRE
{

    public class Main
    {
        public Main()
        {
            Console.WriteLine("Hello World!");
            ECSManager.CreateEntity("ScriptingTest");
        }
    }

    public class ECSManager
    {
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern static void CreateEntity(string name);
    }
}

