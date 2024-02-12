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
				expectedPosition = new vec3(0, 10, 20);
				expectedRotation = new vec3(30, 180, 0);
				expectedDistance = 40;
				cameraController.lookOnlyBool = false;
			}

			if (regionB)
			{
				expectedPosition = new vec3(0, 35, 50);
				expectedRotation = new vec3(45, 180, 0);
				expectedDistance = 50;
				cameraController.lookOnlyBool = false;
			}

			if (regionC)
			{
				expectedPosition = new vec3(0, 40, 50);
				expectedRotation = new vec3(25, 180, 0);
				expectedDistance = 35;
				cameraController.lookOnlyBool = false;
			}

			if (regionD)
			{
				expectedPosition = new vec3(0, 40, 50);
				expectedRotation = new vec3(45, 180, 0);
				expectedDistance = 55;
				cameraController.lookOnlyBool = false;
			}

			if (regionE)
			{
				expectedPosition = new vec3(0, 60, 50);
				expectedRotation = new vec3(45, 180, 0);
				expectedDistance = 55;
				cameraController.lookOnlyBool = false;
			}

			if (regionF)
			{
				expectedPosition = new vec3(0, 40, 50);
				expectedRotation = new vec3(25, 180, 0);
				expectedDistance = 35;
				cameraController.lookOnlyBool = false;
			}

			if (regionG)
			{
				cameraController.staticPosition = new vec3(110, 20, -415);
				expectedPosition = new vec3(50, 40, 50);
				expectedRotation = new vec3(35, 270, 0);
				expectedDistance = 45;
				cameraController.lookOnlyBool = true;
			}

			if (regionH)
			{
				expectedPosition = new vec3(50, 40, 50);
				expectedRotation = new vec3(25, 180, 0);
				expectedDistance = 40;
				cameraController.lookOnlyBool = false;
			}

			cameraController.expectedPosition = expectedPosition;
			cameraController.expectedRotation = expectedRotation;
			cameraController.expectedDistance = expectedDistance;
		}

		private bool IsInsideTrigger(Entity entity)
		{
			return PS.IsTriggerEnter(Holey.ID, entity.ID) || PS.IsTriggerStay(Holey.ID, entity.ID);
		}
	}
}
