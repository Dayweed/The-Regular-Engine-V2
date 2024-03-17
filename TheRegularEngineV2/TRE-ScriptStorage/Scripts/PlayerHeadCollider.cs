using GlmSharp;

namespace TRE
{
	using PS = PhysicsSystem;

	class PlayerHeadCollider : Entity
	{
		private Entity playerObj;
		private const string mole1tag = "RedCollider";
		private const string mole2tag = "BlueCollider";
		//bool isPlayer1 = false;
		//bool isPlayer2 = false;
		// private vec3 offset;

		public void SetToPlayer()
		{
			// PS.GetColliderOffset(playerObj.ID, out offset);

			if (playerObj == null || ECSManager.IsValidEntity(playerObj.ID) == false)
			{
				return;
			}
			if (this.CompareTag(mole1tag))
			{
				vec3 newPos = playerObj.transform.Position;
				if (!playerObj.GetComponent<MoleyController>().isScaled)
				{
					newPos.y += playerObj.GetComponent<MoleyController>().currOffset + playerObj.GetComponent<CapsuleCollider>().HalfHeight * 2f + 1f;
					PS.ResizeBoxCollider(this.ID, new vec3(2, 0.5f, 2));
				}
				else
				{
					if (playerObj.GetComponent<MoleyController>().mainBlueberry)
					{
						newPos.y += playerObj.GetComponent<MoleyController>().currOffset + playerObj.GetComponent<CapsuleCollider>().HalfHeight * 2f;
						PS.ResizeBoxCollider(this.ID, new vec3(4, 2, 4));
					}

					if (playerObj.GetComponent<MoleyController>().mainStrawberry)
					{
						newPos.y += playerObj.GetComponent<MoleyController>().currOffset + playerObj.GetComponent<CapsuleCollider>().HalfHeight * 2f;
						PS.ResizeBoxCollider(this.ID, new vec3(4, 2, 4));
					}
				}

				transform.Position = newPos;
			}
			else if (this.CompareTag(mole2tag))
			{
				vec3 newPos = playerObj.transform.Position;
				if (!playerObj.GetComponent<HoleyController>().isScaled)
				{
					newPos.y += playerObj.GetComponent<HoleyController>().currOffset + playerObj.GetComponent<CapsuleCollider>().HalfHeight * 2f + 1f;
					PS.ResizeBoxCollider(this.ID, new vec3(2, 0.5f, 2));
				}
				else
				{
					if (playerObj.GetComponent<HoleyController>().mainBlueberry)
					{
						newPos.y += playerObj.GetComponent<HoleyController>().currOffset + playerObj.GetComponent<CapsuleCollider>().HalfHeight * 2f + 2.25f;
					}

					if (playerObj.GetComponent<HoleyController>().mainStrawberry)
					{
						newPos.y += playerObj.GetComponent<HoleyController>().currOffset + playerObj.GetComponent<CapsuleCollider>().HalfHeight * 2f + 8f;
						PS.ResizeBoxCollider(this.ID, new vec3(3, 0.5f, 3));
					}
				}

				transform.Position = newPos;
			}
		}

		public void Start()
		{
			if (this.CompareTag(mole1tag))
			{
				playerObj = ECSManager.FindEntityByName("Moley");
			}
			else if (this.CompareTag(mole2tag))
			{
				playerObj = ECSManager.FindEntityByName("Holey");
			}
		}

		public void Update()
		{
			SetToPlayer();
		}
	}
}
