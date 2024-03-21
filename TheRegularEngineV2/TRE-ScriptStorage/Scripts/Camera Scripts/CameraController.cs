using GlmSharp;

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
		public float offsetX;
		public float offsetZ;
		public float forcedX;
		public float forcedZ;
		public bool toTransition;
		public float transitionDuration = 0.8f;
		public bool freeCamera = true;

		public bool lookOnlyBool = false; // true = look only(stationary position), false = follow player

		public bool forceXaxis = false;
		public bool forceZaxis = false;

		private const float lerpSpeed = 2f; //faster is higher

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
			freeCamera = false;
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
					pos = new vec3(MidPosTransform.Position.x + offsetX, expectedYPos, MidPosTransform.Position.z + offsetZ);
					if (forceZaxis)
					{
						pos.z = forcedZ;
					}
					else
					{
						pos.z = MidPosTransform.Position.z + offsetZ;
					}
					if (forceXaxis)
					{
						pos.x = forcedX;
					}
					else
					{
						pos.x = MidPosTransform.Position.x + offsetX;
					}
				}

				

				finalPos.x = MathF.Lerp(finalPos.x, pos.x, lerpSpeed * Time.deltaTime);
				finalPos.y = MathF.Lerp(finalPos.y, pos.y, lerpSpeed * Time.deltaTime);
				finalPos.z = MathF.Lerp(finalPos.z, pos.z, lerpSpeed * Time.deltaTime);
				CameraSystem.SetMainCameraFollow(finalPos, distance);
			}
			else
			{
				CameraSystem.UpdateMainCameraTransform();
			}

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
