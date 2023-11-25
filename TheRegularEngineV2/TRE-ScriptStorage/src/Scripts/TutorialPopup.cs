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

		private bool IsActivated = false;
		private bool HasBeenTriggeredBefore = false;

		public void Start()
		{
			UIPopup1 = ECSManager.FindEntityByName("PopupUI1");
			HasBeenTriggeredBefore = false;
			IsActivated = false;
		}

		public void Update()
		{
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
