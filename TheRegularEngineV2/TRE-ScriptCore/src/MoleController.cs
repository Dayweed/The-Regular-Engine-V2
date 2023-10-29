using System;
using System.Collections.Generic;
using System.Reflection;
using System.Runtime.CompilerServices;
using static System.Runtime.CompilerServices.RuntimeHelpers;
using System.Threading;
namespace TRE
{
	public class MoleController : Entity
	{
        //check if player is on the ground (for now , just a plane)
        private bool isGrounded = true;
        //Maxium height the player can jump
        private Vector3 maxHeight = new Vector3(0, 10000, 0);
        //direction vector
        private Vector3 dirVec;
        //Wake up the mole
        private bool isAwake = false;


        //check if player used super power
        private bool isScaled = false;
        //Default scale
        private float defaultScale = 1;
        //Increase character scale
        private float superScale = 5;
        //For now the floor collision
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
            TransformSystem.GetPosition(this.ID, out Vector3 pos);
            PhysicsSystem.ConstrainRotationX(this.ID, true);
            PhysicsSystem.ConstrainRotationZ(this.ID, true);

            dirVec = new Vector3(0, 0, 0);

            if (InputSystem.GetKeyDown(InputKeys.W))
            {
                dirVec.z += 1;
            }

            if (InputSystem.GetKeyDown(InputKeys.S))
            {
                dirVec.z += -1;
            }

            if (InputSystem.GetKeyDown(InputKeys.A))
            {
                dirVec.x += 1;
            }

            if (InputSystem.GetKeyDown(InputKeys.D))
            {
                dirVec.x += -1;
            }

            if (InputSystem.GetKeyDown(InputKeys.Space))
            {
                // if the player JUST starts to touch the ground OR has been chilling on the ground for a while
                isGrounded = PhysicsSystem.IsCollisionEnter(this.ID, Plane_collider.ID) || PhysicsSystem.IsCollisionStay(this.ID, Plane_collider.ID);

                if (isGrounded)
                    Jump(maxHeight);
            }

            if (InputSystem.GetKeyDown(InputKeys.E))
            {
                if (!isScaled)
                {
                    PhysicsSystem.ResizeCapsuleCollider(this.ID, superScale, superScale);
                    isScaled = true;
                }
                else if (isScaled)
                {
                    PhysicsSystem.ResizeCapsuleCollider(this.ID, defaultScale, defaultScale);
                    isScaled = false;
                }
            }

            dirVec.Normalize();

            Vector3 tmp = dirVec * 10;

            PhysicsSystem.AddForce(this.ID, tmp, ForceMode.VelocityChange);
        }
        private void Jump(Vector3 JumpHeight)
        {
            PhysicsSystem.AddForce(this.ID, JumpHeight, ForceMode.Force);
        }
    }
}
