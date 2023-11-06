using System;
using System.Collections.Generic;
using System.Reflection;
using System.Runtime.CompilerServices;
using static System.Runtime.CompilerServices.RuntimeHelpers;
using System.Threading;

namespace TRE
{
    public class HoleCheckDisplay : Entity
    {
        public bool isCompleted = false;
        public int NoOfTriggers = 2;    // Number of triggers needed

        public List<HoleCheckTrigger> triggersComp;

        public HoleCheckDisplay()
        {
            triggersComp = new List<HoleCheckTrigger>();
        }

        public void Start()
        {
            // Add triggers that are it's child as triggers
            for (int i = 0; i < NoOfTriggers; i++)
            {
                Entity trigger = parenting.GetChild(i);

                if (trigger == null)
                {
                    Debug.LogError("Child (" + i + ") is invalid!");
                    return;
                }
                if (!trigger.CompareTag("Trigger"))
                {
                    Debug.LogError("Child (" + i + ") [" + trigger.name + "] Tag must be [Trigger]!");
                    return;
                }
                if (trigger.GetComponent<HoleCheckTrigger>() == null)
                {
                    Debug.LogError("Child (" + i + ") [" + trigger.name + "] HoleCheckTrigger");
                    return;
                }

                trigger.GetComponent<HoleCheckTrigger>().triggerDisplay = this;
                triggersComp.Add(trigger.GetComponent<HoleCheckTrigger>());
            }
        }

        public void Update()
        {

        }

        public void CheckTrigger()
        {
            // Returns if it is already completed
            if (isCompleted) return;

            // Returns if all didnt hit
            foreach (HoleCheckTrigger trigger in triggersComp)
            {
                if (!trigger.isHit) return;
            }

            // TO DO Display as all triggered
            Debug.Log("I HAVE BEEN TRIGGERED!");
            isCompleted = true;
        }
    }
}