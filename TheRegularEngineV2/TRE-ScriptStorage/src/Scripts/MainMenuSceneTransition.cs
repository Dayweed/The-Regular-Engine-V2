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
        public bool spacePressed;
        public bool enterPressed;

        public void Start()
        {
            TutorialSceneName = "Tutorial";
            ControlsDisplaySceneName = "ControlsDisplay";
        }

        public void Update()
        {
            if (InputSystem.GetKeyTrigger(InputKeys.Space))
            {
                spacePressed = true;
            }

            if (InputSystem.GetKeyTrigger(InputKeys.Enter))
            {
                enterPressed = true;
            }

            if (spacePressed && enterPressed)
            {
                Scene.ChangeScene(TutorialSceneName);

                spacePressed = false;
                enterPressed = false;
            }
        }
    }
}
