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
        Entity CourseComplete;
        Entity Star_1;
        Entity Star_2;
        Entity Star_3;

        bool SlappedOn;

        public void Start()
        {
            CourseComplete = ECSManager.FindEntityByName("CourseComplete");
            Star_1 = ECSManager.FindEntityByName("Star_1");
            Star_2 = ECSManager.FindEntityByName("Star_2");
            Star_3 = ECSManager.FindEntityByName("Star_3");

            int numStars = 0;
            int maxStars = 0;
            if (Int32.TryParse(PersistentSystem.GetValue("StarsObtained"), out numStars) && Int32.TryParse(PersistentSystem.GetValue("MaxStarsObtained"), out maxStars))
            {
                if (maxStars <= 2 || numStars <= 2)
                {
                    Star_3.SetActive(false);
                }
                if (maxStars <= 1 || numStars <= 1)
                {
                    Star_2.SetActive(false);
                }
                if (maxStars <= 0 || numStars <= 0)
                {
                    Star_1.SetActive(false);
                }
            }
            else
            {
                Star_1.SetActive(false);
                Star_2.SetActive(false);
                Star_3.SetActive(false);
            }

            SlappedOn = false;
        }

        public void Update()
        {
            if (!SlappedOn)
            {
                CourseComplete.GetComponent<VFX_SlapOn>().SlapOn();
                SlappedOn = true;
            }

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