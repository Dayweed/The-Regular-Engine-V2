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

		private string deactivatedMaterial = "HITW_Display.material";
		private string activatedMaterial = "HITW_Display_Correct.material";

		private ulong HoleCheckSFX;

		public HoleCheckDisplay()
		{
			triggersComp = new List<HoleCheckTrigger>();
		}

		public void Start()
		{
			parenting.GetChild(2).GetComponent<MeshRenderer>().Material = deactivatedMaterial;

			// Add triggers that are it's child as triggers
			NoOfTriggers = parenting.GetTotalChildren();

            for (int i = 0; i < NoOfTriggers; i++)
			{
				Entity trigger = parenting.GetChild(i);

				if (trigger == null)
				{
					Debug.LogError("Child (" + i + ") is invalid!");
					return;
				}
				//if (!trigger.CompareTag("Trigger"))
				//{
				//    Debug.LogError("Child (" + i + ") [" + trigger.name + "] Tag must be [Trigger]!");
				//    return;
				//}
				if (trigger.GetComponent<HoleCheckTrigger>() == null)
				{
					Debug.LogError("Child (" + i + ") [" + trigger.name + "] HoleCheckTrigger");
					return;
				}

				trigger.GetComponent<HoleCheckTrigger>().triggerDisplay = this;
				triggersComp.Add(trigger.GetComponent<HoleCheckTrigger>());
			}

			HoleCheckSFX = ECSManager.FindIDFromName("SFX_HoleCheck");
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

			if (ECSManager.IsValidEntity(HoleCheckSFX))
			{
				AudioSystem.Play(HoleCheckSFX);
			}

			parenting.GetChild(2).GetComponent<MeshRenderer>().Material = activatedMaterial;

			/* Material instances GUID
				> 18cf136263b2c948
				> b67077a64edeaafc
				> 74617a06db4fb375
				> e9766d0227b43187
				> bc2355f39496ffbf
			*/
		}
	}
}