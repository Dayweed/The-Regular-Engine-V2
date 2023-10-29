using System;
using System.Reflection;
using System.Runtime.CompilerServices;

namespace TRE
{
	public class CameraController : Entity
	{
		private Entity Player1;
		public float distance = 50;

		public void Start()
		{
			Player1 = ECSManager.FindEntityByName("Holey");
			Core.Log("My ID is " + this.ID);
			Core.Log("Player1 ID is " + Player1.ID);
		}

		public void Update()
		{
			TransformSystem.GetPosition(Player1.ID, out Vector3 pos);
			CameraSystem.SetMainCameraLookAt(pos, distance);

			//For trigger 1
			if (pos.x >= 40)
			{
				CameraSystem.TransitionMainCamera(new Vector3(100, 7, 20), new Vector3(45, 180, 0), 0.001f);
			}
			else
			{
				CameraSystem.TransitionMainCamera(new Vector3(0, 7, 20), new Vector3(30, 180, 0), 0.001f);
			}
		}
	}
}
