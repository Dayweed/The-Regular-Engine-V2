using System;
using System.Collections.Generic;
using System.Reflection;
using System.Runtime.CompilerServices;
using System.Security.Policy;
using static System.Runtime.CompilerServices.RuntimeHelpers;
using System.Threading;
using GlmSharp;
using System.Runtime.Remoting.Metadata.W3cXsd2001;

namespace TRE
{
	public class SceneLogic : Entity
	{
		private int lateStart = 0;

		private string currentSceneName;
		private string nextSceneName;

		public List<HoleCheckDisplay> triggerComplete;

		public List<List<HoleCheckDisplay>> triggerStars;

		public SpriteRenderer courseComplete;
		public static float currentTime;
		public float waitingTime = 0.90f;

		public bool forceGoToNextScene = false;

		VFX_Emerge StarEmerge;

		// Star Title
		Entity StarsCollected;
		Entity Stars1;
		Entity Stars2;
		Entity Stars3;
		float goalYPos = 0f;
		float hiddenYPos = -650f;
		float displayYPos = -400f;
		float titleMoveSpeed = 2f;
		float titleOffset = 0.05f;
		float timerCurrent = 0.0f;
		float timerDisplay = 3.0f;
		bool displayStars = false;

		Entity StarParticle;

		private ulong endsceneBGM;
		private ulong mainBGM;

        Entity blueberryPrefab = new Entity(7670209894207584463);
        Entity strawberryPrefab = new Entity(13004780274330328106);
		Entity blueberryCheat;
		Entity strawberryCheat;

        public void Start()
		{
			currentTime = 0.0f;
			currentSceneName = Scene.GetSceneName();
			triggerComplete = new List<HoleCheckDisplay>();
			triggerStars = new List<List<HoleCheckDisplay>>();

			Debug.Log("Name " + currentSceneName);

			PersistentSystem.SetValue("PrevScene", currentSceneName);

			if (currentSceneName == "Tutorial")
			{
				// Add for course complete triggers
				triggerComplete.Add(ECSManager.FindEntityByName("TriggerDisplay_3").GetComponent<HoleCheckDisplay>());
				triggerComplete.Add(ECSManager.FindEntityByName("TriggerDisplay_4").GetComponent<HoleCheckDisplay>());

				// Add for optional stars triggers
				List<HoleCheckDisplay> holeCheckDisplays = new List<HoleCheckDisplay>();
				holeCheckDisplays.Add(ECSManager.FindEntityByName("TriggerDisplay_1").GetComponent<HoleCheckDisplay>());
				holeCheckDisplays.Add(ECSManager.FindEntityByName("TriggerDisplay_2").GetComponent<HoleCheckDisplay>());
				triggerStars.Add(holeCheckDisplays);

				PersistentSystem.SetValue(currentSceneName + "MaxStarsObtained", "1");

				nextSceneName = "ResultScreen";
			}
			else if (currentSceneName == "Level_1")
			{
				// Add for course complete triggers
				// triggerComplete.Add(ECSManager.FindEntityByName("TriggerDisplay_3").GetComponent<HoleCheckDisplay>());
				// triggerComplete.Add(ECSManager.FindEntityByName("TriggerDisplay_4").GetComponent<HoleCheckDisplay>());

				// Add for optional stars triggers
				List<HoleCheckDisplay> holeCheckDisplays = new List<HoleCheckDisplay>
				{
					ECSManager.FindEntityByName("TriggerDisplay_1").GetComponent<HoleCheckDisplay>(),
					ECSManager.FindEntityByName("TriggerDisplay_2").GetComponent<HoleCheckDisplay>()
				};
				triggerStars.Add(holeCheckDisplays);

				PersistentSystem.SetValue(currentSceneName + "MaxStarsObtained", "1");

				nextSceneName = "ResultScreen";
			}
			courseComplete = ECSManager.FindEntityByName("CourseComplete").GetComponent<SpriteRenderer>();

			StarEmerge = ECSManager.FindEntityByName("Star_VFX").GetComponent<VFX_Emerge>();

			StarsCollected = ECSManager.FindEntityByName("TitleStarsCollected");
			Stars1 = ECSManager.FindEntityByName("Star1");
			Stars2 = ECSManager.FindEntityByName("Star2");
			Stars3 = ECSManager.FindEntityByName("Star3");
			goalYPos = displayYPos;
            timerCurrent = timerDisplay;

			//Star VFX
			StarParticle = ECSManager.Instantiate(new Entity(8119697912220926596));
			StarParticle.GetComponent<Particle>().IsActive = false;

			endsceneBGM = ECSManager.FindIDFromName("BGM_End");
			mainBGM = ECSManager.FindIDFromName("BGM");
		}

		public void Update()
		{
			#region CHEATS
			if (currentSceneName == "Tutorial")
			{
				if ((InputSystem.GetKeyHold(InputKeys.LeftControl) || InputSystem.GetKeyHold(InputKeys.RightControl)) && InputSystem.GetKeyPress(InputKeys.D1))
				{
					forceGoToNextScene = true;
				}
			}
			else if (currentSceneName == "Level_1")
			{
                if ((InputSystem.GetKeyHold(InputKeys.LeftControl) || InputSystem.GetKeyHold(InputKeys.RightControl)) && InputSystem.GetKeyPress(InputKeys.D1))
                {
                    forceGoToNextScene = true;
                }
                //if ((InputSystem.GetKeyHold(InputKeys.LeftControl) || InputSystem.GetKeyHold(InputKeys.RightControl)) && InputSystem.GetKeyPress(InputKeys.D1))
                //{
                //	MoleyController moley = ECSManager.FindEntityByName("Moley").GetComponent<MoleyController>();
                //	moley.GetComponent<Transform>().Position = new GlmSharp.vec3(180, 50, -330);

                //	HoleyController holey = ECSManager.FindEntityByName("Holey").GetComponent<HoleyController>();
                //	holey.GetComponent<Transform>().Position = new GlmSharp.vec3(180, 50, -310);
                //}

                //if ((InputSystem.GetKeyHold(InputKeys.LeftControl) || InputSystem.GetKeyHold(InputKeys.RightControl)) && InputSystem.GetKeyPress(InputKeys.D2))
                //{
                //	MoleyController moley = ECSManager.FindEntityByName("Moley").GetComponent<MoleyController>();
                //	moley.GetComponent<Transform>().Position = new GlmSharp.vec3(600, 120, -170);

                //	HoleyController holey = ECSManager.FindEntityByName("Holey").GetComponent<HoleyController>();
                //	holey.GetComponent<Transform>().Position = new GlmSharp.vec3(600, 120, -150);
                //}
            }

			if ((InputSystem.GetKeyHold(InputKeys.LeftControl) || InputSystem.GetKeyHold(InputKeys.RightControl)) && InputSystem.GetKeyRelease(InputKeys.D3))
			{
				Entity moley = ECSManager.FindEntityByName("Moley");
				Entity holey = ECSManager.FindEntityByName("Holey");

				if (ECSManager.IsValidEntity(moley.ID))
				{
					moley.GetComponent<MoleyController>().keepInventory = !moley.GetComponent<MoleyController>().keepInventory;
				}
				if (ECSManager.IsValidEntity(holey.ID))
				{
                    holey.GetComponent<HoleyController>().keepInventory = !holey.GetComponent<HoleyController>().keepInventory;
                    Debug.Log("KEEPINVENTORY MODE: " + holey.GetComponent<HoleyController>().keepInventory);
                }
            }

            if ((InputSystem.GetKeyHold(InputKeys.LeftControl) || InputSystem.GetKeyHold(InputKeys.RightControl)) && InputSystem.GetKeyRelease(InputKeys.D4))
			{
                Entity moley = ECSManager.FindEntityByName("Moley");
                Entity holey = ECSManager.FindEntityByName("Holey");

                if (ECSManager.IsValidEntity(moley.ID))
                {
                    moley.GetComponent<MoleyController>().creativeMode = !moley.GetComponent<MoleyController>().creativeMode;
                }
                if (ECSManager.IsValidEntity(holey.ID))
                {
                    holey.GetComponent<HoleyController>().creativeMode = !holey.GetComponent<HoleyController>().creativeMode;
                    Debug.Log("CREATIVE MODE: " + holey.GetComponent<HoleyController>().creativeMode);
                }
			}

			if ((InputSystem.GetKeyHold(InputKeys.LeftControl) || InputSystem.GetKeyHold(InputKeys.RightControl)) && InputSystem.GetKeyRelease(InputKeys.D5))
            {
                Entity moley = ECSManager.FindEntityByName("Moley");
                Entity holey = ECSManager.FindEntityByName("Holey");

                Entity blueberryCheat = new Entity(7670209894207584463);
                Entity strawberryCheat = new Entity(13004780274330328106);
                GlmSharp.vec3 newMoleyPos = new GlmSharp.vec3(moley.GetComponent<Transform>().Position.x, moley.GetComponent<Transform>().Position.y + 5f, moley.GetComponent<Transform>().Position.z);
                GlmSharp.vec3 newHoleyPos = new GlmSharp.vec3(holey.GetComponent<Transform>().Position.x, holey.GetComponent<Transform>().Position.y + 5f, holey.GetComponent<Transform>().Position.z);

                Entity moleyBlueberry = ECSManager.Instantiate(blueberryCheat);
                moleyBlueberry.GetComponent<Transform>().Position = newMoleyPos;

                Entity moleyStrawberry = ECSManager.Instantiate(strawberryCheat);
                moleyStrawberry.GetComponent<Transform>().Position = newMoleyPos;

                Entity holeyBlueberry = ECSManager.Instantiate(blueberryCheat);
                holeyBlueberry.GetComponent<Transform>().Position = newHoleyPos;

                Entity holeyStrawberry = ECSManager.Instantiate(strawberryCheat);
                holeyStrawberry.GetComponent<Transform>().Position = newHoleyPos;
            }
			#endregion


			// Late Start to ensure transform for stars arent screwed by parenting
			if (lateStart < 2) ++lateStart;
            if (lateStart == 2 && ECSManager.IsValidEntity(StarsCollected.ID) && ECSManager.IsValidEntity(Stars1.ID) && ECSManager.IsValidEntity(Stars2.ID) && ECSManager.IsValidEntity(Stars3.ID))
            {
                DetermineStarsDisplay(currentSceneName);
				++lateStart;
            }

            // Check if any of the list
            for (int i = triggerStars.Count - 1; i >= 0; --i)
			{
				List<HoleCheckDisplay> holeCheckDisplays = triggerStars[i];
				bool isCompleted = true;
				foreach (HoleCheckDisplay trigger in holeCheckDisplays)
				{
					if (!trigger.isCompleted)
					{
						isCompleted = false;
					}
				}
				if (isCompleted)
				{
					IncrementStars(currentSceneName);
					triggerStars.RemoveAt(i);
					if (StarEmerge != null)
					{
						StarEmerge.Emerge();

						//StarParticle.GetComponent<Transform>().Position = CameraSystem.GetMainCameraPosition();
						//	StarParticle.GetComponent<Transform>().Position += CameraSystem.GetMainCameraForwardVec().Normalized * 55f;
						//StarParticle.GetComponent<Transform>().Position = new GlmSharp.vec3(StarParticle.GetComponent<Transform>().Position.x, StarParticle.GetComponent<Transform>().Position.y - 10f, StarParticle.GetComponent<Transform>().Position.z);
						StarParticle.GetComponent<Transform>().Position = new GlmSharp.vec3(0f, -1500f,0f);
						StarParticle.GetComponent<Particle>().IsActive = true;
					}
				}
			}


            #region Stars
            if (ECSManager.IsValidEntity(StarsCollected.ID))
			{
				if (!displayStars && timerCurrent > 0) timerCurrent -= Time.deltaTime;
				// Do for stars collected
				vec3 titleStarsCollectedPos = StarsCollected.GetComponent<Transform>().Position;
				if (StarEmerge != null && StarEmerge.ReachEndPosition())
				{
					goalYPos = displayYPos;
					// Determine which stars to display
					DetermineStarsDisplay(currentSceneName);
					timerCurrent = timerDisplay;
					displayStars = true;
				}
				else if (!displayStars && timerCurrent <= 0.0f)
				{
					goalYPos = hiddenYPos;
				}
				// Lerp title to pos
				float titleStarsCollectedPosY = MathF.Lerp(StarsCollected.GetComponent<Transform>().Position.y, goalYPos, titleMoveSpeed * Time.deltaTime);
				StarsCollected.GetComponent<Transform>().Position = new vec3(titleStarsCollectedPos.x, titleStarsCollectedPosY, titleStarsCollectedPos.z);
				// Resume Text Bounce if it is close to the position
				if (goalYPos == displayYPos && Math.Abs(titleStarsCollectedPosY - titleStarsCollectedPos.y) < titleOffset)
				{
					displayStars = false;
				}
			}
			#endregion



			// Go to next scene if list of triggers are completed
			bool goToNextScene = triggerComplete.Count == 0 ? false : true;


			foreach (HoleCheckDisplay trigger in triggerComplete)
			{
				if (!trigger.isCompleted)
				{
					goToNextScene = false;
					if (!forceGoToNextScene)
					{
						return;
					}
				}
			}

			//go to next scene after a while
			if (goToNextScene || forceGoToNextScene)
			{
				if (!courseComplete.isVisible)
				{
					courseComplete.isVisible = true;
					ECSManager.FindEntityByName("CourseComplete").GetComponent<VFX_SlapOn>().SlapOn();
				}

				if (currentTime >= waitingTime)
				{
					currentTime = 0.0f;
					Scene.TransitionScene(nextSceneName, 4.5f);
				}

				else
				{
					currentTime += Time.deltaTime;
				}

				
				if (ECSManager.IsValidEntity(endsceneBGM))
				{
					AudioSystem.Play(endsceneBGM);
					AudioSystem.Stop(mainBGM);
				}
				else
				{
					AudioSystem.Stop(endsceneBGM);
				}
			}
		}

		public void IncrementStars(String mapName)
		{
			int numStars = 0;

			if (Int32.TryParse(PersistentSystem.GetValue("TotalStarsObtained"), out numStars))
			{
				++numStars;
				PersistentSystem.SetValue("TotalStarsObtained", numStars.ToString());
				Debug.Log("Stars " + PersistentSystem.GetValue("TotalStarsObtained"));
			}

			int mapStars = 0;
			if (Int32.TryParse(PersistentSystem.GetValue(mapName + "StarsObtained"), out mapStars))
			{
				++mapStars;
				PersistentSystem.SetValue(mapName + "StarsObtained", mapStars.ToString());
				Debug.Log(mapName + " Stars " + PersistentSystem.GetValue(mapName + "StarsObtained"));
			}
		}



		private void DetermineStarsDisplay(String mapName)
		{
			Stars1.SetActive(false);
			Stars2.SetActive(false);
			Stars3.SetActive(false);
			int mapStars = 0;
			if (Int32.TryParse(PersistentSystem.GetValue(mapName + "StarsObtained"), out mapStars))
			{
				//Debug.Log(mapName + "StarsObtained: " + mapStars);
			}
			if (mapStars >= 3)
			{
				Stars3.SetActive(true);
			}
			if (mapStars >= 2)
			{
				Stars2.SetActive(true);
			}
			if (mapStars >= 1)
			{
				Stars1.SetActive(true);
			}
		}
	}
}