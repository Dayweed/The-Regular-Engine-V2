using GlmSharp;

namespace TRE
{
	using PS = PhysicsSystem;
	public class Level_2CameraManager : Entity
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
			regionB = IsHoleyMoleyInsideTrigger(Trigger_B);
			regionC = IsHoleyMoleyInsideTrigger(Trigger_C);
			regionD = IsInsideTrigger(Trigger_D);
			regionE = IsHoleyMoleyInsideTrigger(Trigger_E);
			regionF = IsHoleyMoleyInsideTrigger(Trigger_F);
			regionG = IsHoleyMoleyInsideTrigger(Trigger_G);
			regionH = IsHoleyMoleyInsideTrigger(Trigger_H);
			regionI = IsHoleyMoleyInsideTrigger(Trigger_I);
			regionJ = IsInsideTrigger(Trigger_J);
			regionK = IsHoleyMoleyInsideTrigger(Trigger_K);
			regionL = IsInsideTrigger(Trigger_L);
			regionM = IsInsideTrigger(Trigger_M);

			if (regionA)
			{
				//starting region before entering level
				expectedPosition = new vec3(0, 10, 20);
				expectedRotation = new vec3(30, 180, 0);
				expectedDistance = 35;
				cameraController.lookOnlyBool = false;
				cameraController.expectedYPos = -2f;
				cameraController.offsetX = 0f;
				cameraController.offsetZ = 0f;
				expectedDuration = 0.8f;
				CheckTransition(Trigger_A);
			}

			if (regionB)
			{
				//section one first floor
				expectedPosition = new vec3(-50, 10, 20);
				expectedRotation = new vec3(45, 180, 0);
				expectedDistance = 50;
				cameraController.lookOnlyBool = false;
				cameraController.expectedYPos = 10f;
				cameraController.offsetX = 5f;
				cameraController.offsetZ = 5f;
				expectedDuration = 0.8f;
				CheckTransition(Trigger_B);
			}

			if (regionC)
			{
				//section two second floor
				expectedPosition = new vec3(-50, 10, 20);
				expectedRotation = new vec3(45, 180, 0);
				expectedDistance = 80;
				cameraController.lookOnlyBool = false;
				cameraController.expectedYPos = 30f;
				cameraController.offsetX = 10f;
				cameraController.offsetZ = 10f;
				expectedDuration = 0.8f;
				CheckTransition(Trigger_C);
			}

			if (regionD)
			{
				//first hitw
				expectedPosition = new vec3(0, 10, 20);
				expectedRotation = new vec3(30, 90, 0);
				expectedDistance = 50;
				cameraController.lookOnlyBool = false;
				cameraController.expectedYPos = 30f;
				cameraController.offsetX = 0f;
				cameraController.offsetZ = 0f;
				expectedDuration = 0.8f;
				CheckTransition(Trigger_D);
			}

			if (regionE)
			{
				//section two rooftop - before collectible section
				expectedPosition = new vec3(0, 10, 20);
				expectedRotation = new vec3(55, 180, 0);
				expectedDistance = 60;
				cameraController.lookOnlyBool = false;
				cameraController.expectedYPos = 40f;
				cameraController.offsetX = 0f;
				cameraController.offsetZ = 0f;
				expectedDuration = 0.8f;
				CheckTransition(Trigger_E);
			}

			if (regionF)
			{
				//section two floor - before collectible section
				expectedPosition = new vec3(0, 10, 20);
				expectedRotation = new vec3(45, 180, 0);
				expectedDistance = 50;
				cameraController.lookOnlyBool = false;
				cameraController.expectedYPos = 10f;
				cameraController.offsetX = 0f;
				cameraController.offsetZ = 0f;
				expectedDuration = 0.8f;
				CheckTransition(Trigger_F);
			}

			if (regionG)
			{
				//section two rooftop - collictible platform section
				expectedPosition = new vec3(0, 10, 20);
				expectedRotation = new vec3(55, 180, 0);
				expectedDistance = 80;
				cameraController.lookOnlyBool = false;
				cameraController.expectedYPos = 40f;
				cameraController.offsetX = -20f;
				cameraController.offsetZ = 0f;
				expectedDuration = 0.8f;
				CheckTransition(Trigger_G);
			}

			if (regionH)
			{
				//section two rooftop - collictible platform section
				expectedPosition = new vec3(0, 10, 20);
				expectedRotation = new vec3(45, 180, 0);
				expectedDistance = 40;
				cameraController.lookOnlyBool = false;
				cameraController.expectedYPos = 10f;
				cameraController.offsetX = 0f;
				cameraController.offsetZ = 20f;
				expectedDuration = 0.8f;
				CheckTransition(Trigger_H);
			}

			if (regionI)
			{
				//last pivot platform section
				expectedPosition = new vec3(0, 10, 20);
				expectedRotation = new vec3(60, 90, 0);
				expectedDistance = 60;
				cameraController.lookOnlyBool = false;
				cameraController.expectedYPos = 50f;
				cameraController.offsetX = 0f;
				cameraController.offsetZ = 0f;
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
