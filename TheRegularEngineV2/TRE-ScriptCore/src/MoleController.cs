using System;
using System.Reflection;
using System.Runtime.CompilerServices;

namespace TRE
{
	public class MoleController
	{
		//Player has a TransformSystem
		public Entity Mole = new Entity("Test", "");
		public MoleController()
		{
			Mole.ID = ECSManager.FindIDFromName(Mole.name);
		}

		public void Update()
		{
			TransformSystem.GetPosition(Mole.ID, out Vector3 pos);

			Vector3 dirVec = new Vector3(0, 0, 0);

			if (InputSystem.GetKeyDown(InputKeys.W))
			{
				dirVec.z += 1;
				//PhysicsSystem.AddForce(Mole.id, dirVec);
			}

			if (InputSystem.GetKeyDown(InputKeys.S))
			{
				dirVec.z += -1;
				//PhysicsSystem.AddForce(Mole.id, dirVec);
			}

			if (InputSystem.GetKeyDown(InputKeys.A))
			{
				dirVec.x += 1;
			}

			if (InputSystem.GetKeyDown(InputKeys.D))
			{
				dirVec.x += -1;
			}

			dirVec.Normalize();

			Vector3 tmp = dirVec * 60;

			// PhysicsSystem.AddForce(Mole.id, tmp);
			PhysicsSystem.AddForce(Mole.ID, tmp, PhysicsSystem.ForceMode.Force);

			TransformSystem.SetPosition(Mole.ID, pos);
		}
	}
}
