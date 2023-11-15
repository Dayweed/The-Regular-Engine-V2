using System;
using System.Collections.Generic;
using System.Reflection;
using System.Runtime.CompilerServices;
using static System.Runtime.CompilerServices.RuntimeHelpers;
using System.Threading;
using GlmSharp;

namespace TRE
{
    public class PlatformLogic : Entity
    {
        public vec3[] positions;
        public int currentIndex;

        private float lerpSpeed = 1.25f;
        private float offset = 0.1f;

        public PlatformLogic()
        {
            positions = new vec3[0];
        }

        public void Start()
        {
            // Determine platform positions based on data
            if (Scene.GetSceneName() == "Level_1" && name == "Platform_1")
            {
                currentIndex = 0;
                positions = new vec3[] { new vec3(22, 18, -210), new vec3(22, 18, -174) };
            }
        }

        public void Update()
        {
            // Lerps through each positions
            if (positions.Length > 0)
            {
                transform.Position = MathF.Vec3Lerp(transform.Position, positions[currentIndex], lerpSpeed * Time.deltaTime);
                // Move to next index, if it is very close to the ideal position
                if (transform.Position.x >= positions[currentIndex].x - offset && transform.Position.x <= positions[currentIndex].x + offset
                    && transform.Position.y >= positions[currentIndex].y - offset && transform.Position.y <= positions[currentIndex].y + offset
                    && transform.Position.z >= positions[currentIndex].z - offset && transform.Position.z <= positions[currentIndex].z + offset)
                {
                    ++currentIndex;
                    if (currentIndex >= positions.Length) currentIndex = 0;
                }
            }
        }
    }
}