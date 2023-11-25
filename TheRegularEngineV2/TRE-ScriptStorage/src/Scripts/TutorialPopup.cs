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


        public void Start()
		{
			UIPopup1 = ECSManager.FindEntityByName("PopupUI1");
            UIPopupTutorialStart = ECSManager.FindEntityByName("ControlsPopup");
            HasBeenTriggeredBefore = false;
			IsActivated = false;
		}

		public void Update()
		{
			if (UIPopupTutorialStartExist && InputSystem.GetKeyDown(InputKeys.Space))
			{
				UIPopupTutorialStartExist = false;
				UIPopupTutorialStart.DestroySelf();
            }

			if (IsActivated)
			{
				UIPopup1.GetComponent<SpriteRenderer>().isVisible = true;
			}

			if (IsActivated && InputSystem.GetKeyDown(InputKeys.Space))
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
