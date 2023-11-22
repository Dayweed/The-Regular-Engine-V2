using System;
using System.Collections.Generic;
using System.Reflection;
using System.Runtime.CompilerServices;
using static System.Runtime.CompilerServices.RuntimeHelpers;
using System.Threading;

namespace TRE
{
    public class MainMenuSceneTransition : Entity
    {
        private string TutorialSceneName;
        private string ControlsDisplaySceneName;

        public void Start()
        {
            TutorialSceneName = "Tutorial";
            ControlsDisplaySceneName = "ControlsDisplay";
        }

        public void Update()
        {
            if (InputSystem.GetKeyTrigger(InputKeys.Space) && InputSystem.GetKeyTrigger(InputKeys.Enter))
            {
                Scene.ChangeScene(TutorialSceneName);
            }

            //if (InputSystem.GetKeyTrigger(InputKeys.Space))
            //{
            //    Scene.ChangeScene(ControlsDisplaySceneName);
            //}
        }
    }
}
