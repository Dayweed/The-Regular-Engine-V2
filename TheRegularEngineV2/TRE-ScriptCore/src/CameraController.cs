using System;
using System.Reflection;
using System.Runtime.CompilerServices;

namespace TRE
{
	public class CameraController : Entity
	{
		private Entity Player1;
		private Entity Player2;

		private float distance;

		public Vector3 expectedPosition;
		public Vector3 expectedRotation;
		public float expectedDistance;

		private float lerpTime = 0.01f;

		public void Start()
		{
			Player1 = ECSManager.FindEntityByName("Holey");
			Player2 = ECSManager.FindEntityByName("Moley");
		}

		public void Update()
		{
			TransformSystem.GetPosition(Player1.ID, out Vector3 holeyPos);
			TransformSystem.GetPosition(Player2.ID, out Vector3 moleyPos);

			CameraSystem.TransitionMainCamera(expectedPosition, expectedRotation, 0.001f);
			distance = MathF.Lerp(distance, expectedDistance, lerpTime);

			Vector3 pos = moleyPos + holeyPos;
			pos /= 2;
			CameraSystem.SetMainCameraLookAt(pos, distance);
		}
	}
}
