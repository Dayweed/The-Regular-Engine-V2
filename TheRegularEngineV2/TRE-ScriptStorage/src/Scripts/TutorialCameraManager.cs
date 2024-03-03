using GlmSharp;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace TRE
{
	using PS = PhysicsSystem;

	public class TutorialCameraManager : Entity
	{
		private Entity MidPos;

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
			MidPos = ECSManager.FindEntityByName("MidPos");
			Debug.Log("MidPos ID is " + MidPos.ID);

			Trigger_A = ECSManager.FindEntityByName("Trigger_A");
			Debug.Log("Trigger_A ID is " + Trigger_A.ID);

			Trigger_B = ECSManager.FindEntityByName("Trigger_B");
			Debug.Log("Trigger_B ID is " + Trigger_B.ID);

			Trigger_C = ECSManager.FindEntityByName("Trigger_C");
			Debug.Log("Trigger_C ID is " + Trigger_C.ID);

			Trigger_D = ECSManager.FindEntityByName("Trigger_D");
			Debug.Log("Trigger_D ID is " + Trigger_D.ID);

			Trigger_E = ECSManager.FindEntityByName("Trigger_E");
			Debug.Log("Trigger_E ID is " + Trigger_E.ID);

			Trigger_F = ECSManager.FindEntityByName("Trigger_F");
			Debug.Log("Trigger_F ID is " + Trigger_F.ID);

			Trigger_G = ECSManager.FindEntityByName("Trigger_G");
			Debug.Log("Trigger_G ID is " + Trigger_G.ID);

			Trigger_H = ECSManager.FindEntityByName("Trigger_H");
			Debug.Log("Trigger_H ID is " + Trigger_H.ID);

			cameraController = ECSManager.FindEntityByName("Main Camera").GetComponent<CameraController>();

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
				//starting region
				expectedPosition = new vec3(0, 10, 20);
				expectedRotation = new vec3(30, 180, 0);
				expectedDistance = 35;
				cameraController.lookOnlyBool = false;
				cameraController.expectedYPos = -2f;
			}

			if (regionB)
			{
				//first platforming section w/ blueberries
				expectedPosition = new vec3(0, 30, 60);
				expectedRotation = new vec3(45, 180, 0);
				expectedDistance = 55;
				cameraController.lookOnlyBool = false;
				cameraController.expectedYPos = -2f;
			}

			if (regionC)
			{
				//first hitw
				expectedPosition = new vec3(0, 40, 50);
				expectedRotation = new vec3(25, 180, 0);
				expectedDistance = 35;
				cameraController.lookOnlyBool = false;
				cameraController.expectedYPos = -2f;
			}

			if (regionD)
			{
				//falling objs section
				expectedPosition = new vec3(0, 40, 50);
				expectedRotation = new vec3(45, 180, 0);
				expectedDistance = 55;
				cameraController.lookOnlyBool = false;
				cameraController.expectedYPos = 0f;
			}

			if (regionE)
			{
				//platforming section
				expectedPosition = new vec3(0, 60, 50);
				expectedRotation = new vec3(45, 180, 0);
				expectedDistance = 60;
				cameraController.lookOnlyBool = false;
				cameraController.expectedYPos = 0f;
			}

			if (regionF)
			{
				//second hitw
				expectedPosition = new vec3(0, 40, 50);
				expectedRotation = new vec3(25, 180, 0);
				expectedDistance = 35;
				cameraController.lookOnlyBool = false;
				cameraController.expectedYPos = 0f;
			}

			if (regionG)
			{
				//final section
				cameraController.staticPosition = new vec3(110, 20, -450);
				expectedPosition = new vec3(50, 40, 50);
				expectedRotation = new vec3(35, 220, 0);
				expectedDistance = 50;
				cameraController.lookOnlyBool = true;
				cameraController.expectedYPos = 28f;
			}

			if (regionH)
			{
				//last hitw
				expectedPosition = new vec3(50, 40, 50);
				expectedRotation = new vec3(25, 180, 0);
				expectedDistance = 40;
				cameraController.lookOnlyBool = false;
				cameraController.expectedYPos = 28f;
			}

			cameraController.expectedPosition = expectedPosition;
			cameraController.expectedRotation = expectedRotation;
			cameraController.expectedDistance = expectedDistance;
			cameraController.toTransition = PS.IsTriggerEnter(MidPos.ID, Trigger_A.ID) || PS.IsTriggerEnter(MidPos.ID, Trigger_B.ID) || PS.IsTriggerEnter(MidPos.ID, Trigger_C.ID)
											|| PS.IsTriggerEnter(MidPos.ID, Trigger_D.ID) || PS.IsTriggerEnter(MidPos.ID, Trigger_E.ID) || PS.IsTriggerEnter(MidPos.ID, Trigger_F.ID)
											|| PS.IsTriggerEnter(MidPos.ID, Trigger_G.ID) || PS.IsTriggerEnter(MidPos.ID, Trigger_H.ID);
		}

		private bool IsInsideTrigger(Entity entity)
		{
			return PS.IsTriggerEnter(MidPos.ID, entity.ID) || PS.IsTriggerStay(MidPos.ID, entity.ID);
		}
	}
}
