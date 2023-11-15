using System;
using System.Collections.Generic;
using System.Reflection;
using System.Runtime.CompilerServices;
using static System.Runtime.CompilerServices.RuntimeHelpers;
using System.Threading;
using GlmSharp;

namespace TRE
{
    public class RollingObj : Entity
    {
        public int moveDir = 1;
        public vec3 defaultVector = new vec3(1, 0, 0);
        public vec3 moveVector = new vec3(1, 0, 0);

        public vec3 rotateVector = new vec3(0, 0, -1);
        public vec3 threesixty = new vec3(0, 0, 360);

        // For checking ledges
        Entity lLedge;
        Entity rLedge;

        private float moveSpeed = 2.25f;
        private float rotateSpeed = 50.0f;

        public RollingObj()
        {

        }

        public void Start()
        {
            lLedge = parenting.parent.parenting.GetChildFromName("LeftLedge");
            rLedge = parenting.parent.parenting.GetChildFromName("RightLedge");

            // Rotate moveVector based on angle
        }

        public void Update()
        {
            // Check if the ledges is no longer being triggered
            if (lLedge == null || rLedge == null) return;

            if (PhysicsSystem.IsTriggerEnter(ID, lLedge.ID) || PhysicsSystem.IsTriggerEnter(ID, rLedge.ID))
            {
                Bounceback();
            }

            transform.Position += moveVector * moveDir * moveSpeed * Time.deltaTime;
            transform.Rotation += rotateVector * moveDir * rotateSpeed * Time.deltaTime;
            transform.Rotation = transform.Rotation.z > 360 ? transform.Rotation - threesixty : transform.Rotation;
            transform.Rotation = transform.Rotation.z < 0 ? transform.Rotation + threesixty : transform.Rotation;
        }

        public void Bounceback()
        {
            moveDir = moveDir == 1 ? -1 : 1;
        }

        private void OnCollisionStay(System.UInt64 otherID)
        {
            Entity other = new Entity(otherID);
            if (other.CompareTag("Red"))
            {
                other.GetComponent<MoleyController>().TakeDamage();
            }
            else if (other.CompareTag("Blue"))
            {
                other.GetComponent<HoleyController>().TakeDamage();
            }
            else if (other.ID == lLedge.ID || other.ID == rLedge.ID)
            {
                Bounceback();
            }
        }
    }
}