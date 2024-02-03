using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace TRE
{
    public class FontTest : Entity
    {
        private Entity fonttest;

        public void Start()
        {
            fonttest = ECSManager.FindEntityByName("fonttest");
        }

        public void Update()
        {
            float time = Time.GetDeltaTime();
            TextSystem.SetTextMessage(fonttest.ID, "It works");
        }
    }
}
