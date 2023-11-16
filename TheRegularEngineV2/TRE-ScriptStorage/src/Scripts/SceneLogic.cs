using System;
using System.Collections.Generic;
using System.Reflection;
using System.Runtime.CompilerServices;
using static System.Runtime.CompilerServices.RuntimeHelpers;
using System.Threading;

namespace TRE
{
    public class SceneLogic : Entity
    {
        private string currentSceneName;
        private string nextSceneName;

        public List<HoleCheckDisplay> triggerComplete;

        public void Start()
        {
            currentSceneName = Scene.GetSceneName();
            triggerComplete = new List<HoleCheckDisplay>();
            Debug.Log("Name " + currentSceneName);
            if (currentSceneName == "Tutorial")
            {
                triggerComplete.Add(ECSManager.FindEntityByName("TriggerDisplay_3").GetComponent<HoleCheckDisplay>());
                triggerComplete.Add(ECSManager.FindEntityByName("TriggerDisplay_4").GetComponent<HoleCheckDisplay>());
                nextSceneName = "Level_1";
            }
        }

        public void Update()
        {
            // Go to next scene if list of triggers are completed
            bool goToNextScene = triggerComplete.Count == 0 ? false : true;
            foreach (HoleCheckDisplay trigger in triggerComplete)
            {
                if (!trigger.isCompleted)
                {
                    goToNextScene = false;
                    return;
                }
            }
            if (goToNextScene)
            {
                Scene.TransitionScene(nextSceneName, 4f);

                //temp only!! not a very smooth transition atm
                if (ECSManager.IsValidEntity(12557813022109059017))
                {
                    AudioSystem.Play(12557813022109059017);
                }
                else
                {
					AudioSystem.Stop(12557813022109059017);
				}
            }
        }
    }
}