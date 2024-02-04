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
	public class LevelOneObject : Entity
	{
		private Entity LevelObject;

		public bool pickedUp = false;

		public void OnCreate()
		{
			LevelObject = ECSManager.FindEntityByName("LevelObject_PickMe");
		}

		private void OnTriggerEnter(System.UInt64 otherID)
		{
			Entity other = new Entity(otherID);

			if (other.CompareTag("Red") || other.CompareTag("Blue"))
			{
				GetComponent<MeshRenderer>().Visible = false;
				pickedUp = true;
			}
		}
	}
}
