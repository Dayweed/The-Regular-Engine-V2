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
            TextSystem.StartDialogue(fonttest.ID);
            //RendererSystem.Engine_SetSkyboxEnvironment("Skybox5.png", "Skybox5.png", "Skybox5.png", "Skybox5.png", "Skybox5.png", "Skybox5.png");
        }

        public void Update()
        {
            //TextSystem.SetTextMessage(fonttest.ID, "It works");

            if (InputSystem.GetKeyPress(InputKeys.Space))
            {
                TextSystem.ResetDialogue(fonttest.ID);
            }
        }
    }
}
