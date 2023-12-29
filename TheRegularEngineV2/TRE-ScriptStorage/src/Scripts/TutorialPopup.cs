using System;
using System.Collections.Generic;
using System.Reflection;
using System.Runtime.CompilerServices;
using static System.Runtime.CompilerServices.RuntimeHelpers;
using System.Threading;
using GlmSharp;
using System.Management.Instrumentation;

namespace TRE
{
	public class TutorialPopup : Entity
	{
		private Entity UIPopup1;
		private Entity UIPopupTutorialStart;

		private bool IsActivated = false;
		private bool HasBeenTriggeredBefore = false;

		private bool UIPopupTutorialStartExist = true;

		private Entity RightHUDRef;
        private Entity LeftHUDRef;

        public void Start()
		{
			UIPopup1 = ECSManager.FindEntityByName("PopupUI1");
            UIPopupTutorialStart = ECSManager.FindEntityByName("ControlsPopup");
            HasBeenTriggeredBefore = false;
			IsActivated = false;

            RightHUDRef = ECSManager.FindEntityByName("RightCharacter_HUD");
            LeftHUDRef = ECSManager.FindEntityByName("LeftCharacter_HUD");

			UIPopupTutorialStartExist = true;
		}

		public void Update()
		{
			if (UIPopupTutorialStartExist && InputSystem.GetKeyHold(InputKeys.Space))
			{
				UIPopupTutorialStartExist = false;
				UIPopupTutorialStart.GetComponent<SpriteRenderer>().isVisible = false;

				//Render all the HUD after the popup of control page finishes
				RightHUDRef.GetComponent<SpriteRenderer>().isVisible = true;
                LeftHUDRef.GetComponent<SpriteRenderer>().isVisible = true;
            }

			if (IsActivated)
			{
				UIPopup1.GetComponent<SpriteRenderer>().isVisible = true;
			}

			if (IsActivated && InputSystem.GetKeyHold(InputKeys.Space))
			{
				UIPopup1.GetComponent<SpriteRenderer>().isVisible = false;
				IsActivated = !IsActivated;
			}
		}

		public void OnTriggerStay(System.UInt64 otherID)
		{
			Entity other = new Entity(otherID);

			// Check is interacted with moles players
			if ((EngineGetTag(otherID) == "Red" || EngineGetTag(otherID) == "Blue") && !HasBeenTriggeredBefore)
			{
				IsActivated = true;
				HasBeenTriggeredBefore = true;
			}
		}
	}
}
