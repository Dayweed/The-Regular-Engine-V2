using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace TRE
{
	public class GameOver : Entity
	{
		private SpriteRenderer oops;
		private static float currentTime;
		private float waitingTime = 0.90f;

		private Entity holey_ref;
		private Entity moley_ref;

		public void Start()
		{
			currentTime = 0.0f;
			holey_ref = ECSManager.FindEntityByName("Holey");
			moley_ref = ECSManager.FindEntityByName("Moley");

			oops = ECSManager.FindEntityByName("oops").GetComponent<SpriteRenderer>();

			//PersistentSystem.SetValue("PrevScene", Scene.GetSceneName());
			PersistentSystem.SetValue(Scene.GetSceneName()+"StarsObtained", "0");

		}

		public void Update()
		{
			bool goToGameOver = false;

			if (holey_ref.GetComponent<HoleyController>().GetIsDead() && moley_ref.GetComponent<MoleyController>().GetIsDead())
			{
				goToGameOver = true;
				holey_ref.SetActive(false);
				moley_ref.SetActive(false);
			}

			if (goToGameOver)
			{
				if (!oops.isVisible)
				{
					oops.isVisible = true;
					ECSManager.FindEntityByName("oops").GetComponent<VFX_SlapOn>().SlapOn();
				}

				if (currentTime >= waitingTime)
				{
					currentTime = 0.0f;
					Scene.TransitionScene(Scene.GetSceneName(), 7f);
				}
				else
				{
					currentTime += Time.deltaTime;
				}
			}
		}


	}
}
