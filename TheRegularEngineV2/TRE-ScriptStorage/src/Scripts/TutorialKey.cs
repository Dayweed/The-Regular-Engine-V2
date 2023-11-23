using System;
using System.Collections.Generic;
using System.Reflection;
using System.Runtime.CompilerServices;
using static System.Runtime.CompilerServices.RuntimeHelpers;
using System.Threading;
using GlmSharp;
using System.Management.Instrumentation;

namespace TRE
{
    public class TutorialKey : Entity
    {
        private Entity FinalPlatform;

        public void OnCreate()
        {
            FinalPlatform = ECSManager.FindEntityByName("Final_Platform");
        }

        private void OnTriggerEnter(System.UInt64 otherID)
        { 
            Entity other = new Entity(otherID);

            if (EngineGetTag(otherID) == "Red" || EngineGetTag(otherID) == "Blue")
            {
                TransformSystem.SetPosition(FinalPlatform.ID, new vec3(78, 9.5f, -347));
                this.SetActive(false);

                if(ECSManager.IsValidEntity(5918200844399750609))
                {
                    AudioSystem.Play(5918200844399750609);
                }
            }
        }
    }
}
