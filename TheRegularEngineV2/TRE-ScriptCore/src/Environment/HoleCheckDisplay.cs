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

        public void OnCreate()
        {
            // Add triggers that are it's child as triggers
            for (int i = 0; i < NoOfTriggers; i++)
            {
                Entity trigger = parenting.GetChild(i);

                if (trigger == null || !trigger.CompareTag("Trigger") || !trigger.HasComponent<HoleCheckTrigger>())
                {
                    Debug.LogError("Child (" + i + ") is invalid and not a trigger! Tag must be [Trigger] and HoleCheckTrigger");
                    return;
                }

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