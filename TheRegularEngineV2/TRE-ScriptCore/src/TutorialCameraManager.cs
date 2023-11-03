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

		private Vector3 expectedPosition;
		private Vector3 expectedRotation;
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
				expectedPosition = new Vector3(0, 10, 20);
				expectedRotation = new Vector3(30, 180, 0);
				expectedDistance = 35;
			}

			if (regionB)
			{
				expectedPosition = new Vector3(0, 35, 50);
				expectedRotation = new Vector3(45, 180, 0);
				expectedDistance = 50;
			}

			if (regionC)
			{
				expectedPosition = new Vector3(0, 40, 50);
				expectedRotation = new Vector3(15, 180, 0);
				expectedDistance = 25;
			}

			if (regionD)
			{
				expectedPosition = new Vector3(0, 40, 50);
				expectedRotation = new Vector3(45, 180, 0);
				expectedDistance = 50;
			}

			if (regionE)
			{
				expectedPosition = new Vector3(0, 40, 50);
				expectedRotation = new Vector3(30, 180, 0);
				expectedDistance = 50;
			}

			if (regionF)
			{
				expectedPosition = new Vector3(0, 40, 50);
				expectedRotation = new Vector3(45, 180, 0);
				expectedDistance = 65;
			}

			if (regionG)
			{
				expectedPosition = new Vector3(50, 40, 50);
				expectedRotation = new Vector3(35, 200, 0);
				expectedDistance = 50;
			}

			if (regionH)
			{
				expectedPosition = new Vector3(50, 40, 50);
				expectedRotation = new Vector3(20, 180, 0);
				expectedDistance = 20;
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
