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

        public float bufferTime = 0.5f;
        public float bufferCurr = 0;

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

            //if (!lLedge.isGrounded || !rLedge.isGrounded)
            //{
            //    Bounceback();
            //}
            transform.Position += moveVector * moveDir * moveSpeed * Time.deltaTime;
            transform.Rotation += rotateVector * moveDir * rotateSpeed * Time.deltaTime;
            //MyObject.transform.Rotation = MyObject.transform.Rotation.z > 360 ? MyObject.transform.Rotation - threesixty : MyObject.transform.Rotation;
            //MyObject.transform.Rotation = MyObject.transform.Rotation.z < 0 ? MyObject.transform.Rotation + threesixty : MyObject.transform.Rotation;
        }

        public void Bounceback()
        {
            if (bufferCurr > 0)
            {
                bufferCurr -= Time.deltaTime;
                return;
            }
            moveDir = moveDir == 1 ? -1 : 1;
            bufferCurr = bufferTime;
        }

        private void OnTriggerStay(System.UInt64 otherID)
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