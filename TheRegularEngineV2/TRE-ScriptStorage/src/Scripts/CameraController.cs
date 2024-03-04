using System;
using System.Reflection;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using GlmSharp;

namespace TRE
{
	public class CameraController : Entity
	{
		/*private Entity Player1;
		private Entity Player2;

		private Transform Player1Transform;
		private Transform Player2Transform;

		private MoleyController MoleyController;
		private HoleyController HoleyController;

		private float distance = 20;

		public vec3 staticPosition;
		private vec3 finalStaticPosition;

		private vec3 finalPos;

		public vec3 expectedPosition;
		public vec3 expectedRotation;
		public float expectedDistance;

		public bool lookOnlyBool = false; // true = look only(stationary position), false = follow player

		private float lerpSpeed = 0.001f;

		public void Start()
		{
			Player1 = ECSManager.FindEntityByName("Moley");
			Player2 = ECSManager.FindEntityByName("Holey");

			Player1Transform = Player1.GetComponent<Transform>();
			Player2Transform = Player2.GetComponent<Transform>();

			MoleyController = Player1.GetComponent<MoleyController>();
			HoleyController = Player2.GetComponent<HoleyController>();
			finalPos = Player1Transform.Position + Player2Transform.Position;
			finalPos /= 2;
		}

		public void Update()
		{
			vec3 pos = Player1Transform.Position + Player2Transform.Position;
			pos /= 2;

			//make it fixed y so if both players jump, the camera doesnt keep bobbing up and down
			pos.y = (Math.Max(Player1Transform.Position.y, Player2Transform.Position.y) + pos.y) / 2f;
			distance = MathF.Lerp(distance, expectedDistance, lerpSpeed);

			if (lookOnlyBool)
			{
				CameraSystem.TransitionMainCamera(expectedPosition, expectedRotation, 0.8f);
				//CameraSystem.SetMainCameraLookAt(pos);
				finalStaticPosition.x = MathF.Lerp(finalStaticPosition.x, staticPosition.x, lerpSpeed);
				finalStaticPosition.y = MathF.Lerp(finalStaticPosition.y, staticPosition.y, lerpSpeed);
				finalStaticPosition.z = MathF.Lerp(finalStaticPosition.z, staticPosition.z, lerpSpeed);
				//CameraSystem.SetMainCameraFollow(finalStaticPosition, distance);
				CameraSystem.SetMainCameraFollow(finalStaticPosition, distance);
				finalPos = finalStaticPosition;
			}
			else
			{
				CameraSystem.TransitionMainCamera(expectedPosition, expectedRotation, 0.8f);
				finalPos.x = MathF.Lerp(finalPos.x, pos.x, lerpSpeed * 5f);
				finalPos.y = MathF.Lerp(finalPos.y, pos.y, lerpSpeed * 5f);
				finalPos.z = MathF.Lerp(finalPos.z, pos.z, lerpSpeed * 5f);
				CameraSystem.SetMainCameraFollow(finalPos, distance);
				finalStaticPosition = finalPos;
			}

			Player1.GetComponent<MoleyController>().turnDirection = (int)expectedRotation.y;
			Player2.GetComponent<HoleyController>().turnDirection = (int)expectedRotation.y;
		}*/

		private Entity Player1;
		private Entity Player2;

		private Entity MidPos;

		private Transform Player1Transform;
		private Transform Player2Transform;

		private Transform MidPosTransform;

		private MoleyController MoleyController;
		private HoleyController HoleyController;

		private PlayerMidPosition PlayerMidPosition;

		private float distance = 20;

		public vec3 staticPosition;
		private vec3 finalStaticPosition;

		private vec3 finalPos;

		public vec3 expectedPosition;
		public vec3 expectedRotation;
		public float expectedDistance;
		public float expectedYPos;
		public bool toTransition;

		public bool lookOnlyBool = false; // true = look only(stationary position), false = follow player

		private float lerpSpeed = 0.01f;

		public void Start()
		{
			Player1 = ECSManager.FindEntityByName("Moley");
			Player2 = ECSManager.FindEntityByName("Holey");

			MidPos = ECSManager.FindEntityByName("MidPos");

			Player1Transform = Player1.GetComponent<Transform>();
			Player2Transform = Player2.GetComponent<Transform>();

			MidPosTransform = MidPos.GetComponent<Transform>();

			MoleyController = Player1.GetComponent<MoleyController>();
			HoleyController = Player2.GetComponent<HoleyController>();

			PlayerMidPosition = MidPos.GetComponent<PlayerMidPosition>();

			finalPos = MidPosTransform.Position;
		}

		public void Update()
		{
			//finalPos = MidPosTransform.Position;
			PlayerMidPosition.expectedYPos = expectedYPos;

			//make it fixed y so if both players jump, the camera doesnt keep bobbing up and down
			//finalPos.y = useThisYPos;
			//expectedYPos = MathF.Lerp(pos.y, expectedYPos, lerpSpeed);
			distance = MathF.Lerp(distance, expectedDistance, lerpSpeed);

			//if (lookOnlyBool)
			//{
			//	//CameraSystem.TransitionMainCamera(expectedPosition, expectedRotation, 0.8f);
			//	finalStaticPosition.x = MathF.Lerp(finalStaticPosition.x, staticPosition.x, lerpSpeed);
			//	finalStaticPosition.y = MathF.Lerp(finalStaticPosition.y, staticPosition.y, lerpSpeed);
			//	finalStaticPosition.z = MathF.Lerp(finalStaticPosition.z, staticPosition.z, lerpSpeed);
			//	CameraSystem.SetMainCameraFollow(finalStaticPosition, distance);
			//	finalPos = finalStaticPosition;
			//}
			//else
			//{
			//	//CameraSystem.TransitionMainCamera(expectedPosition, expectedRotation, 0.8f);

			//}
			vec3 pos;
			if (lookOnlyBool)
			{
				pos = staticPosition;
			}
			else
			{
				pos = MidPosTransform.Position;
				pos.y = expectedYPos;
			}

			finalPos.x = MathF.Lerp(finalPos.x, pos.x, lerpSpeed);
			finalPos.y = MathF.Lerp(finalPos.y, pos.y, lerpSpeed);
			finalPos.z = MathF.Lerp(finalPos.z, pos.z, lerpSpeed);
			CameraSystem.SetMainCameraFollow(finalPos, distance);
			//finalStaticPosition = finalPos;

			if (toTransition)
			{
				CameraSystem.TransitionMainCamera(expectedPosition, expectedRotation, 0.8f);
				//toTransition = false;
			}

			Player1.GetComponent<MoleyController>().turnDirection = (int)expectedRotation.y;
			Player2.GetComponent<HoleyController>().turnDirection = (int)expectedRotation.y;
		}
	}
}
