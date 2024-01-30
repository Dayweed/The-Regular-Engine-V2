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

		private MoleyController MoleyController;
		private HoleyController HoleyController;

		private float distance = 20;

		public vec3 expectedPosition;
		public vec3 expectedRotation;
		public float expectedDistance;

		public bool lookOnlyBool = false; // true = look only(stationary position), false = follow player

		private float lerpSpeed = 0.0005f;

		public void Start()
		{
			Player1 = ECSManager.FindEntityByName("Moley");
			Player2 = ECSManager.FindEntityByName("Holey");

			Player1Transform = Player1.GetComponent<Transform>();
			Player2Transform = Player2.GetComponent<Transform>();

			MoleyController = Player1.GetComponent<MoleyController>();
			HoleyController = Player2.GetComponent<HoleyController>();
		}

		public void Update()
		{
			vec3 pos = Player1Transform.Position + Player2Transform.Position;
			pos /= 2;

			//make it fixed y so if both players jump, the camera doesnt keep bobbing up and down
			pos.y = (Math.Max(Player1Transform.Position.y, Player2Transform.Position.y) + pos.y) / 2;
			distance = MathF.Lerp(distance, expectedDistance, lerpSpeed);

			if (lookOnlyBool)
			{
				CameraSystem.TransitionMainCamera(expectedPosition, expectedRotation, lerpSpeed);
				CameraSystem.SetMainCameraLookAt(pos);
			}
			else
			{
				CameraSystem.TransitionMainCamera(expectedPosition, expectedRotation, lerpSpeed);
				CameraSystem.SetMainCameraFollow(pos, distance);
			}
		}
	}
}
