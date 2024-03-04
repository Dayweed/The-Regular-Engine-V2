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

		private float moveSpeed = 25f;
		private float offset = 0.1f;

		private float currentTime = 0.0f;
		private float delay = 1f;

		private Entity TriggerPlatformCollider;
		private Entity Holey;
		private Entity Moley;

		public PlatformLogic()
		{
			positions = new vec3[0];
		}

		private vec3 oldPosition = new vec3(0, 0, 0);

		public void Start()
		{
			TriggerPlatformCollider = parenting.GetChildFromName("TriggerPlatform");
			Holey = ECSManager.FindEntityByName("Holey");
			Moley = ECSManager.FindEntityByName("Moley");

			if (Scene.GetSceneName() == "TODELETE")
			{
				currentIndex = 0;
				positions = new vec3[] { new vec3(16.394f, 14.422f, -230), new vec3(16.394f, 14.422f, -175) };
			}
			if (Scene.GetSceneName() == "Level_1")
			{
				// Determine platform positions based on data
				if (name == "MovingPlatform_1")
				{
					currentIndex = 0;
					positions = new vec3[] { new vec3(16.394f, 13.403f, -230), new vec3(16.394f, 13.403f, -175) };
				}

				if (name == "MovingPlatform_2")
				{
					currentIndex = 0;
					positions = new vec3[] { new vec3(146.419f, 14.422f, -240), new vec3(146.419f, 14.422f, -166) };
				}

				if (name == "MovingPlatform_3")
				{
					currentIndex = 1;
					positions = new vec3[] { new vec3(174.419f, 14.422f, -240), new vec3(174.419f, 14.422f, -166) };
				}

				if (name == "MovingPlatform_4")
				{
					currentIndex = 0;
					positions = new vec3[] { new vec3(202.419f, 14.422f, -240), new vec3(202.419f, 14.422f, -166) };
				}

				if (name == "MovingPlatform_5")
				{
					currentIndex = 0;
					positions = new vec3[] { new vec3(376.419f, 42.422f, -349.472f), new vec3(376.419f, 42.422f, -309.694f) };
				}

				if (name == "MovingPlatform_6")
				{
					currentIndex = 0;
					delay = 4;
					positions = new vec3[] { new vec3(376.419f, 42.422f, -258.370f), new vec3(450.655f, 42.422f, -258.370f) };
				}

				if (name == "Level1_HitW_02")
				{
					currentIndex = 1;
					delay = 4;
					positions = new vec3[] { new vec3(450.655f, 60.340f, -227.529f), new vec3(524.892f, 60.340f, -227.529f) };
				}

				if (name == "MovingPlatform_7")
				{
					currentIndex = 0;
					positions = new vec3[] { new vec3(563.380f, 42.422f, -182.099f), new vec3(563.380f, 90.913f, -182.099f) };
				}

				if (name == "MovingPlatform_8")
				{
					currentIndex = 0;
					delay = 2;
					positions = new vec3[] { new vec3(644.877f, 90.422f, -182.099f), new vec3(833.877f, 90.422f, -182.099f) };
				}

				if (name == "MovingPlatform_9")
				{
					currentIndex = 1;
					delay = 2;
					positions = new vec3[] { new vec3(914.419f, 90.422f, -182.099f), new vec3(943.419f, 90.422f, -182.099f) };
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

			// Make Moley or Holey follow the platform if within trigger box
			if (HasComponent<DirectPathfinding>() && ECSManager.IsValidEntity(TriggerPlatformCollider.ID))
			{
				// Check if is in trigger
				if (PhysicsSystem.IsTriggerStay(TriggerPlatformCollider.ID, Holey.ID))
				{
					Holey.GetComponent<Transform>().Position += transform.Position - GetComponent<DirectPathfinding>().oldPosition;
				}
				if (PhysicsSystem.IsTriggerStay(TriggerPlatformCollider.ID, Moley.ID))
				{
					Moley.GetComponent<Transform>().Position += transform.Position - GetComponent<DirectPathfinding>().oldPosition;
				}
			}

            if (currentIndex >= positions.Length) return;

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

			// Make Moley or Holey follow the platform if within trigger box
			if (ECSManager.IsValidEntity(TriggerPlatformCollider.ID))
			{
				// Check if is in trigger
				if (PhysicsSystem.IsTriggerStay(TriggerPlatformCollider.ID, Holey.ID))
				{
					//Holey.GetComponent<Transform>().Position += transform.Position - oldPosition;
				}
				if (PhysicsSystem.IsTriggerStay(TriggerPlatformCollider.ID, Moley.ID))
				{
					//Moley.GetComponent<Transform>().Position += transform.Position - oldPosition;
				}
			}
		}

		public void OnCollisionStay(System.UInt64 otherID)
		{
			Entity other = new Entity(otherID);
			//if on the platform unchild it
			if (HasComponent<DirectPathfinding>())
			{
				if (other.CompareTag("Red") || other.CompareTag("Blue") || other.CompareTag("Strawberry") || other.CompareTag("Blueberry"))
				{
					other.GetComponent<Transform>().Position += (transform.Position - GetComponent<DirectPathfinding>().oldPosition) * Time.deltaTime;
					vec3 ahh = transform.Position - GetComponent<DirectPathfinding>().oldPosition;
				}
			}
			else
			{
				if (other.CompareTag("Red") || other.CompareTag("Blue") || other.CompareTag("Strawberry") || other.CompareTag("Blueberry"))
				{
					other.GetComponent<Transform>().Position += transform.Position - oldPosition;
				}
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