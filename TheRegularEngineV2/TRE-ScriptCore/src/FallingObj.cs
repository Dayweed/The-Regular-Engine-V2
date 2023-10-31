using System;
using System.Collections.Generic;
using System.Reflection;
using System.Runtime.CompilerServices;
using static System.Runtime.CompilerServices.RuntimeHelpers;
using System.Threading;

namespace TRE
{
    public class FallingObj : Entity
    {
        public float Duration;
        public float CurrentTime;

        public Vector3 currentPos;

        public void Start()
        {
            Duration = 7.0f;
            CurrentTime = Duration;
            currentPos = transform.Position;
        }

        public void Update()
        {
            CurrentTime -= Time.deltaTime;

            if (CurrentTime <= 0)
            {
                TransformSystem.SetPosition(ID, currentPos);
                CurrentTime = Duration;
            }
        }
    }
}