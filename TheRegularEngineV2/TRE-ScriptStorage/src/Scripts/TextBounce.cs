using System;
using System.Collections.Generic;
using System.Reflection;
using System.Runtime.CompilerServices;
using static System.Runtime.CompilerServices.RuntimeHelpers;
using System.Threading;
using GlmSharp;

namespace TRE
{
    public class TextBounce : Entity
    {
        float initialYPos;
        float timer = 0;
        float height = 10;
        float period = 10f;

        public void Start()
        {
            timer = 0;
            TransformSystem.GetPosition(this.ID, out vec3 pos);
            initialYPos = pos.y;
        }

        public void Update()
        {
            timer += Time.GetDeltaTime();
            TransformSystem.GetPosition(this.ID, out vec3 pos);
            float value = initialYPos + height * MathF.Sin(period * timer);
            pos.y = value;
            TransformSystem.SetPosition(this.ID, pos);
        }
    }
}