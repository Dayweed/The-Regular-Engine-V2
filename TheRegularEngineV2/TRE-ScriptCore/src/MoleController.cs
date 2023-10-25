using System;
using System.Reflection;
using System.Runtime.CompilerServices;

namespace TRE
{
	public class MoleController
	{
		//Player has a TransformSystem
		public Entity Mole = new Entity("Test");
		public MoleController()
		{
			Mole.id = ECSManager.FindIDFromName(Mole.name);
		}

		public void Update()
		{
			TransformSystem.GetPosition(Mole.id, out Vector3 pos);
			//
			Vector3 tmp = new Vector3(0, 0, 0);

			if (InputSystem.GetKeyDown(InputKeys.W))
			{
				tmp.x = 60;
				PhysicsSystem.AddForce(Mole.id, tmp);
			}

			if (InputSystem.GetKeyDown(InputKeys.S))
			{
				tmp.y = -60;
				PhysicsSystem.AddForce(Mole.id, tmp);
			}

			if (InputSystem.GetKeyDown(InputKeys.A))
			{
				pos.x -= 10.0f;
			}

			if (InputSystem.GetKeyDown(InputKeys.D))
			{
				pos.x += 10.0f;
			}

			TransformSystem.SetPosition(Mole.id, pos);
		}
	}
}
