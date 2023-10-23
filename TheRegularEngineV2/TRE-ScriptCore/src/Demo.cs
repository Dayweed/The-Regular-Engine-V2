using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace TRE
{
    public class Demo
    {
        public Entity Temp;

        public Demo()
        {

            Temp = new Entity("Temp");
            Temp.id = ECSManager.CreateEntity("Temp");

            Vector3 Position = new Vector3(0,0,0);
            TransformSystem.SetPosition(Temp.id, Position);
        }

        public void Update()
        {
            Vector3 temp;
            TransformSystem.GetPosition(Temp.id, out temp);
            temp.x += 0.1f;
            TransformSystem.SetPosition(Temp.id, temp);

        }

    }
}
