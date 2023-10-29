using System;
using System.Reflection;
using System.Runtime.CompilerServices;

namespace TRE
{
	public class MoleController : Entity
	{
        //check if player is on the ground (for now , just a plane)
        private bool isGrounded = true;
        //Maxium height the player can jump
        private Vector3 maxHeight = new Vector3(0, 10000, 0);


        //check if player used super power
        private bool isScaled = false;
        private float defaultScale = 1;
        private float superScale = 50;

        public Entity Plane_collider;

        //Player has a TransformSystem

        public void Start()
        {
            Plane_collider = ECSManager.FindEntityByName("Plane collider");
            Core.Log("My ID is " + this.ID);
            Core.Log("Plane ID is " + Plane_collider.ID);
        }

		public void Update()
		{
            // Move The Test Object 
            //TransformSystem.GetPosition(this.ID, out Vector3 pos);
            //

            Vector3 dirVec = new Vector3(0, 0, 0);

            if (InputSystem.GetKeyDown(InputKeys.W))
            {
                dirVec.z += 1;
                //PS.AddForce(Mole.id, dirVec);
            }

            if (InputSystem.GetKeyDown(InputKeys.S))
            {
                dirVec.z += -1;
                //PS.AddForce(Mole.id, dirVec);
            }

            if (InputSystem.GetKeyDown(InputKeys.A))
            {
                dirVec.x += -1;
            }

            if (InputSystem.GetKeyDown(InputKeys.D))
            {
                dirVec.x += 1;
            }

            if (InputSystem.GetKeyDown(InputKeys.Space))
            {
                // if the player JUST starts to touch the ground OR has been chilling on the ground for a while
                isGrounded = PhysicsSystem.IsCollisionEnter(this.ID, Plane_collider.ID) || PhysicsSystem.IsCollisionStay(this.ID, Plane_collider.ID);

                if (isGrounded)
                {
                    Jump(maxHeight); // uh oh beeeg number (for forcemode.force)
                    // Jump(new Vector3(0, 35, 0)); // ah, much better (for forcemode.velchange)
                }
            }

            if (InputSystem.GetKeyDown(InputKeys.E))
            {
                if (!isScaled)
                {
                    PhysicsSystem.ResizeSphereCollider(this.ID, superScale);
                    isScaled = true;
                }
                else if (isScaled)
                {
                    PhysicsSystem.ResizeSphereCollider(this.ID, defaultScale);
                    isScaled = false;
                }
            }

            dirVec.Normalize();

            Vector3 tmp = dirVec * 60;

            PhysicsSystem.AddForce(this.ID, tmp, ForceMode.Force);

            /* NOT SURE IF THIS IS THE CORRECT ONE...
            TransformSystem.GetPosition(this.ID, out Vector3 pos);

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
            PhysicsSystem.AddForce(this.ID, tmp, ForceMode.Force);

            TransformSystem.SetPosition(this.ID, pos);
            */
        }
        private void Jump(Vector3 JumpHeight)
        {
            PhysicsSystem.AddForce(this.ID, JumpHeight, ForceMode.Force);
            // PS.AddForce(Test.id, 35, PS.ForceMode.VelocityChange);
        }
		private void OnTriggerStay(System.UInt64 otherID)
		{
			Entity other = new Entity(otherID);
			Core.Log("Triggered with " + ECSManager.FindNameFromID(other.ID));
		}
	}
}
