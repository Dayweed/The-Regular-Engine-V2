using System;
using System.Reflection;
using System.Runtime.CompilerServices;

namespace TRE
{
	public class CameraController : Entity
	{
		private Entity Player1;
		private float distance = 35;

		public bool regionA;
		public bool regionB;
		public bool regionC;
		public bool regionD;
		public bool regionE;
		public bool regionF;
		public bool regionG;

		private float distanceA = 35;
		private float distanceB = 50;
		private float distanceC = 25;
		private float distanceD = 50;
		private float distanceE = 50;
		private float distanceF = 65;
		private float distanceG = 40;

		private float lerpTime = 0.01f;

		public void Start()
		{
			Player1 = ECSManager.FindEntityByName("Holey");
		}

		public void Update()
		{
			TransformSystem.GetPosition(Player1.ID, out Vector3 pos);

			if(regionA)
			{
				CameraSystem.TransitionMainCamera(new Vector3(0, 10, 20), new Vector3(30, 180, 0), 0.001f);
				distance = Lerp(distance, distanceA, lerpTime);
			}

			if (regionB)
			{
				CameraSystem.TransitionMainCamera(new Vector3(0, 35, 50), new Vector3(45, 180, 0), 0.001f);
				distance = Lerp(distance, distanceB, lerpTime);
			}

			if (regionC)
			{
				CameraSystem.TransitionMainCamera(new Vector3(0, 40, 50), new Vector3(15, 180, 0), 0.001f);
				distance = Lerp(distance, distanceC, lerpTime);
			}

			if(regionD) 
			{
				CameraSystem.TransitionMainCamera(new Vector3(0, 40, 50), new Vector3(45, 180, 0), 0.001f);
				distance = Lerp(distance, distanceD, lerpTime);
			}

			if (regionE)
			{
				CameraSystem.TransitionMainCamera(new Vector3(0, 40, 50), new Vector3(30, 180, 0), 0.001f);
				distance = Lerp(distance, distanceE, lerpTime);
			}

			if (regionF)
			{
				CameraSystem.TransitionMainCamera(new Vector3(0, 40, 50), new Vector3(45, 180, 0), 0.001f);
				distance = Lerp(distance, distanceF, lerpTime);
			}

			if (regionG)
			{
				CameraSystem.TransitionMainCamera(new Vector3(0, 40, 50), new Vector3(15, 180, 0), 0.001f);
				distance = Lerp(distance, distanceG, lerpTime);
			}

			CameraSystem.SetMainCameraLookAt(pos, distance);
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
