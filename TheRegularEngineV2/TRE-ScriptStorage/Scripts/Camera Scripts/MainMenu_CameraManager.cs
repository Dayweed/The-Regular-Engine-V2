using GlmSharp;

namespace TRE
{
	using PS = PhysicsSystem;
	public class MainMenu_CameraManager : Entity
	{
		private Entity Holey;

		private Entity Trigger_A;
		private Entity Trigger_B;

		private bool regionA;
		private bool regionB;

		CameraController cameraController;

		private vec3 expectedPosition;
		private vec3 expectedRotation;
		private float expectedDistance;
		private float expectedDuration;

		public void Start()
		{
			Holey = ECSManager.FindEntityByName("Holey");
			Debug.Log("Holey ID is " + Holey.ID);

			Trigger_A = ECSManager.FindEntityByName("Trigger_A");
			Debug.Log("Trigger_A ID is " + Trigger_A.ID);

			Trigger_B = ECSManager.FindEntityByName("Trigger_B");
			Debug.Log("Trigger_B ID is " + Trigger_B.ID);

			cameraController = ECSManager.FindEntityByName("Main Camera").GetComponent<CameraController>();
			Debug.Log("CameraController ID is " + cameraController.ID);

			expectedDistance = 0;
			expectedPosition = new vec3(-9, 34, 50);
			expectedRotation = new vec3(40, 180, 0);

			cameraController.freeCamera = true;
		}

		public void Update()
		{
			cameraController.lookOnlyBool = true;
			regionA = IsInsideTrigger(Trigger_A);
			regionB = IsInsideTrigger(Trigger_B);

			if (regionA)
			{
				cameraController.staticPosition = new vec3(-9, 34, 50);
				cameraController.lookOnlyBool = true;
				expectedPosition = new vec3(-9, 34, 50);
				expectedRotation = new vec3(40, 180, 0);
				expectedDistance = 0;
				expectedDuration = 1.5f;
			}

			if (regionB)
			{
				cameraController.staticPosition = new vec3(113, 57.5f, -140);
				cameraController.lookOnlyBool = true;
				expectedPosition = new vec3(113, 57.5f, -140);
				expectedRotation = new vec3(50, 139, 0);
				expectedDistance = 0;
				expectedDuration = 1.5f;
			}

			cameraController.expectedPosition = expectedPosition;
			cameraController.expectedRotation = expectedRotation;
			cameraController.expectedDistance = expectedDistance;
			cameraController.transitionDuration = expectedDuration;
			cameraController.toTransition = PS.IsTriggerEnter(Holey.ID, Trigger_A.ID) || PS.IsTriggerEnter(Holey.ID, Trigger_B.ID);
		}

		private bool IsInsideTrigger(Entity entity)
		{
			//make a gameobject that will always be in the middle of both characters and then find that obj instead
			return ECSManager.IsValidEntity(entity.ID) && PS.IsTriggerEnter(Holey.ID, entity.ID);

		}
	}
}
