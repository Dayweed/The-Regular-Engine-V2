using System;
using System.Reflection;
using System.Runtime.CompilerServices;

namespace TRE
{
	public class CameraController : Entity
	{
		private Entity Player1;
		private float distance = 50;

		public void Start()
		{
			Player1 = ECSManager.FindEntityByName("Player1");
			Core.Log("My ID is " + Player1.ID);
		}

		public void Update()
		{
			TransformSystem.GetPosition(Player1.ID, out Vector3 pos);
			//CameraSystem.SetMainCameraLookAt(pos, distance);

			//For trigger 1
			if (true)
			{
			}

		}


	}
}
