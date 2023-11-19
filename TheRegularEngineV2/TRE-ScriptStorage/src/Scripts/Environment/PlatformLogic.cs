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

		private float lerpSpeed = 1.5f;
		private float offset = 0.1f;

		public PlatformLogic()
		{
			positions = new vec3[0];
		}

		private vec3 oldPosition = new vec3(0,0,0);

		public void Start()
		{
			if (Scene.GetSceneName() == "Level_1")
			{
				// Determine platform positions based on data
				if (name == "MovingPlatform_1")
				{
					currentIndex = 0;
					positions = new vec3[] { new vec3(16.394f, 14.422f, -227), new vec3(16.394f, 14.422f, -178) };
				}

				if (name == "MovingPlatform_2")
				{
					currentIndex = 0;
					positions = new vec3[] { new vec3(146.419f, 14.422f, -237), new vec3(146.419f, 14.422f, -169) };
				}

				if (name == "MovingPlatform_3")
				{
					currentIndex = 1;
					positions = new vec3[] { new vec3(174.419f, 14.422f, -237), new vec3(174.419f, 14.422f, -169) };
				}

				if (name == "MovingPlatform_4")
				{
					currentIndex = 0;
					positions = new vec3[] { new vec3(200.419f, 14.422f, -237), new vec3(200.419f, 14.422f, -169) };
				}

				if (name == "MovingPlatform_5")
				{
					currentIndex = 0;
					positions = new vec3[] { new vec3(380.419f, 42.422f, -353), new vec3(380.419f, 42.422f, -309) };
				}

				if (name == "MovingPlatform_6")
				{
					currentIndex = 0;
					positions = new vec3[] { new vec3(380.419f, 42.422f, -257), new vec3(446.419f, 42.422f, -257) };
				}

				if (name == "Collider_MovingHITW")
				{
					currentIndex = 1;
					positions = new vec3[] { new vec3(446.419f, 42.422f, -227.107f), new vec3(528.400f, 42.422f, -227.107f) };
				}

				if (name == "MovingPlatform_7")
				{
					currentIndex = 0;
					positions = new vec3[] { new vec3(566.419f, 42.422f, -171), new vec3(566.419f, 90.422f, -171) };
				}

				if (name == "MovingPlatform_8")
				{
					currentIndex = 0;
					positions = new vec3[] { new vec3(642.419f, 90.422f, -171), new vec3(756.419f, 90.422f, -171) };
				}

				if (name == "MovingPlatform_9")
				{
					currentIndex = 1;
					positions = new vec3[] { new vec3(832.419f, 90.422f, -171), new vec3(894.419f, 90.422f, -171) };
				}
			}
		}

		public void Update()
		{
			oldPosition = transform.Position;

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

		private void OnCollisionStay(System.UInt64 otherID)
		{
			Entity other = new Entity(otherID);
			//if on the platform unchild it
			if (other.CompareTag("Red") || other.CompareTag("Blue"))
			{
				other.GetComponent<Transform>().Position += transform.Position - oldPosition;
			}
		}
	}
}