using System;
using System.Collections.Generic;
using System.Reflection;
using System.Runtime.CompilerServices;
using System.Security.Policy;
using static System.Runtime.CompilerServices.RuntimeHelpers;
using System.Threading;

namespace TRE
{
    public class SceneLogic : Entity
    {
        private string currentSceneName;
        private string nextSceneName;

        public List<HoleCheckDisplay> triggerComplete;

        public List<List<HoleCheckDisplay>> triggerStars;

        public SpriteRenderer courseComplete;
        public static float currentTime;
        public float waitingTime = 0.90f;

        VFX_Emerge StarEmerge;

        public void Start()
        {
            currentTime = 0.0f;
            currentSceneName = Scene.GetSceneName();
            triggerComplete = new List<HoleCheckDisplay>();
            triggerStars = new List<List<HoleCheckDisplay>>();

            Debug.Log("Name " + currentSceneName);

            PersistentSystem.SetValue("PrevScene", currentSceneName);

            PersistentSystem.SetValue("StarsObtained", "0");

            if (currentSceneName == "Tutorial")
            {
                // Add for course complete triggers
                triggerComplete.Add(ECSManager.FindEntityByName("TriggerDisplay_3").GetComponent<HoleCheckDisplay>());
                triggerComplete.Add(ECSManager.FindEntityByName("TriggerDisplay_4").GetComponent<HoleCheckDisplay>());

                // Add for optional stars triggers
                List<HoleCheckDisplay> holeCheckDisplays = new List<HoleCheckDisplay>();
                holeCheckDisplays.Add(ECSManager.FindEntityByName("TriggerDisplay_1").GetComponent<HoleCheckDisplay>());
                holeCheckDisplays.Add(ECSManager.FindEntityByName("TriggerDisplay_2").GetComponent<HoleCheckDisplay>());
                triggerStars.Add(holeCheckDisplays);

                PersistentSystem.SetValue("MaxStarsObtained", "1");

                nextSceneName = "ResultScreen";
            }
            courseComplete = ECSManager.FindEntityByName("CourseComplete").GetComponent<SpriteRenderer>();

            StarEmerge = ECSManager.FindEntityByName("Star_VFX").GetComponent<VFX_Emerge>();
        }

        public void Update()
        {
            // Check if any of the list 
            for (int i = triggerStars.Count - 1; i >= 0; --i)
            {
                List<HoleCheckDisplay> holeCheckDisplays = triggerStars[i];
                bool isCompleted = true;
                foreach (HoleCheckDisplay trigger in holeCheckDisplays)
                {
                    if (!trigger.isCompleted)
                    {
                        isCompleted = false;
                    }
                }
                if (isCompleted)
                {
                    IncrementStars();
                    triggerStars.RemoveAt(i);
                    if (StarEmerge != null)
                    {
                        StarEmerge.Emerge();
                    }
                }
            }

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

            //go to next scene after a while
            if (goToNextScene)
            {
                if (!courseComplete.isVisible)
                {
                    courseComplete.isVisible = true;
                    ECSManager.FindEntityByName("CourseComplete").GetComponent<VFX_SlapOn>().SlapOn();
                }

                if (currentTime >= waitingTime)
                {
                    currentTime = 0.0f;
                    Scene.TransitionScene(nextSceneName, 7f);
                }

                else
                {
                    currentTime += Time.deltaTime;
                }

                //temp only!! not a very smooth transition atm
                if (ECSManager.IsValidEntity(12557813022109059017))
                {
                    AudioSystem.Play(12557813022109059017);
                    AudioSystem.Stop(677401345089954524);
                }
                else
                {
					AudioSystem.Stop(12557813022109059017);
				}
            }
        }

        public void IncrementStars()
        {
            int numStars = 0;

            if (Int32.TryParse(PersistentSystem.GetValue("StarsObtained"), out numStars))
            {
                ++numStars;
                PersistentSystem.SetValue("StarsObtained", numStars.ToString());
                Debug.Log("Stars " + PersistentSystem.GetValue("StarsObtained"));
            }
        }
    }
}