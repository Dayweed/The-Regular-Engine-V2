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
		private Entity Holey;

		private Entity Trigger_A;
		private Entity Trigger_B;
		private Entity Trigger_C;
		private Entity Trigger_D;
		private Entity Trigger_E;
		private Entity Trigger_F;
		private Entity Trigger_G;
		private Entity Trigger_H;

		private bool regionA;
		private bool regionB;
		private bool regionC;
		private bool regionD;
		private bool regionE;
		private bool regionF;
		private bool regionG;
		private bool regionH;

		CameraController cameraController;

		private vec3 expectedPosition;
		private vec3 expectedRotation;
		private float expectedDistance;

		public void Start()
		{
			Holey = ECSManager.FindEntityByName("Holey");
			Debug.Log("Holey ID is " + Holey.ID);

			Trigger_A = ECSManager.FindEntityByName("Trigger_A");
			Debug.Log("Trigger_A ID is " + Trigger_A.ID);

			Trigger_B = ECSManager.FindEntityByName("Trigger_B");
			Debug.Log("Trigger_B ID is " + Trigger_B.ID);

			Trigger_C = ECSManager.FindEntityByName("Trigger_C");
			Trigger_D = ECSManager.FindEntityByName("Trigger_D");
			Trigger_E = ECSManager.FindEntityByName("Trigger_E");
			Trigger_F = ECSManager.FindEntityByName("Trigger_F");
			Trigger_G = ECSManager.FindEntityByName("Trigger_G");
			Trigger_H = ECSManager.FindEntityByName("Trigger_H");


			cameraController = ECSManager.FindEntityByName("Main Camera").GetComponent<CameraController>();
			Debug.Log("CameraController ID is " + cameraController.ID);

			expectedDistance = 35;
			expectedPosition = new vec3(0, 10, 20);
			expectedRotation = new vec3(30, 180, 0);

		}

		public void Update()
		{
			regionA = IsInsideTrigger(Trigger_A);
			regionB = IsInsideTrigger(Trigger_B);
			regionC = IsInsideTrigger(Trigger_C);
			regionD = IsInsideTrigger(Trigger_D);
			regionE = IsInsideTrigger(Trigger_E);
			regionF = IsInsideTrigger(Trigger_F);
			regionG = IsInsideTrigger(Trigger_G);
			regionH = IsInsideTrigger(Trigger_H);

			if (regionA)
			{
				expectedPosition = new vec3(0, 10, 20);
				expectedRotation = new vec3(30, 180, 0);
				expectedDistance = 35;
				cameraController.lookOnlyBool = false;
			}

			if (regionB)
			{
				expectedPosition = new vec3(0, 10, 20);
				expectedRotation = new vec3(45, 90, 0);
				expectedDistance = 50;
				cameraController.lookOnlyBool = false;
			}

			if (regionC)
			{
				cameraController.staticPosition = new vec3(130, 50, -204);
				cameraController.lookOnlyBool = true;
				expectedPosition = new vec3(112, 82, -207);
				expectedRotation = new vec3(50, 90, 0);
				expectedDistance = 60;
			}

			if (regionD)
			{
				expectedPosition = new vec3(121, 82, -207);
				expectedRotation = new vec3(45, 90, 0);
				expectedDistance = 70;
				cameraController.lookOnlyBool = false;
			}

			if (regionE)
			{
				cameraController.staticPosition = new vec3(320, 45, -330);
				cameraController.lookOnlyBool = true;
				expectedPosition = new vec3(260, 99, -335);
				expectedRotation = new vec3(45, 90, 0);
				expectedDistance = 80;
			}

			if (regionF)
			{
				expectedPosition = new vec3(0, 10, 20);
				expectedRotation = new vec3(30, 0, 0);
				expectedDistance = 110;
				cameraController.lookOnlyBool = false;
			}

			if (regionG)
			{
				expectedPosition = new vec3(0, 10, 20);
				expectedRotation = new vec3(30, 180, 0);
				expectedDistance = 80;
				cameraController.lookOnlyBool = false;
			}

			if (regionH) 
			{
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
			return ECSManager.IsValidEntity(entity.ID) && (PS.IsTriggerEnter(Holey.ID, entity.ID) || PS.IsTriggerStay(Holey.ID, entity.ID));
		}
	}
}
