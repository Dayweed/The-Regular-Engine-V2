using System;
using System.Reflection;
using System.Runtime.CompilerServices;

namespace TRE
{
	public class CameraController : Entity
	{
		private Entity Player1;
		private float distance = 35;

		public bool regionStart;
		public bool region1;
		public bool region2;

		private float distanceStart = 35;
		private float distance1 = 90;
		private float distance2 = 25;

		private float lerpTime = 0.01f;

		public void Start()
		{
			Player1 = ECSManager.FindEntityByName("Holey");
		}

		public void Update()
		{
			TransformSystem.GetPosition(Player1.ID, out Vector3 pos);

			if(regionStart)
			{
				CameraSystem.TransitionMainCamera(new Vector3(0, 10, 20), new Vector3(30, 180, 0), 0.001f);
				distance = Lerp(distance, distanceStart, lerpTime);
			}

			if (region1)
			{
				CameraSystem.TransitionMainCamera(new Vector3(0, 40, 50), new Vector3(45, 180, 0), 0.001f);
				distance = Lerp(distance, distance1, lerpTime);
			}

			if (region2)
			{
				CameraSystem.TransitionMainCamera(new Vector3(0, 40, 50), new Vector3(15, 180, 0), 0.001f);
				distance = Lerp(distance, distance2, lerpTime);
			}

			CameraSystem.SetMainCameraLookAt(pos, distance);

			Debug.Log("Region Start " + regionStart);
			Debug.Log("Region 1 " + region1);
			Debug.Log("Region 2 " + region2);
		}
		private float Lerp(float start, float end, float t)
		{
			if(t > 1)
			{
				t = 1;
			}
			if(t < 0)
			{
				t = 0;
			}
			return start * (1 - t) + end * t;
		}
	}
}
