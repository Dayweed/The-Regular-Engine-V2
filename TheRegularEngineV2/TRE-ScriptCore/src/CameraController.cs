using System;
using System.Reflection;
using System.Runtime.CompilerServices;

namespace TRE
{
	public class CameraController : Entity
	{
		private Entity Player1;
		private Entity Player2;

		private MoleController moleController;
		private float distance = 35;

		private float distanceA = 35;
		private float distanceB = 50;
		private float distanceC = 25;
		private float distanceD = 50;
		private float distanceE = 50;
		private float distanceF = 65;
		private float distanceG = 50;
		private float distanceH = 20;

		private float lerpTime = 0.01f;

		public void Start()
		{
			Player1 = ECSManager.FindEntityByName("Holey");
			moleController = Player1.GetComponent<MoleController>();
			Player2 = ECSManager.FindEntityByName("Moley");
		}

		public void Update()
		{
			TransformSystem.GetPosition(Player1.ID, out Vector3 holeyPos);
			TransformSystem.GetPosition(Player2.ID, out Vector3 moleyPos);

			if(moleController.regionA)
			{
				CameraSystem.TransitionMainCamera(new Vector3(0, 10, 20), new Vector3(30, 180, 0), 0.001f);
				distance = MathF.Lerp(distance, distanceA, lerpTime);
			}

			if (moleController.regionB)
			{
				CameraSystem.TransitionMainCamera(new Vector3(0, 35, 50), new Vector3(45, 180, 0), 0.001f);
				distance = MathF.Lerp(distance, distanceB, lerpTime);
			}

			if (moleController.regionC)
			{
				CameraSystem.TransitionMainCamera(new Vector3(0, 40, 50), new Vector3(15, 180, 0), 0.001f);
				distance = MathF.Lerp(distance, distanceC, lerpTime);
			}

			if(moleController.regionD) 
			{
				CameraSystem.TransitionMainCamera(new Vector3(0, 40, 50), new Vector3(45, 180, 0), 0.001f);
				distance = MathF.Lerp(distance, distanceD, lerpTime);
			}

			if (moleController.regionE)
			{
				CameraSystem.TransitionMainCamera(new Vector3(0, 40, 50), new Vector3(30, 180, 0), 0.001f);
				distance = MathF.Lerp(distance, distanceE, lerpTime);
			}

			if (moleController.regionF)
			{
				CameraSystem.TransitionMainCamera(new Vector3(0, 40, 50), new Vector3(45, 180, 0), 0.001f);
				distance = MathF.Lerp(distance, distanceF, lerpTime);
			}

			if (moleController.regionG)
			{
				CameraSystem.TransitionMainCamera(new Vector3(50, 40, 50), new Vector3(35, 200, 0), 0.001f);
				distance = MathF.Lerp(distance, distanceG, lerpTime);
			}

			if (moleController.regionH) 
			{
				CameraSystem.TransitionMainCamera(new Vector3(50, 40, 50), new Vector3(20, 180, 0), 0.001f);
				distance = MathF.Lerp(distance, distanceH, lerpTime);

			}

			Vector3 pos = moleyPos + holeyPos;
			pos /= 2;
			CameraSystem.SetMainCameraLookAt(pos, distance);
		}
	}
}
