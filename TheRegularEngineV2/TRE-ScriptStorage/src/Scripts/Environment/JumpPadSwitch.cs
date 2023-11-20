using System;
using System.Collections.Generic;
using System.Reflection;
using System.Runtime.CompilerServices;
using static System.Runtime.CompilerServices.RuntimeHelpers;
using System.Threading;

namespace TRE
{
    public class JumpPadSwitch : Entity
    {
        public Entity JumpPad;

        public JumpPadSwitch()
        {
            
        }

        public void OnCreate()
        {
            JumpPad = ECSManager.FindEntityByName("JumpPad");
            if (JumpPad == null)
            {
                Debug.LogError("JumpPadSwitch JumpPad is null!");
                return;
            }
            if (JumpPad.GetComponent<JumpPad>() == null)
            {
                Debug.LogError("JumpPadSwitch JumpPad Component is null!");
                return;
            }
        }

        public void Update()
        {

        }

        public void OnTriggerStay(System.UInt64 otherID)
        {
            Entity other = new Entity(otherID);
            // Check is activated jumppad
            if (other.CompareTag("Red") || other.CompareTag("Blue"))
            {
                JumpPad.GetComponent<JumpPad>().ActivatePad(true);
            }
        }

        public void OnTriggerExit(System.UInt64 otherID)
        {
            Entity other = new Entity(otherID);
            // Check is activated jumppad
            if (other.CompareTag("Red") || other.CompareTag("Blue"))
            {
                JumpPad.GetComponent<JumpPad>().ActivatePad(false);
            }
        }
    }
}