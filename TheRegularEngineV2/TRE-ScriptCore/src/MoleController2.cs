using System;
using System.Collections.Generic;
using System.Reflection;
using System.Runtime.CompilerServices;
using static System.Runtime.CompilerServices.RuntimeHelpers;
using System.Threading;

namespace TRE
{
    class MoleController2 : Entity
    {
        //Check if player is on the ground
        private bool isGrounded = true;
        //Max height
        private Vector3 maxHeight = new Vector3(0, 5, 0);
        //direction vector
        private Vector3 dirVec;
        //Movement Vector
        Vector3 movementVectoer = Vector3.zero;
        //Max velocity
        private float maxVelocity = 15f;
        //Acceleration
        private float acceleration = 300f;
        //Deceleration
        private float deceleration = -10f;
        //velocity in the air
        private float airVelocity = 8f;
        //force direction
        private Vector3 forceDirection = Vector3.zero;
        //final velocity
        private Vector3 finalVelocity = Vector3.zero;


        //Wake up the mole
        private bool isAwake = false;


        //check if player used super power
        private bool isScaled = false;
        //Default scale
        private float defaultScale = 5;
        //Increase character scale
        private float superScale = 1;
        private Vector3 fat = new Vector3(3f, 2f, 3f);
        //Return width back to 0
        private Vector3 thin = new Vector3(0.01f, 0.01f, 0.01f);

        private Vector3 playerDirection = new Vector3(0, 0, 1);

        //For camera controller
        private Entity Trigger_A;
        private Entity Trigger_B;
        private Entity Trigger_C;
        private Entity Trigger_D;
        private Entity Trigger_E;
        private Entity Trigger_F;
        private Entity Trigger_G;

        public bool regionA;
        public bool regionB;
        public bool regionC;
        public bool regionD;
        public bool regionE;
        public bool regionF;
        public bool regionG;

        public void Start()
        {
            TransformSystem.SetRotation(this.ID, new Vector3(0, 0, 0));
        }

        public void Update()
        {
            TransformSystem.GetPosition(this.ID, out Vector3 pos);
            PhysicsSystem.ConstrainRotationX(this.ID, true);
            PhysicsSystem.ConstrainRotationY(this.ID, true);
            PhysicsSystem.ConstrainRotationZ(this.ID, true);

            //Movement Related stuff
            PhysicsSystem.GetLinearVelocity(this.ID, out Vector3 currVelocity);

            dirVec = new Vector3(0, 0, 0);

            if (InputSystem.GetKeyDown(InputKeys.I))
            {
                dirVec.z += -1;
                playerDirection.y = 180;
            }

            if (InputSystem.GetKeyDown(InputKeys.K))
            {
                dirVec.z += 1;
                playerDirection.y = 0;
            }

            if (InputSystem.GetKeyDown(InputKeys.J))
            {
                dirVec.x += -1;
                playerDirection.y = 270;
            }

            if (InputSystem.GetKeyDown(InputKeys.L))
            {
                dirVec.x += 1;
                playerDirection.y = 90;
            }

            if (InputSystem.GetKeyDown(InputKeys.I))
            {
                if (InputSystem.GetKeyDown(InputKeys.L))
                {
                    playerDirection.y = 135;
                }
                if (InputSystem.GetKeyDown(InputKeys.J))
                {
                    playerDirection.y = 225;
                }
            }

            if (InputSystem.GetKeyDown(InputKeys.K))
            {
                if (InputSystem.GetKeyDown(InputKeys.L))
                {
                    playerDirection.y = 45;
                }
                if (InputSystem.GetKeyDown(InputKeys.J))
                {
                    playerDirection.y = 315;
                }
            }

            if (InputSystem.GetKeyDown(InputKeys.Enter))
            {
                if (isGrounded)
                {
                    Jump(maxHeight);
                }
            }

            if (InputSystem.GetKeyDown(InputKeys.Backspace))
            {
                if (!isScaled)
                {
                    //tall boi
                    PhysicsSystem.ResizeCapsuleCollider(this.ID, defaultScale, superScale);
                    isScaled = true;
                }
                else if (isScaled)
                {
                    //for tall boi
                    PhysicsSystem.ResizeCapsuleCollider(this.ID, 3, 2);
                    isScaled = false;
                }
            }

            dirVec.Normalize();

            if (dirVec != Vector3.zero)
            {
                if (Math.Sqrt(currVelocity.x * currVelocity.x + currVelocity.z * currVelocity.z) < maxVelocity)
                {
                    finalVelocity = currVelocity + (dirVec * acceleration * Time.GetDeltaTime());
                    PhysicsSystem.SetLinearVelocity(this.ID, finalVelocity);
                }
                else
                {
                    Vector3 tmp = dirVec * maxVelocity;
                    finalVelocity = new Vector3(tmp.x, currVelocity.y, tmp.z);
                    PhysicsSystem.SetLinearVelocity(this.ID, finalVelocity);
                }
            }

            TransformSystem.SetRotation(this.ID, playerDirection);
        }
        private void Jump(Vector3 JumpHeight)
        {
            //PhysicsSystem.SetLinearVelocity(this.ID, JumpHeight);
            PhysicsSystem.AddForce(this.ID, JumpHeight, ForceMode.VelocityChange);
        }

        public static float lerp(float start, float end, float t)
        {
            if (t > 1)
                t = 1;
            else if (t < 0)
                t = 0;
            return start + (end - start) * t;
        }

        private void OnCollisionStay(System.UInt64 otherID)
        {
            isGrounded = false;

            Entity other = new Entity(otherID);
            if (PhysicsSystem.IsCollisionStay(this.ID, otherID))
            {
                if (EngineGetTag(otherID) == "Ground" || EngineGetTag(otherID) == "Player")
                {
                    isGrounded = true;
                }
                else
                {
                    isGrounded = false;
                }
            }
        }
    }
}
