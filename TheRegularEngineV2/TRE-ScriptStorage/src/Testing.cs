using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace TRE
{
    public class Test : Entity
    {

        void OnCreate()
        {
            Console.WriteLine("I am Calling this From script Storage");
        }
        
        void Update()
        {
            Console.WriteLine("detached");
        }
    }
}
