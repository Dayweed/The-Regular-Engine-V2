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

		private float lerpTime = 0.01f;

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
			CameraSystem.TransitionMainCamera(expectedPosition, expectedRotation, 0.001f);
			distance = MathF.Lerp(distance, expectedDistance, lerpTime);

			vec3 pos = Player1Transform.Position + Player2Transform.Position;
			pos /= 2;

			if(MoleyController.isGrounded == false)
			{
				pos.y = Player2Transform.Position.y;
			}
			else if(HoleyController.isGrounded == false)
			{
				pos.y = Player1Transform.Position.y;
			}

			CameraSystem.SetMainCameraLookAt(pos, distance);
		}
	}
}
