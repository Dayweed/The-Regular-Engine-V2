using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using GlmSharp;

namespace TRE
{
	using PS = PhysicsSystem;
	public class Level_1CameraManager : Entity
	{
		private Entity MidPos;
		private Entity Holey;
		private Entity Moley;

		private Entity Trigger_A;
		private Entity Trigger_BeforeB;
		private Entity Trigger_B;
		private Entity Trigger_C;
		private Entity Trigger_D;
		private Entity Trigger_E;
		private Entity Trigger_F;
		private Entity Trigger_G;
		private Entity Trigger_H;
		private Entity Trigger_AfterH;
		private Entity Trigger_BeforeI;
		private Entity Trigger_I;
		private Entity Trigger_J;

		private bool regionA;
		private bool regionBeforeB;
		private bool regionB;
		private bool regionC;
		private bool regionD;
		private bool regionE;
		private bool regionF;
		private bool regionG;
		private bool regionH;
		private bool regionAfterH;
		private bool regionBeforeI;
		private bool regionI;
		private bool regionJ;

		CameraController cameraController;

		private vec3 expectedPosition;
		private vec3 expectedRotation;
		private float expectedDistance;

		public void Start()
		{
			MidPos = ECSManager.FindEntityByName("MidPos");
			Debug.Log("Holey ID is " + MidPos.ID);

			Holey = ECSManager.FindEntityByName("Holey");
			Moley = ECSManager.FindEntityByName("Moley");

			Trigger_A = ECSManager.FindEntityByName("Trigger_A");
			Trigger_BeforeB = ECSManager.FindEntityByName("Trigger_BeforeB");
			Trigger_B = ECSManager.FindEntityByName("Trigger_B");
			Trigger_C = ECSManager.FindEntityByName("Trigger_C");
			Trigger_D = ECSManager.FindEntityByName("Trigger_D");
			Trigger_E = ECSManager.FindEntityByName("Trigger_E");
			Trigger_F = ECSManager.FindEntityByName("Trigger_F");
			Trigger_G = ECSManager.FindEntityByName("Trigger_G");
			Trigger_H = ECSManager.FindEntityByName("Trigger_H");
			Trigger_AfterH = ECSManager.FindEntityByName("Trigger_AfterH");
			Trigger_BeforeI = ECSManager.FindEntityByName("Trigger_BeforeI");
			Trigger_I = ECSManager.FindEntityByName("Trigger_I");
			Trigger_J = ECSManager.FindEntityByName("Trigger_J");


			cameraController = ECSManager.FindEntityByName("Main Camera").GetComponent<CameraController>();
			Debug.Log("CameraController ID is " + cameraController.ID);

			expectedDistance = 35;
			expectedPosition = new vec3(0, 10, 20);
			expectedRotation = new vec3(30, 180, 0);

		}

		public void Update()
		{
			regionA = IsInsideTrigger(Trigger_A);
			regionBeforeB = IsInsideTrigger(Trigger_BeforeB);
			regionB = IsHoleyMoleyInsideTrigger(Trigger_B);
			regionC = IsHoleyMoleyInsideTrigger(Trigger_C);
			regionD = IsInsideTrigger(Trigger_D);
			regionE = IsInsideTrigger(Trigger_E);
			regionF = IsInsideTrigger(Trigger_F);
			regionG = IsInsideTrigger(Trigger_G);
			regionH = IsHoleyMoleyInsideTrigger(Trigger_H);
			regionAfterH = IsInsideTrigger(Trigger_AfterH);
			regionBeforeI = IsHoleyMoleyInsideTrigger(Trigger_BeforeI);
			regionI = IsInsideTrigger(Trigger_I);
			regionJ = IsInsideTrigger(Trigger_J);

			if (regionA)
			{
				//starting region up until below the 2nd floor
				expectedPosition = new vec3(0, 10, 20);
				expectedRotation = new vec3(30, 180, 0);
				expectedDistance = 35;
				cameraController.lookOnlyBool = false;
				cameraController.expectedYPos = -2f;
			}

			if (regionBeforeB)
			{
				//right under the 2nd floor
				expectedPosition = new vec3(-50, 10, 20);
				expectedRotation = new vec3(45, 90, 0);
				expectedDistance = 50;
				cameraController.lookOnlyBool = false;
				cameraController.expectedYPos = 10f;
			}

			if (regionB)
			{
				//region with jumppad - first floor
				expectedPosition = new vec3(-50, 10, 20);
				expectedRotation = new vec3(45, 90, 0);
				expectedDistance = 50;
				cameraController.lookOnlyBool = false;
				cameraController.expectedYPos = 10f;
			}

			if (regionC)
			{
				//region with jumppad - 2nd floor & moving platform 1
				expectedPosition = new vec3(0, 10, 20);
				expectedRotation = new vec3(50, 90, 0);
				expectedDistance = 50;
				cameraController.lookOnlyBool = false;
				cameraController.expectedYPos = 30f;
			}

			if (regionD)
			{
				//first hitw
				expectedPosition = new vec3(0, 10, 20);
				expectedRotation = new vec3(20, 90, 0);
				expectedDistance = 35;
				cameraController.lookOnlyBool = false;
				cameraController.expectedYPos = 30f;
			}

			if (regionE)
			{
				//moving platform section
				cameraController.staticPosition = new vec3(130, 50, -204);
				cameraController.lookOnlyBool = true;
				expectedPosition = new vec3(112, 82, -207);
				expectedRotation = new vec3(50, 90, 0);
				expectedDistance = 60;
				cameraController.expectedYPos = 30f;
			}

			if (regionF)
			{
				//rolling objects section
				expectedPosition = new vec3(121, 82, -207);
				expectedRotation = new vec3(50, 90, 0);
				expectedDistance = 70;
				cameraController.lookOnlyBool = false;
				cameraController.expectedYPos = 30f;
			}

			if (regionG)
			{
				//jumppad section after rolling objs - 1st floor
				cameraController.staticPosition = new vec3(320, 45, -330);
				cameraController.lookOnlyBool = true;
				expectedPosition = new vec3(260, 99, -335);
				expectedRotation = new vec3(45, 90, 0);
				expectedDistance = 80;
				cameraController.expectedYPos = 60f;
			}

			if (regionH) 
			{
				//2nd floor before moving hitw
				expectedPosition = new vec3(0, 10, 20);
				expectedRotation = new vec3(30, 0, 0);
				expectedDistance = 50;
				cameraController.lookOnlyBool = false;
				cameraController.expectedYPos = 60f;
			}

			if (regionAfterH)
			{
				//on moving hitw and ground before vertical moving platform
				expectedPosition = new vec3(0, 10, 20);
				expectedRotation = new vec3(30, 90, 0);
				expectedDistance = 50;
				cameraController.lookOnlyBool = false;
				cameraController.expectedYPos = 90f;
			}

			if (regionBeforeI)
			{
				//on the vertical platform
				expectedPosition = new vec3(0, 10, 20);
				expectedRotation = new vec3(30, 180, 0);
				expectedDistance = 80;
				cameraController.lookOnlyBool = false;
				cameraController.expectedYPos = 90f;
			}

			if (regionI)
			{
				//last platforming section
				expectedPosition = new vec3(0, 10, 20);
				expectedRotation = new vec3(30, 180, 0);
				expectedDistance = 80;
				cameraController.lookOnlyBool = false;
			}

			if (regionJ)
			{
				//last hitw
				expectedPosition = new vec3(0, 10, 20);
				expectedRotation = new vec3(30, 90, 0);
				expectedDistance = 50;
				cameraController.lookOnlyBool = false;
			}

			cameraController.expectedPosition = expectedPosition;
			cameraController.expectedRotation = expectedRotation;
			cameraController.expectedDistance = expectedDistance;
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
	}
}
