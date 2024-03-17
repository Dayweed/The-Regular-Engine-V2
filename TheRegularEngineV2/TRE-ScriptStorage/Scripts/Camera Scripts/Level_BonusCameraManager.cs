using GlmSharp;

namespace TRE
{
	using PS = PhysicsSystem;
	public class Level_BonusCameraManager : Entity
	{
		private Entity MidPos;
		private Entity Holey;
		private Entity Moley;

		private Entity Trigger_A;
		private Entity Trigger_B;
		private Entity Trigger_C;
		private Entity Trigger_D;
		private Entity Trigger_E;
		private Entity Trigger_F;
		private Entity Trigger_G;
		private Entity Trigger_H;
		private Entity Trigger_I;
		private Entity Trigger_J;
		private Entity Trigger_K;
		private Entity Trigger_L;
		private Entity Trigger_M;

		private Entity Previous_Trigger = null;

		private bool regionA;
		private bool regionB;
		private bool regionC;
		private bool regionD;
		private bool regionE;
		private bool regionF;
		private bool regionG;
		private bool regionH;
		private bool regionI;
		private bool regionJ;
		private bool regionK;
		private bool regionL;
		private bool regionM;

		CameraController cameraController;

		private vec3 expectedPosition;
		private vec3 expectedRotation;
		private float expectedDistance;
		private float expectedDuration;

		public void Start()
		{
			MidPos = ECSManager.FindEntityByName("MidPos");
			Debug.Log("Holey ID is " + MidPos.ID);

			Holey = ECSManager.FindEntityByName("Holey");
			Moley = ECSManager.FindEntityByName("Moley");

			Trigger_A = ECSManager.FindEntityByName("Trigger_A");
			Trigger_B = ECSManager.FindEntityByName("Trigger_B");
			Trigger_C = ECSManager.FindEntityByName("Trigger_C");
			Trigger_D = ECSManager.FindEntityByName("Trigger_D");
			Trigger_E = ECSManager.FindEntityByName("Trigger_E");
			Trigger_F = ECSManager.FindEntityByName("Trigger_F");
			Trigger_G = ECSManager.FindEntityByName("Trigger_G");
			Trigger_H = ECSManager.FindEntityByName("Trigger_H");
			Trigger_I = ECSManager.FindEntityByName("Trigger_I");
			Trigger_J = ECSManager.FindEntityByName("Trigger_J");
			Trigger_K = ECSManager.FindEntityByName("Trigger_K");
			Trigger_L = ECSManager.FindEntityByName("Trigger_L");
			Trigger_M = ECSManager.FindEntityByName("Trigger_M");


			cameraController = ECSManager.FindEntityByName("Main Camera").GetComponent<CameraController>();
			Debug.Log("CameraController ID is " + cameraController.ID);

			expectedDistance = 35;
			expectedPosition = new vec3(0, 10, 20);
			expectedRotation = new vec3(30, 180, 0);
		}

		public void Update()
		{
			cameraController.freeCamera = true;
			regionA = IsInsideTrigger(Trigger_A);
			regionB = IsInsideTrigger(Trigger_B);
			regionC = IsHoleyMoleyInsideTrigger(Trigger_C);
			regionD = IsHoleyMoleyInsideTrigger(Trigger_D);
			regionE = IsInsideTrigger(Trigger_E);
			regionF = IsInsideTrigger(Trigger_F);
			regionG = IsInsideTrigger(Trigger_G);
			regionH = IsInsideTrigger(Trigger_H);
			regionI = IsHoleyMoleyInsideTrigger(Trigger_I);
			regionJ = IsInsideTrigger(Trigger_J);
			regionK = IsHoleyMoleyInsideTrigger(Trigger_K);
			regionL = IsInsideTrigger(Trigger_L);
			regionM = IsInsideTrigger(Trigger_M);

			if (regionA)
			{
				//starting region up until below the 2nd floor
				expectedPosition = new vec3(0, 10, 20);
				expectedRotation = new vec3(45, 180, 0);
				expectedDistance = 55;
				cameraController.lookOnlyBool = false;
				cameraController.expectedYPos = -15f;
				expectedDuration = 0.8f;
				CheckTransition(Trigger_A);
			}

			if (regionB)
			{
				//right under the 2nd floor
				expectedPosition = new vec3(-50, 10, 20);
				expectedRotation = new vec3(45, 180, 0);
				expectedDistance = 55;
				cameraController.lookOnlyBool = false;
				cameraController.expectedYPos = 10f;
				expectedDuration = 0.8f;
				CheckTransition(Trigger_B);
			}

			if (regionC)
			{
				//region with jumppad - first floor
				expectedPosition = new vec3(-50, 10, 20);
				expectedRotation = new vec3(45, 180, 0);
				expectedDistance = 80;
				cameraController.lookOnlyBool = false;
				cameraController.expectedYPos = 30f;
				expectedDuration = 0.8f;
				CheckTransition(Trigger_C);
			}

			if (regionD)
			{
				//region with jumppad - 2nd floor & moving platform 1
				expectedPosition = new vec3(0, 10, 20);
				expectedRotation = new vec3(50, 90, 0);
				expectedDistance = 50;
				cameraController.lookOnlyBool = false;
				cameraController.expectedYPos = 30f;
				expectedDuration = 0.8f;
				CheckTransition(Trigger_D);
			}

			if (regionE)
			{
				//first hitw
				expectedPosition = new vec3(0, 10, 20);
				expectedRotation = new vec3(20, 90, 0);
				expectedDistance = 35;
				cameraController.lookOnlyBool = false;
				cameraController.expectedYPos = 30f;
				expectedDuration = 0.8f;
				CheckTransition(Trigger_E);
			}

			if (regionF)
			{
				//moving platform section
				cameraController.staticPosition = new vec3(130, 50, -204);
				cameraController.lookOnlyBool = true;
				expectedPosition = new vec3(112, 82, -207);
				expectedRotation = new vec3(50, 90, 0);
				expectedDistance = 60;
				cameraController.expectedYPos = 30f;
				expectedDuration = 0.8f;
				CheckTransition(Trigger_F);
			}

			if (regionG)
			{
				//rolling objects section
				expectedPosition = new vec3(121, 82, -207);
				expectedRotation = new vec3(50, 90, 0);
				expectedDistance = 70;
				cameraController.lookOnlyBool = false;
				cameraController.expectedYPos = 30f;
				expectedDuration = 1f;
				CheckTransition(Trigger_G);
			}

			if (regionH)
			{
				//jumppad section after rolling objs - 1st floor
				cameraController.staticPosition = new vec3(320, 45, -330);
				cameraController.lookOnlyBool = true;
				expectedPosition = new vec3(260, 99, -335);
				expectedRotation = new vec3(45, 90, 0);
				expectedDistance = 80;
				cameraController.expectedYPos = 60f;
				expectedDuration = 0.8f;
				CheckTransition(Trigger_H);
			}

			if (regionI)
			{
				//2nd floor before moving hitw
				expectedPosition = new vec3(0, 10, 20);
				expectedRotation = new vec3(30, 0, 0);
				expectedDistance = 50;
				cameraController.lookOnlyBool = false;
				cameraController.expectedYPos = 60f;
				expectedDuration = 0.8f;
				CheckTransition(Trigger_I);
			}

			if (regionJ)
			{
				//on moving hitw and ground before vertical moving platform
				expectedPosition = new vec3(0, 10, 20);
				expectedRotation = new vec3(30, 90, 0);
				expectedDistance = 50;
				cameraController.lookOnlyBool = false;
				cameraController.expectedYPos = 90f;
				expectedDuration = 0.8f;
				CheckTransition(Trigger_J);
			}

			if (regionK)
			{
				//on the vertical platform
				expectedPosition = new vec3(0, 10, 20);
				expectedRotation = new vec3(30, 180, 0);
				expectedDistance = 80;
				cameraController.lookOnlyBool = false;
				cameraController.expectedYPos = 90f;
				expectedDuration = 0.8f;
				CheckTransition(Trigger_K);
			}

			if (regionL)
			{
				//last platforming section
				//for some reason it's offset to the left idk why
				expectedPosition = new vec3(0, 10, 20);
				expectedRotation = new vec3(30, 180, 0);
				expectedDistance = 60;
				cameraController.lookOnlyBool = false;
				cameraController.expectedYPos = 100f;
				expectedDuration = 0.8f;
				CheckTransition(Trigger_L);
			}

			if (regionM)
			{
				//last hitw
				expectedPosition = new vec3(0, 10, 20);
				expectedRotation = new vec3(30, 90, 0);
				expectedDistance = 50;
				cameraController.lookOnlyBool = false;
				cameraController.expectedYPos = 105f;
				expectedDuration = 0.8f;
				CheckTransition(Trigger_M);
			}

			cameraController.expectedPosition = expectedPosition;
			cameraController.expectedRotation = expectedRotation;
			cameraController.expectedDistance = expectedDistance;
			cameraController.transitionDuration = expectedDuration;
		}

		private bool IsInsideTrigger(Entity entity)
		{
			return ECSManager.IsValidEntity(entity.ID) && (PS.IsTriggerEnter(MidPos.ID, entity.ID) || PS.IsTriggerStay(MidPos.ID, entity.ID));
		}

		private bool IsHoleyMoleyInsideTrigger(Entity entity)
		{
			return ECSManager.IsValidEntity(entity.ID) &&
				(PS.IsTriggerEnter(Holey.ID, entity.ID) || PS.IsTriggerStay(Holey.ID, entity.ID)) &&
				(PS.IsTriggerEnter(Moley.ID, entity.ID) || PS.IsTriggerStay(Moley.ID, entity.ID));
		}

		private bool IsHoleyMoleyEnterTrigger(Entity entity)
		{
			return ECSManager.IsValidEntity(entity.ID) &&
				(PS.IsTriggerEnter(Holey.ID, entity.ID)) &&
				(PS.IsTriggerEnter(Moley.ID, entity.ID));
		}

		private void CheckTransition(Entity entity)
		{
			if (Previous_Trigger != entity)
			{
				cameraController.toTransition = true;
				Previous_Trigger = entity;
			}
		}
	}
}
