using System;
using System.Collections.Generic;
using System.Reflection;
using System.Runtime.CompilerServices;
using static System.Runtime.CompilerServices.RuntimeHelpers;
using System.Threading;
using GlmSharp;

namespace TRE
{
	public class PlatformLogic : Entity
	{
		private Entity TriggerPlatformCollider;
		private Entity Holey;
		private Entity Moley;

		public void Start()
		{
			TriggerPlatformCollider = parenting.GetChildFromName("TriggerPlatform");
			Holey = ECSManager.FindEntityByName("Holey");
			Moley = ECSManager.FindEntityByName("Moley");
		}

		public void Update()
		{
			// Make Moley or Holey follow the platform if within trigger box
			if (HasComponent<DirectPathfinding>() && ECSManager.IsValidEntity(TriggerPlatformCollider.ID))
			{
				// Check if is in trigger
				if (PhysicsSystem.IsTriggerStay(TriggerPlatformCollider.ID, Holey.ID))
				{
					Holey.GetComponent<Transform>().Position += transform.Position - GetComponent<DirectPathfinding>().oldPosition;
				}
				if (PhysicsSystem.IsTriggerStay(TriggerPlatformCollider.ID, Moley.ID))
				{
					Moley.GetComponent<Transform>().Position += transform.Position - GetComponent<DirectPathfinding>().oldPosition;
				}
			}
		}

		public void OnCollisionStay(System.UInt64 otherID)
		{
			Entity other = new Entity(otherID);
            if (other.CompareTag("Red") || other.CompareTag("Blue") || other.CompareTag("Strawberry") || other.CompareTag("Blueberry"))
            {
                other.GetComponent<Transform>().Position += (transform.Position - GetComponent<DirectPathfinding>().oldPosition) * Time.deltaTime;
            }
        }
	}
}