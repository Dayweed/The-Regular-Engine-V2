using GlmSharp;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace TRE
{
	class CactusPowerUp : Entity
	{
		private Entity playerObj;
		private string leftCactus = "LeftCactus";
		private string rightCactus = "RightCactus";

		//Scaling variables
		private float lerpSpeed = 5f;
		private vec3 defaultScale = new vec3(0.00000000001f, 0.00000000001f, 0.00000000001f);
		private vec3 currScale = new vec3(0f, 0f, 0f);
		private float separationMultiplier = 1.6f;
		private float verticalMultiplier = 1.5f;

		private vec3 leftScale = new vec3(1f, 4.2f, 1f); // the taller side
		private vec3 rightScale = new vec3(1f, 2f, 1f); // the shorter side

		private int[] directions = new int[7];

		private void SetToPlayer()
		{
			if (playerObj == null || ECSManager.IsValidEntity(playerObj.ID) == false)
			{
				return;
			}

			if (playerObj.GetComponent<HoleyController>().mainStrawberry == true)
			{
				PhysicsSystem.GetColliderOffset(playerObj.ID, out vec3 offset);
				vec3 newPos = playerObj.transform.Position;
				newPos.y += (playerObj.GetComponent<HoleyController>().currentHeight + playerObj.GetComponent<HoleyController>().currentRadius
						+ offset.y) * verticalMultiplier;

				transform.Position = newPos;

				TransformSystem.GetRotation(playerObj.ID, out vec3 playerRot);

				// place the left and right cactus parts according to Holey's rotation
				ApplyRotation(NiceRotationAngle(playerRot.y));
			}
		}

		private void ScaleChange()
		{
			if (this.CompareTag(rightCactus))
			{
				if (playerObj.GetComponent<HoleyController>().isScaled == false || !playerObj.GetComponent<HoleyController>().mainStrawberry)
				{
					currScale = MathF.Vec3Lerp(currScale, defaultScale, lerpSpeed * Time.deltaTime);
					PhysicsSystem.ResizeBoxCollider(this.ID, currScale);
				}
				else if (playerObj.GetComponent<HoleyController>().mainStrawberry)
				{
					currScale = MathF.Vec3Lerp(currScale, rightScale, lerpSpeed * Time.deltaTime);
					PhysicsSystem.ResizeBoxCollider(this.ID, currScale);
				}
			}
			else if (this.CompareTag(leftCactus))
			{
				if (playerObj.GetComponent<HoleyController>().isScaled == false || !playerObj.GetComponent<HoleyController>().mainStrawberry)
				{
					currScale = MathF.Vec3Lerp(currScale, defaultScale, lerpSpeed * Time.deltaTime);
					PhysicsSystem.ResizeBoxCollider(this.ID, currScale);
				}
				else if (playerObj.GetComponent<HoleyController>().mainStrawberry)
				{
					currScale = MathF.Vec3Lerp(currScale, leftScale, lerpSpeed * Time.deltaTime);
					PhysicsSystem.ResizeBoxCollider(this.ID, currScale);
				}
			}
		}

		private bool IsInRange(float rotY, float direction)
		{
			const float rangeInDegrees = 5.0f;
			if (direction - rangeInDegrees <= rotY && rotY <= direction + rangeInDegrees)
				return true;
			else
				return false;
		}

		// a 'nice' rotation is a rotation in degrees of only the following:
		// { 0, 45, 90, 135, 180, 225, 270, 315 }
		private float NiceRotationAngle(float rotYInDegrees)
		{
			while (rotYInDegrees < 0)
				rotYInDegrees += 360;
			while (rotYInDegrees > 360)
				rotYInDegrees -= 360;

			for (int i = 0; i < directions.Length; ++i)
			{
				if (IsInRange(rotYInDegrees, directions[i]))
					return directions[i];
			}
			return 0;
		}

		private void ApplyRotation(float niceRotY)
		{
			vec3 pos = transform.Position;
			float tempRadius = playerObj.GetComponent<HoleyController>().currentRadius * separationMultiplier;
			
			float angleInRadians = 0;
			if (CompareTag(leftCactus))
				angleInRadians = NiceRotationAngle(niceRotY - 90) / 180.0f * 3.141502f;
			else if (CompareTag(rightCactus))
				angleInRadians = NiceRotationAngle(niceRotY + 90) / 180.0f * 3.141502f;
			
			pos.z += tempRadius * (float)Math.Cos(angleInRadians);
			pos.x += tempRadius * (float)Math.Sin(angleInRadians);
			transform.Position = pos;
		}

		public void Start()
		{
			playerObj = ECSManager.FindEntityByName("Holey");

			// I can't do just the line below... :(
			// int[] directions = { 45, 90, 135, 180, 225, 270, 315 };
			for (int i = 0; i < 7; ++i)
				directions[i] = 45 * (i + 1);
		}

		public void Update()
		{
			// set to Holey's position
			SetToPlayer();
			//Check if Holey has strawberry then allow scaling
			ScaleChange();
		}
	}
}
