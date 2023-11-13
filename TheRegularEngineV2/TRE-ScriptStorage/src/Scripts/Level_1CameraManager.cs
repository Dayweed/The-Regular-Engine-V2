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
		
		private bool regionA;
		private bool regionB;
		
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

			if (regionA)
			{
				expectedPosition = new vec3(0, 10, 20);
				expectedRotation = new vec3(30, 180, 0);
				expectedDistance = 35;
			}

			if(regionB)
			{
				expectedPosition = new vec3(0, 10, 20);
				expectedRotation = new vec3(45, 90, 0);
				expectedDistance = 60;
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
