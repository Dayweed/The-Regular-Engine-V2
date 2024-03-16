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

			expectedDistance = 35;
			expectedPosition = new vec3(0, 10, 20);
			expectedRotation = new vec3(30, 180, 0);

			cameraController.freeCamera = true;

		}

		public void Update()
		{
			regionA = IsInsideTrigger(Trigger_A);
			regionB = IsInsideTrigger(Trigger_B);

			if (regionA)
			{
				expectedPosition = new vec3(0, 10, 20);
				expectedRotation = new vec3(30, 180, 0);
				expectedDistance = 35;
				expectedDuration = 0.8f;
			}

			if (regionB)
			{
				expectedPosition = new vec3(0, 10, 0);
				expectedRotation = new vec3(45, 180, 0);
				expectedDistance = 60;
				expectedDuration = 0.8f;
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
			return ECSManager.IsValidEntity(entity.ID) && (PS.IsTriggerEnter(Holey.ID, entity.ID) || PS.IsTriggerStay(Holey.ID, entity.ID));
		}
	}
}
