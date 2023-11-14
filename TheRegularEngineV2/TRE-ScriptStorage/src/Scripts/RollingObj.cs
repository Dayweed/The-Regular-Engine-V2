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
        RollingObjLedge lLedge;
        RollingObjLedge rLedge;

        // For display object
        Entity MyObject;

        private float moveSpeed = 2.25f;
        private float rotateSpeed = 50.0f;

        public RollingObj()
        {

        }

        public void Start()
        {
            MyObject = parenting.GetChildFromName("RollingObjectRender");
            lLedge = parenting.GetChildFromName("LeftLedge").GetComponent<RollingObjLedge>();
            rLedge = parenting.GetChildFromName("RightLedge").GetComponent<RollingObjLedge>();

            // Rotate moveVector based on angle
        }

        public void Update()
        {
            // Check if the ledges is no longer being triggered
            if (MyObject == null || lLedge == null || rLedge == null) return;

            if (!lLedge.isGrounded || !rLedge.isGrounded)
            {
                Bounceback();
            }

            transform.Position += moveVector * moveDir * moveSpeed * Time.deltaTime;
            MyObject.transform.Position = transform.Position;
            MyObject.transform.Rotation += rotateVector * moveDir * rotateSpeed * Time.deltaTime;
            //MyObject.transform.Rotation = MyObject.transform.Rotation.z > 360 ? MyObject.transform.Rotation - threesixty : MyObject.transform.Rotation;
            //MyObject.transform.Rotation = MyObject.transform.Rotation.z < 0 ? MyObject.transform.Rotation + threesixty : MyObject.transform.Rotation;
        }

        public void Bounceback()
        {
            moveDir = moveDir == 1 ? -1 : 1;
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
            else
            {
                Bounceback();
            }
        }
    }
}