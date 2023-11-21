using System;
using System.Collections.Generic;
using System.Reflection;
using System.Runtime.CompilerServices;
using static System.Runtime.CompilerServices.RuntimeHelpers;
using System.Threading;

namespace TRE
{
    public class ResultLogic : Entity
    {
        public void Start()
        {
            
        }

        public void Update()
        {
            if (InputSystem.GetKeyTrigger(InputKeys.Enter) || InputSystem.GetKeyTrigger(InputKeys.Space))
            {
                // Determine which scene to go next
                if (PersistentSystem.GetValue("PrevScene") == "Tutorial")
                {
                    Scene.TransitionScene("Level_1", 7f);
                }
            }
        }
    }
}