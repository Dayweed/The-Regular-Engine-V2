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

        // For checking ledges
        Entity lLedge;
        Entity rLedge;

        private float moveSpeed = 1.25f;

        public RollingObj()
        {

        }

        public void Start()
        {
            lLedge = parenting.GetChildFromName("LeftLedge");
            rLedge = parenting.GetChildFromName("RightLedge");

            // Rotate moveVector based on angle
        }

        public void Update()
        {
            
        }

        public void Bounceback()
        {
            moveDir = moveDir == 1 ? -1 : 1;
        }
    }
}