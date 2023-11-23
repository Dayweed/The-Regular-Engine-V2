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

		private float moveSpeed = 7.5f;
		private float offset = 0.1f;

		private float currentTime = 0.0f;
		private float delay = 2.0f;

		public PlatformLogic()
		{
			positions = new vec3[0];
		}

		private vec3 oldPosition = new vec3(0,0,0);

		public void Start()
		{
            if (Scene.GetSceneName() == "TODELETE")
            {
                currentIndex = 0;
                positions = new vec3[] { new vec3(16.394f, 14.422f, -227), new vec3(16.394f, 14.422f, -178) };
            }
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

			if (currentIndex < positions.Length)
			{
                transform.Position = positions[currentIndex];
            }
		}

		public void Update()
		{
			oldPosition = transform.Position;

			float dirX = (positions[currentIndex].x - transform.Position.x);
            float dirY = (positions[currentIndex].y - transform.Position.y);
            float dirZ = (positions[currentIndex].z - transform.Position.z);

			vec3 dir = new vec3(dirX, dirY, dirZ);
			vec3 normDir = new vec3();
			if (dir.Length > 0)
			{
                normDir = dir.Normalized;
            }

            // Lerps through each positions
            if (positions.Length > 0)
			{
				if (currentTime <= 0.0f)
				{
					transform.Position += normDir * moveSpeed * Time.deltaTime;
					// Move to next index, if it is very close to the ideal position
					if (IsNearPosition(positions[currentIndex]))
					{
						++currentIndex;
						if (currentIndex >= positions.Length) currentIndex = 0;
						//transform.Position = positions[currentIndex];
                        currentTime = delay;
                    }
				}
				else
				{
					currentTime -= Time.deltaTime;
                }
			}
		}

		public void OnCollisionStay(System.UInt64 otherID)
		{
			Entity other = new Entity(otherID);
			//if on the platform unchild it
			if (other.CompareTag("Red") || other.CompareTag("Blue"))
			{
				other.GetComponent<Transform>().Position += transform.Position - oldPosition;
			}
		}

		private bool IsNearPosition(vec3 targetPosition)
		{
            vec3 minusOffset = targetPosition - new vec3(offset);
            vec3 plusOffset = targetPosition + new vec3(offset);

			vec3 minOffset = new vec3(
				(minusOffset.x < plusOffset.x) ? minusOffset.x : plusOffset.x,
				(minusOffset.y < plusOffset.y) ? minusOffset.y : plusOffset.y,
				(minusOffset.z < plusOffset.z) ? minusOffset.z : plusOffset.z
                );

			vec3 maxOffset = new vec3(
				(minusOffset.x > plusOffset.x) ? minusOffset.x : plusOffset.x,
				(minusOffset.y > plusOffset.y) ? minusOffset.y : plusOffset.y,
				(minusOffset.z > plusOffset.z) ? minusOffset.z : plusOffset.z
                );

            return transform.Position.x >= minOffset.x && transform.Position.x <= maxOffset.x
                && transform.Position.y >= minOffset.y && transform.Position.y <= maxOffset.y
                && transform.Position.z >= minOffset.z && transform.Position.z <= maxOffset.z;
		}
	}
}