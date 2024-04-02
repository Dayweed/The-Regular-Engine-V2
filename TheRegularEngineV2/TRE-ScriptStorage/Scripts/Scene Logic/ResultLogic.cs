using System;

namespace TRE
{
	using AS = AudioSystem;
	public class ResultLogic : Entity
	{
		Entity CourseComplete;
		Entity Star_1;
		Entity Star_2;
		Entity Star_3;
		Entity Star_1_BG;
		Entity Star_2_BG;
		Entity Star_3_BG;
        Entity PartyHat;
        Entity Maracca;

        Entity LastStar = null;

        private float currentTimer;
		private bool startBufferComplete = false;
		private const float delayBufferStart = 2f;
		private const float delayBufferStars = 0.05f;

		int numStars = 0;
		int maxStars = 0;

		private ulong resultStarSFX;
		private ulong ResultSFX;

		public void Start()
		{
			
			CourseComplete = ECSManager.FindEntityByName("CourseComplete");
			Star_1 = ECSManager.FindEntityByName("Star_1");
			Star_2 = ECSManager.FindEntityByName("Star_2");
			Star_3 = ECSManager.FindEntityByName("Star_3");
			Star_1_BG = ECSManager.FindEntityByName("Star_1_BG");
			Star_2_BG = ECSManager.FindEntityByName("Star_2_BG");
			Star_3_BG = ECSManager.FindEntityByName("Star_3_BG");
            PartyHat = ECSManager.FindEntityByName("PartyHat");
            Maracca = ECSManager.FindEntityByName("Maracca");

			resultStarSFX = ECSManager.FindIDFromName("SFX_ResultStar");
			ResultSFX = ECSManager.FindIDFromName("SFX_Result");

			string prevSceneName = PersistentSystem.GetValue("PrevScene");
			if (int.TryParse(PersistentSystem.GetValue(prevSceneName + "StarsObtained"), out numStars) && int.TryParse(PersistentSystem.GetValue(prevSceneName + "MaxStarsObtained"), out maxStars))
			{
				if (maxStars <= 2)
					Star_3_BG.SetActive(false);
				if (maxStars <= 1)
					Star_2_BG.SetActive(false);
				if (maxStars <= 0)
					Star_1_BG.SetActive(false);
			}
			else
			{
				Star_1_BG.SetActive(false);
				Star_2_BG.SetActive(false);
				Star_3_BG.SetActive(false);
			}

			// Default false
			Star_1.SetActive(false);
			Star_2.SetActive(false);
			Star_3.SetActive(false);
            PartyHat.SetActive(false);
            Maracca.SetActive(false);

			currentTimer = delayBufferStart;
		}

		public void Update()
		{
			// Go to next scene
			if (InputSystem.GetKeyPress(InputKeys.Enter) || InputSystem.GetKeyPress(InputKeys.Space) || InputSystem.GetControllerButtonTriggered(0, InputSystem.Button.A))
			{
				// Determine which scene to go next
				// All levels go to the main menu, except the last level (goes to end cutscene instead).
				if (PersistentSystem.GetValue("PrevScene") == "Level_2")
					Scene.TransitionScene("CutsceneEnd", 4f);
				else
					Scene.TransitionScene("MainMenu", 4f);
			}

			// Close Game
			if (InputSystem.GetKeyHold(InputKeys.Escape))
			{
				Game.CloseGame();
			}

			if (!startBufferComplete)
			{
				currentTimer -= Time.deltaTime;
				if (currentTimer <= 0)
				{
					startBufferComplete = true;
					currentTimer = delayBufferStars;
				}
			}

			// Ignore if startBufferComplete is done
			if (!startBufferComplete) return;

			// Start running SlapOn vfx for each star
			// Slap on first star first
			if (Star_1_BG.GetActive() && !Star_1.GetActive() && numStars >= 1)
			{
				Star_1.SetActive(true);
				Star_1.GetComponent<VFX_SlapOn>().SlapOn();

				if (ECSManager.IsValidEntity(resultStarSFX))
				{
					AS.Play(resultStarSFX);
                }

                LastStar = (numStars == 1) ? Star_1 : null;
            }
			// Do Star_2 if Star_1 is done
			else if (Star_1.GetComponent<VFX_SlapOn>().CompletedVFX() && Star_2_BG.GetActive() && !Star_2.GetActive() && numStars >= 2)
			{
				Star_2.SetActive(true);
				Star_2.GetComponent<VFX_SlapOn>().SlapOn();

				if (ECSManager.IsValidEntity(resultStarSFX))
				{
					AS.Play(resultStarSFX);
				}

                LastStar = (numStars == 2) ? Star_2 : null;
            }
			// Do Star_3 if Star_2 is done
			else if (Star_2.GetComponent<VFX_SlapOn>().CompletedVFX() && Star_3_BG.GetActive() && !Star_3.GetActive() && numStars >= 3)
			{
				Star_3.SetActive(true);
				Star_3.GetComponent<VFX_SlapOn>().SlapOn();

				if (ECSManager.IsValidEntity(resultStarSFX))
				{
					AS.Play(resultStarSFX);
                }

                LastStar = (numStars == 3) ? Star_3 : null;
            }
			// Do PartyHat/Maracca if all stars are done
			else if (LastStar == null || LastStar.GetComponent<VFX_SlapOn>().CompletedVFX())
			{
                if (PersistentSystem.GetValue("PrevScene") == "Level_1" && PersistentSystem.GetValue("PartyHat") == "true" && !PartyHat.GetActive())
				{
					PartyHat.SetActive(true);
                    PartyHat.GetComponent<VFX_SlapOn>().SlapOn();
                }
                if (PersistentSystem.GetValue("PrevScene") == "Level_2" && PersistentSystem.GetValue("Maracca") == "true" && !Maracca.GetActive())
                {
                    Maracca.SetActive(true);
                    Maracca.GetComponent<VFX_SlapOn>().SlapOn();
                }
            }
		}
	}
}