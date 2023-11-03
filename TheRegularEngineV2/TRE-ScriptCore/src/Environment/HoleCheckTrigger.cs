using System;
using System.Collections.Generic;
using System.Reflection;
using System.Runtime.CompilerServices;
using static System.Runtime.CompilerServices.RuntimeHelpers;
using System.Threading;

namespace TRE
{
    public class HoleCheckTrigger : Entity
    {
        public HoleCheckDisplay triggerDisplay;   // Updated by parent if it is HoleCheckDisplay
        public bool isHit = false;

        public HoleCheckTrigger()
        {

        }

        public void OnCreate()
        {
            SetTag("Trigger");
        }

        private void OnCollisionStay(System.UInt64 otherID)
        {
            Entity other = new Entity(otherID);

            // Check is interacted with moles players
            if (EngineGetTag(otherID) == "Red" || EngineGetTag(otherID) == "Blue")
            {
                isHit = true;
                triggerDisplay.CheckTrigger();
            }
            if (PhysicsSystem.IsCollisionExit(this.ID, otherID))
            {
                if (EngineGetTag(otherID) == "Red" || EngineGetTag(otherID) == "Blue")
                {
                    isHit = false;
                    triggerDisplay.CheckTrigger();
                }
            }
        }
    }
}