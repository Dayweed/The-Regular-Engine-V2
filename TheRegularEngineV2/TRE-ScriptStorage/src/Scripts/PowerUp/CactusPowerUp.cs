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
		private string rightCactus = "RightCactus";
		private string leftCactus = "LeftCactus";

		//Scaling variables
		private float lerpSpeed = 5f;
		private vec3 defaultScale = new vec3(0.00000000001f, 0.00000000001f, 0.00000000001f);
		private vec3 leftScale = new vec3(2f, 4f, 2f);          // ROYCE CHANGE VALUES HERE
		private vec3 rightScale = new vec3(2f, 2f, 2f);         // ROYCE CHANGE VALUES HERE
		private vec3 currScale = new vec3(0f, 0f, 0f);

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
						+ offset.y) * 1.2f;
				//Right Cactus Values
				if (this.CompareTag(rightCactus))
					newPos.x += playerObj.GetComponent<HoleyController>().currentRadius * 1.3f;
				//Left Cactus Values
				else if (this.CompareTag(leftCactus))
					newPos.x -= playerObj.GetComponent<HoleyController>().currentRadius * 1.3f;
				transform.Position = newPos;
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

		public void Start()
		{
			playerObj = ECSManager.FindEntityByName("Holey");
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
