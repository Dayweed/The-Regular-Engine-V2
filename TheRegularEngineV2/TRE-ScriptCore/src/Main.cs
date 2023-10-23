using System;
using System.Reflection;
using System.Runtime.CompilerServices;


namespace TRE
{

    

    public class Main
    {
        
        public Main()
        {
            Console.WriteLine("Hello World from C#!");
        }

        public void Update()
        {
        }


        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern static string GetTestGUID();
    }

}

