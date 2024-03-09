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
	public class TutorialKey : Entity
	{
		private Entity FinalPlatform;
		public Entity UpButton;

		public void OnCreate()
		{
			FinalPlatform = ECSManager.FindEntityByName("Final_Platform");
			UpButton = ECSManager.FindEntityByName("UpButton");
		}

		private void OnTriggerEnter(System.UInt64 otherID)
		{
			Entity other = new Entity(otherID);

			if (EngineGetTag(otherID) == "Red" || EngineGetTag(otherID) == "Blue")
			{


				TransformSystem.SetPosition(FinalPlatform.ID, new vec3(98.867f, 11.572f, -480.259f));
				UpButton.SetActive(false);

				if (ECSManager.IsValidEntity(5918200844399750609))
				{
					AudioSystem.Play(5918200844399750609);
				}
			}
		}

		/*private void OnTriggerExit(System.UInt64 otherID)
		{
			Entity other = new Entity(otherID);

			if (other.CompareTag("Red") || other.CompareTag("Blue"))
			{
				UpButton.SetActive(true);
				TransformSystem.SetPosition(FinalPlatform.ID, new vec3(55.439f, 12.572f, -442.979f));
			}
		}*/
	}
}
