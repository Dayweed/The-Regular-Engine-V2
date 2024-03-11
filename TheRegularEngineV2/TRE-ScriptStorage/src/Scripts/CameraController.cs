using GlmSharp;
using System;

namespace TRE
{
	public class CameraController : Entity
	{
		private Entity Player1;
		private Entity Player2;

		private Entity MidPos;

		// private Transform Player1Transform;
		// private Transform Player2Transform;

		private Transform MidPosTransform;

		// private MoleyController MoleyController;
		// private HoleyController HoleyController;

		private PlayerMidPosition PlayerMidPosition;

		private float distance = 20;

		public vec3 staticPosition;
		// private vec3 finalStaticPosition;

		private vec3 finalPos;

		public vec3 expectedPosition;
		public vec3 expectedRotation;
		public float expectedDistance;
		public float expectedYPos;
		public bool toTransition;
		public float transitionDuration = 0.8f;
		public bool freeCamera = true;

		public bool lookOnlyBool = false; // true = look only(stationary position), false = follow player

		private float lerpSpeed = 2f; //faster is higher

		public void Start()
		{
			Player1 = ECSManager.FindEntityByName("Moley");
			Player2 = ECSManager.FindEntityByName("Holey");

			MidPos = ECSManager.FindEntityByName("MidPos");

			// Player1Transform = Player1.GetComponent<Transform>();
			// Player2Transform = Player2.GetComponent<Transform>();

			MidPosTransform = MidPos.GetComponent<Transform>();

			// MoleyController = Player1.GetComponent<MoleyController>();
			// HoleyController = Player2.GetComponent<HoleyController>();

			PlayerMidPosition = MidPos.GetComponent<PlayerMidPosition>();

			finalPos = MidPosTransform.Position;
			freeCamera = true;

			//HARD CODE FOR NOW
			String levelName = Scene.GetSceneName();
			if (levelName == "MainMenu")
			{
				SilhouetteEffect.Engine_SetSilhouetteActive(false);
			}
			else if(levelName == "Tutorial")
			{
				SilhouetteEffect.Engine_SetSilhouetteActive(true);
			}
			else if(levelName == "Level1")
			{
				SilhouetteEffect.Engine_SetSilhouetteActive(true);
			}
			else if(levelName == "Level2")
			{
				SilhouetteEffect.Engine_SetSilhouetteActive(true);
			}
			else if(levelName == "ResultScreen")
			{
				SilhouetteEffect.Engine_SetSilhouetteActive(false);
			}
		}

		public void Update()
		{
			if (freeCamera)
			{
				//finalPos = MidPosTransform.Position;
				PlayerMidPosition.expectedYPos = expectedYPos;

				//make it fixed y so if both players jump, the camera doesnt keep bobbing up and down
				//finalPos.y = useThisYPos;
				//expectedYPos = MathF.Lerp(pos.y, expectedYPos, lerpSpeed);
				distance = MathF.Lerp(distance, expectedDistance, lerpSpeed * Time.deltaTime);

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

				finalPos.x = MathF.Lerp(finalPos.x, pos.x, lerpSpeed * Time.deltaTime);
				finalPos.y = MathF.Lerp(finalPos.y, pos.y, lerpSpeed * Time.deltaTime);
				finalPos.z = MathF.Lerp(finalPos.z, pos.z, lerpSpeed * Time.deltaTime);
				CameraSystem.SetMainCameraFollow(finalPos, distance);
			}
			else
				CameraSystem.UpdateMainCameraTransform();

			if (toTransition)
			{
				CameraSystem.TransitionMainCamera(expectedPosition, expectedRotation, transitionDuration);
				toTransition = false;
			}

			Player1.GetComponent<MoleyController>().turnDirection = (int)expectedRotation.y;
			Player2.GetComponent<HoleyController>().turnDirection = (int)expectedRotation.y;
		}
	}
}
