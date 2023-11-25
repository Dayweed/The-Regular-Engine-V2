using System;
using System.Collections.Generic;
using System.Reflection;
using System.Runtime.CompilerServices;
using static System.Runtime.CompilerServices.RuntimeHelpers;
using System.Threading;
using GlmSharp;

namespace TRE
{
	public class FallingObj : Entity
	{
		public float Duration;
		public float CurrentTime;

		public vec3 currentPos;

		public void Start()
		{
			Duration = 5.0f;
			CurrentTime = Duration;
			currentPos = transform.Position;
		}

		public void Update()
		{
			//CurrentTime -= Time.deltaTime;

			//if (CurrentTime <= 0)
			//{
			//    TransformSystem.SetPosition(ID, currentPos);
			//    CurrentTime = Duration;
			//}
		}
	}
}