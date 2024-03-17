using System.Collections.Generic;

namespace TRE
{
	public class PowerUpUI : Entity
	{
		SpriteRenderer activePowerUp;
		SpriteRenderer notactivePowerUp;

		string strawberryTexture = "Strawberry_HUD.png";
		string blueberryTexture = "Blueberry_HUD.png";

		public void Start()
		{
			activePowerUp = parenting.GetChildFromName("ActivePowerUp").GetComponent<SpriteRenderer>();
			notactivePowerUp = parenting.GetChildFromName("NotActivePowerUp").GetComponent<SpriteRenderer>();
		}

		public void UpdateUI(List<Entity> powerUps)
		{
			// Just in case the child have not assign yet
			if (activePowerUp == null)
				activePowerUp = parenting.GetChildFromName("ActivePowerUp").GetComponent<SpriteRenderer>();
			if (notactivePowerUp == null)
				notactivePowerUp = parenting.GetChildFromName("NotActivePowerUp").GetComponent<SpriteRenderer>();

			if (powerUps.Count == 0)
			{
				activePowerUp.isVisible = false;
				notactivePowerUp.isVisible = false;
				return;
			}

			if (powerUps.Count >= 2)
			{
				DisplayUI(powerUps[0], activePowerUp);
				DisplayUI(powerUps[1], notactivePowerUp);
			}
			else if (powerUps.Count >= 1)
			{
				DisplayUI(powerUps[0], activePowerUp);
				notactivePowerUp.isVisible = false;
			}
		}

		private void DisplayUI(Entity ent, SpriteRenderer display)
		{
			if (!ECSManager.IsValidEntity(ent.ID))
			{
				display.isVisible = false;
			}
			else
			{
				display.isVisible = true;

				if (ent.CompareTag("Strawberry"))
				{
					display.Texture = strawberryTexture;
				}
				else if (ent.CompareTag("Blueberry"))
				{
					display.Texture = blueberryTexture;
				}
			}
		}
	}
}