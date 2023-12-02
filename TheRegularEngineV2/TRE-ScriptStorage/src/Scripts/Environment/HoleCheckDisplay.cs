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

		private string deactivatedMaterial = "HITW_Center.material";
        private string activatedMaterial = "HoleCheck_Green.material";

		public HoleCheckDisplay()
		{
			triggersComp = new List<HoleCheckTrigger>();
		}

		public void Start()
        {
            GetComponent<MeshRenderer>().Material = deactivatedMaterial;

            // Add triggers that are it's child as triggers
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

			if (ECSManager.IsValidEntity(1325263638198634131))
			{
				AudioSystem.Play(1325263638198634131);
			}

			GetComponent<MeshRenderer>().Material = activatedMaterial;

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