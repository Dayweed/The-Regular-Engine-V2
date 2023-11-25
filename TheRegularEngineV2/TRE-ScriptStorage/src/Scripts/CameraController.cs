using System;
using System.Reflection;
using System.Runtime.CompilerServices;

using GlmSharp;

namespace TRE
{
	public class CameraController : Entity
	{
		private Entity Player1;
		private Entity Player2;

		private Transform Player1Transform;
		private Transform Player2Transform;

		private float distance = 20;

		public vec3 expectedPosition;
		public vec3 expectedRotation;
		public float expectedDistance;

		private float lerpTime = 0.01f;

		public void Start()
		{
			Player1 = ECSManager.FindEntityByName("Holey");
			Player2 = ECSManager.FindEntityByName("Moley");

			Player1Transform = Player1.GetComponent<Transform>();
			Player2Transform = Player2.GetComponent<Transform>();
		}

		public void Update()
		{
			CameraSystem.TransitionMainCamera(expectedPosition, expectedRotation, 0.001f);
			distance = MathF.Lerp(distance, expectedDistance, lerpTime);

			vec3 pos = Player1Transform.Position + Player2Transform.Position;
			pos /= 2;
			CameraSystem.SetMainCameraLookAt(pos, distance);
		}
	}
}
