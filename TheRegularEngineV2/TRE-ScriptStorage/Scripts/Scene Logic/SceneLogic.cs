using System;
using System.Collections.Generic;
using System.Dynamic;
using GlmSharp;

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

		private bool forceGoToNextScene = false;

		// cheats flag
		public bool keepinvCheat = false;
		public bool creativeCheat = false;
		public bool triggerCheats = false;

		VFX_Emerge StarEmerge;

		// Star Title
		Entity StarsCollected;
		Entity Stars1;
		Entity Stars2;
		Entity Stars3;
		// float goalYPos = 0f;
		// float hiddenYPos = -650f;
		// float displayYPos = -400f;
		// float titleMoveSpeed = 2f;
		// float titleOffset = 0.05f;
		// float timerCurrent = 0.0f;
		// float timerDisplay = 3.0f;
		// bool displayStars = false;

		Entity StarParticle;
		List<vec3> StarParticlePositions = new List<vec3>() { new vec3(-865, -440, 0), new vec3(-760, -440, 0), new vec3(-655, -440, 0) };
		Entity ConfettiParticleLeft;
		Entity ConfettiParticleRight;
		private bool confettiTime = false;
		private float mConfettiTimeDurian = 3f;
		private bool mConfettispawned = false;

        private ulong endsceneBGM;
		private ulong mainBGM;

		//Audio
		private ulong starSFX;
		private bool starSFXPlayed = false;

		Entity ConfettiTrigger;
		Entity EndingFlagTrigger;
		Entity Holey;
		Entity Moley;

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
				//triggerComplete.Add(ECSManager.FindEntityByName("TriggerDisplay_3").GetComponent<HoleCheckDisplay>());
				//triggerComplete.Add(ECSManager.FindEntityByName("TriggerDisplay_4").GetComponent<HoleCheckDisplay>());

				// Add for optional stars triggers
				List<HoleCheckDisplay> holeCheckDisplay1 = new List<HoleCheckDisplay>();
				List<HoleCheckDisplay> holeCheckDisplay2 = new List<HoleCheckDisplay>();
				List<HoleCheckDisplay> holeCheckDisplay3 = new List<HoleCheckDisplay>();
				holeCheckDisplay1.Add(ECSManager.FindEntityByName("TriggerDisplay_1").GetComponent<HoleCheckDisplay>());
				holeCheckDisplay1.Add(ECSManager.FindEntityByName("TriggerDisplay_2").GetComponent<HoleCheckDisplay>());
				holeCheckDisplay2.Add(ECSManager.FindEntityByName("TriggerDisplay_3").GetComponent<HoleCheckDisplay>());
				holeCheckDisplay2.Add(ECSManager.FindEntityByName("TriggerDisplay_4").GetComponent<HoleCheckDisplay>());
				holeCheckDisplay3.Add(ECSManager.FindEntityByName("TriggerDisplay_5").GetComponent<HoleCheckDisplay>());
				holeCheckDisplay3.Add(ECSManager.FindEntityByName("TriggerDisplay_6").GetComponent<HoleCheckDisplay>());
				triggerStars.Add(holeCheckDisplay1);
				triggerStars.Add(holeCheckDisplay2);
				triggerStars.Add(holeCheckDisplay3);

				starSFX = ECSManager.FindIDFromName("SFX_StarsCollected");

				PersistentSystem.SetValue(currentSceneName + "MaxStarsObtained", "3");

				nextSceneName = "ResultScreen";
			}
			else if (currentSceneName == "Level_1")
			{
				// Add for course complete triggers
				// triggerComplete.Add(ECSManager.FindEntityByName("TriggerDisplay_3").GetComponent<HoleCheckDisplay>());
				// triggerComplete.Add(ECSManager.FindEntityByName("TriggerDisplay_4").GetComponent<HoleCheckDisplay>());

				// Add for optional stars triggers
				List<HoleCheckDisplay> holeCheckDisplay1 = new List<HoleCheckDisplay>();
				List<HoleCheckDisplay> holeCheckDisplay2 = new List<HoleCheckDisplay>();
				List<HoleCheckDisplay> holeCheckDisplay3 = new List<HoleCheckDisplay>();
				holeCheckDisplay1.Add(ECSManager.FindEntityByName("TriggerDisplay_1").GetComponent<HoleCheckDisplay>());
				holeCheckDisplay1.Add(ECSManager.FindEntityByName("TriggerDisplay_2").GetComponent<HoleCheckDisplay>());
				holeCheckDisplay2.Add(ECSManager.FindEntityByName("TriggerDisplay_3").GetComponent<HoleCheckDisplay>());
				holeCheckDisplay2.Add(ECSManager.FindEntityByName("TriggerDisplay_4").GetComponent<HoleCheckDisplay>());
				holeCheckDisplay3.Add(ECSManager.FindEntityByName("TriggerDisplay_5").GetComponent<HoleCheckDisplay>());
				holeCheckDisplay3.Add(ECSManager.FindEntityByName("TriggerDisplay_6").GetComponent<HoleCheckDisplay>());
				triggerStars.Add(holeCheckDisplay1);
				triggerStars.Add(holeCheckDisplay2);
				triggerStars.Add(holeCheckDisplay3);

				PersistentSystem.SetValue(currentSceneName + "MaxStarsObtained", "3");

				nextSceneName = "ResultScreen";
			}
			else if (currentSceneName == "Level_2")
			{
				// Add for course complete triggers
				// triggerComplete.Add(ECSManager.FindEntityByName("TriggerDisplay_3").GetComponent<HoleCheckDisplay>());
				// triggerComplete.Add(ECSManager.FindEntityByName("TriggerDisplay_4").GetComponent<HoleCheckDisplay>());

				// Add for optional stars triggers
				List<HoleCheckDisplay> holeCheckDisplay1 = new List<HoleCheckDisplay>
				{
					ECSManager.FindEntityByName("TriggerDisplay_1").GetComponent<HoleCheckDisplay>(),
					ECSManager.FindEntityByName("TriggerDisplay_2").GetComponent<HoleCheckDisplay>()
				};
				/*List<HoleCheckDisplay> holeCheckDisplay2 = new List<HoleCheckDisplay>();
				List<HoleCheckDisplay> holeCheckDisplay3 = new List<HoleCheckDisplay>();*/
				/*holeCheckDisplay2.Add(ECSManager.FindEntityByName("TriggerDisplay_3").GetComponent<HoleCheckDisplay>());
				holeCheckDisplay2.Add(ECSManager.FindEntityByName("TriggerDisplay_4").GetComponent<HoleCheckDisplay>());
				holeCheckDisplay3.Add(ECSManager.FindEntityByName("TriggerDisplay_5").GetComponent<HoleCheckDisplay>());
				holeCheckDisplay3.Add(ECSManager.FindEntityByName("TriggerDisplay_6").GetComponent<HoleCheckDisplay>());*/
				triggerStars.Add(holeCheckDisplay1);
				/*triggerStars.Add(holeCheckDisplay2);
				triggerStars.Add(holeCheckDisplay3);*/

				PersistentSystem.SetValue(currentSceneName + "MaxStarsObtained", "1");

				nextSceneName = "ResultScreen";
			}
			courseComplete = ECSManager.FindEntityByName("CourseComplete").GetComponent<SpriteRenderer>();

			StarEmerge = ECSManager.FindEntityByName("Star_VFX").GetComponent<VFX_Emerge>();

			StarsCollected = ECSManager.FindEntityByName("TitleStarsCollected");
			Stars1 = ECSManager.FindEntityByName("Star1");
			Stars2 = ECSManager.FindEntityByName("Star2");
			Stars3 = ECSManager.FindEntityByName("Star3");
			// goalYPos = displayYPos;
			// timerCurrent = timerDisplay;

			//Star VFX
			StarParticle = ECSManager.Instantiate(new Entity(Prefab.GetPrefabIDFromName("Star Particle")));
			StarParticle.GetComponent<Particle>().IsActive = false;

			//Confetti VFX
			ConfettiParticleLeft = ECSManager.Instantiate(new Entity(Prefab.GetPrefabIDFromName("Confetti Particle Left")));
			ConfettiParticleRight = ECSManager.Instantiate(new Entity(Prefab.GetPrefabIDFromName("Confetti Particle Right")));
			ConfettiParticleLeft.GetComponent<Particle>().IsActive = false;
			ConfettiParticleRight.GetComponent<Particle>().IsActive = false;

			//For ending flag
			ConfettiTrigger = ECSManager.FindEntityByName("ConfettiTrigger");
			EndingFlagTrigger = ECSManager.FindEntityByName("EndingTrigger");
			Holey = ECSManager.FindEntityByName("Holey's Head Collider");
			Moley = ECSManager.FindEntityByName("Moley's Head Collider");

			endsceneBGM = ECSManager.FindIDFromName("BGM_End");
			mainBGM = ECSManager.FindIDFromName("BGM");

			confettiTime = false;
            mConfettispawned = false;
            mConfettiTimeDurian = 3f;

			// Load Cheats
            if (bool.TryParse(PersistentSystem.GetValue("powerUps"), out bool powerResult))
            {
                keepinvCheat = powerResult;
            }
            else
            { 
                keepinvCheat = false;
				string powerUpsString = keepinvCheat.ToString();
                PersistentSystem.SetValue("powerUps", powerUpsString);
            }

            if (bool.TryParse(PersistentSystem.GetValue("invulnerability"), out bool invResult))
            {
                creativeCheat = invResult;
            }
            else
            {
                creativeCheat = false;
				string invulnerabilityString = creativeCheat.ToString();
				PersistentSystem.SetValue("invulnerability", invulnerabilityString);
            }

            if (keepinvCheat || creativeCheat)
            {
				triggerCheats = true;
            }
            
        }

		public void Update()
		{
			#region CHEATS
			if (currentSceneName == "Tutorial")
			{
				if ((InputSystem.GetKeyHold(InputKeys.LeftControl) || InputSystem.GetKeyHold(InputKeys.RightControl)) && InputSystem.GetKeyPress(InputKeys.D1))
				{
					forceGoToNextScene = true;
                    confettiTime = true;
                }
			}
			else if (currentSceneName == "Level_1")
			{
				if ((InputSystem.GetKeyHold(InputKeys.LeftControl) || InputSystem.GetKeyHold(InputKeys.RightControl)) && InputSystem.GetKeyPress(InputKeys.D1))
				{
					forceGoToNextScene = true;
                    confettiTime = true;
                }

				if ((InputSystem.GetKeyHold(InputKeys.LeftControl) || InputSystem.GetKeyHold(InputKeys.RightControl)) && InputSystem.GetKeyPress(InputKeys.D2))
				{
					MoleyController moley = ECSManager.FindEntityByName("Moley").GetComponent<MoleyController>();
					//first one is to rolling obj section, second one is to last platforming section, third one is all the way to the last section
					//moley.GetComponent<Transform>().Position = new GlmSharp.vec3(180, 50, -330);
					//moley.GetComponent<Transform>().Position = new GlmSharp.vec3(602.175f, 107, -189.596f);
					moley.GetComponent<Transform>().Position = new vec3(970, 108, -193);

					HoleyController holey = ECSManager.FindEntityByName("Holey").GetComponent<HoleyController>();
					//holey.GetComponent<Transform>().Position = new GlmSharp.vec3(180, 50, -310);
					//holey.GetComponent<Transform>().Position = new GlmSharp.vec3(602.175f, 107, -174.596f);
					holey.GetComponent<Transform>().Position = new vec3(970, 108, -167);
				}

				//if ((InputSystem.GetKeyHold(InputKeys.LeftControl) || InputSystem.GetKeyHold(InputKeys.RightControl)) && InputSystem.GetKeyPress(InputKeys.D2))
				//{
				//	MoleyController moley = ECSManager.FindEntityByName("Moley").GetComponent<MoleyController>();
				//	moley.GetComponent<Transform>().Position = new GlmSharp.vec3(600, 120, -170);

				//	HoleyController holey = ECSManager.FindEntityByName("Holey").GetComponent<HoleyController>();
				//	holey.GetComponent<Transform>().Position = new GlmSharp.vec3(600, 120, -150);
				//}
			}
			else if (currentSceneName == "Level_2")
			{
				if ((InputSystem.GetKeyHold(InputKeys.LeftControl) || InputSystem.GetKeyHold(InputKeys.RightControl)) && InputSystem.GetKeyPress(InputKeys.D1))
				{
					forceGoToNextScene = true;
                    confettiTime = true;
                }

				if ((InputSystem.GetKeyHold(InputKeys.LeftControl) || InputSystem.GetKeyHold(InputKeys.RightControl)) && InputSystem.GetKeyPress(InputKeys.D2))
				{
					MoleyController moley = ECSManager.FindEntityByName("Moley").GetComponent<MoleyController>();
					//moley.GetComponent<Transform>().Position = new GlmSharp.vec3(64, 30, -174);
					//moley.GetComponent<Transform>().Position = new GlmSharp.vec3(293, 30, -138);
					//moley.GetComponent<Transform>().Position = new GlmSharp.vec3(406, 19, -430);
					moley.GetComponent<Transform>().Position = new vec3(600, 19, -430);

					HoleyController holey = ECSManager.FindEntityByName("Holey").GetComponent<HoleyController>();
					//holey.GetComponent<Transform>().Position = new GlmSharp.vec3(77, 30, -174);
					//holey.GetComponent<Transform>().Position = new GlmSharp.vec3(293, 30, -120);
					//holey.GetComponent<Transform>().Position = new GlmSharp.vec3(408, 19, -417);
					holey.GetComponent<Transform>().Position = new vec3(600, 19, -417);
				}
			}

			if ((InputSystem.GetKeyHold(InputKeys.LeftControl) || InputSystem.GetKeyHold(InputKeys.RightControl)) && InputSystem.GetKeyRelease(InputKeys.D3))
			{
				keepinvCheat = !keepinvCheat;
				triggerCheats = true;
			}

			if ((InputSystem.GetKeyHold(InputKeys.LeftControl) || InputSystem.GetKeyHold(InputKeys.RightControl)) && InputSystem.GetKeyRelease(InputKeys.D4))
			{
				creativeCheat = !creativeCheat;
                triggerCheats = true;
			}

            if (triggerCheats)
            {
                KeepInv(keepinvCheat);
                CreativeMode(creativeCheat);
                triggerCheats = false;
            }



			if ((InputSystem.GetKeyHold(InputKeys.LeftControl) || InputSystem.GetKeyHold(InputKeys.RightControl)) && InputSystem.GetKeyRelease(InputKeys.D5))
			{
				Entity moley = ECSManager.FindEntityByName("Moley");
				Entity holey = ECSManager.FindEntityByName("Holey");

				Entity blueberryCheat = new Entity(Prefab.GetPrefabIDFromName("Blueberry"));
				Entity strawberryCheat = new Entity(Prefab.GetPrefabIDFromName("Strawberry"));
				vec3 newMoleyPos = new vec3(moley.GetComponent<Transform>().Position.x, moley.GetComponent<Transform>().Position.y + 5f, moley.GetComponent<Transform>().Position.z);
				vec3 newHoleyPos = new vec3(holey.GetComponent<Transform>().Position.x, holey.GetComponent<Transform>().Position.y + 5f, holey.GetComponent<Transform>().Position.z);

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
            //Debug.Log(CameraSystem.GetMainCameraPosition().x.ToString() + " Y: " + CameraSystem.GetMainCameraPosition().y.ToString() + "Z: " + CameraSystem.GetMainCameraPosition().z.ToString());
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
					int currentStars = IncrementStars(currentSceneName);
					triggerStars.RemoveAt(i);
					if (StarEmerge != null && currentStars <= StarParticlePositions.Count)
					{
						StarParticle.GetComponent<Particle>().IsActive = true;
						StarParticle.GetComponent<Transform>().Position = new vec3(0f, -1500f, 0f);
						StarEmerge.Emerge(vec3.Zero, StarParticlePositions[currentStars - 1], new vec3(1.5f, 1.5f, 1));

						if (!starSFXPlayed)
						{
							if (ECSManager.IsValidEntity(starSFX))
								AudioSystem.Play(starSFX);
							starSFXPlayed = true;
						}


						StarParticle.GetComponent<Transform>().Position = CameraSystem.GetMainCameraPosition();
						StarParticle.GetComponent<Transform>().Position += CameraSystem.GetMainCameraForwardVec().Normalized * 55f;
						StarParticle.GetComponent<Transform>().Position = new vec3(StarParticle.GetComponent<Transform>().Position.x,
																				   StarParticle.GetComponent<Transform>().Position.y - 10f, 
																				   StarParticle.GetComponent<Transform>().Position.z);
					}
				}
			}

            if (PhysicsSystem.IsTriggerStay(Holey.ID, ConfettiTrigger.ID) && PhysicsSystem.IsTriggerStay(Moley.ID, ConfettiTrigger.ID))
			{
				confettiTime = true;
			}

            if (confettiTime)
			{
				mConfettiTimeDurian -= Time.GetDeltaTime();
				if (mConfettiTimeDurian <= 0)
				{
					confettiTime = false;
                    ConfettiParticleLeft.GetComponent<Particle>().IsActive = false;
                    ConfettiParticleRight.GetComponent<Particle>().IsActive = false;
                }
				else
				{
					if (!mConfettispawned)
					{
						ConfettiParticleLeft.GetComponent<Particle>().IsActive = true;
						ConfettiParticleRight.GetComponent<Particle>().IsActive = true;
						mConfettispawned = true;
					}
                }
			}

			#region Stars
			if (ECSManager.IsValidEntity(StarsCollected.ID))
			{
				//if (!displayStars && timerCurrent > 0) timerCurrent -= Time.deltaTime;
				// Do for stars collected
				// vec3 titleStarsCollectedPos = StarsCollected.GetComponent<Transform>().Position;
				if (StarEmerge != null && StarEmerge.FinishVFX())
				{
					// goalYPos = displayYPos;
					// Determine which stars to display
					DetermineStarsDisplay(currentSceneName);
					// timerCurrent = timerDisplay;

					if (ECSManager.IsValidEntity(starSFX))
						AudioSystem.Stop(starSFX);
					starSFXPlayed = false;
					// displayStars = true;
				}
				//else if (!displayStars && timerCurrent <= 0.0f)
				//{
				//	goalYPos = hiddenYPos;
				//}
				// Lerp title to pos
				//float titleStarsCollectedPosY = MathF.Lerp(StarsCollected.GetComponent<Transform>().Position.y, goalYPos, titleMoveSpeed * Time.deltaTime);
				//StarsCollected.GetComponent<Transform>().Position = new vec3(titleStarsCollectedPos.x, titleStarsCollectedPosY, titleStarsCollectedPos.z);
				// Resume Text Bounce if it is close to the position
				//if (goalYPos == displayYPos && Math.Abs(titleStarsCollectedPosY - titleStarsCollectedPos.y) < titleOffset)
				//{
				//	displayStars = false;
				//}
			}
			#endregion

			// Go to next scene if list of triggers are completed
			bool goToNextScene = triggerComplete.Count != 0;

			foreach (HoleCheckDisplay trigger in triggerComplete)
			{
				if (!trigger.isCompleted)
				{
					goToNextScene = false;
					if (!forceGoToNextScene)
						return;
				}
			}

			if (PhysicsSystem.IsTriggerStay(Holey.ID, EndingFlagTrigger.ID) && PhysicsSystem.IsTriggerStay(Moley.ID, EndingFlagTrigger.ID))
				goToNextScene = true;

			//go to next scene after a while
			if ((goToNextScene || forceGoToNextScene) && mConfettiTimeDurian <= 0)
			{
				if (!courseComplete.isVisible)
				{
					courseComplete.isVisible = true;
					ECSManager.FindEntityByName("CourseComplete").GetComponent<VFX_SlapOn>().SlapOn();

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

				if (currentTime >= waitingTime)
				{
					currentTime = 0.0f;
					Scene.TransitionScene(nextSceneName, 4.5f);
				}
				else
				{
					currentTime += Time.deltaTime;
				}
			}
		}

		// Return current map stars
		public int IncrementStars(string mapName)
		{
			if (int.TryParse(PersistentSystem.GetValue("TotalStarsObtained"), out int numStars))
			{
				++numStars;
				PersistentSystem.SetValue("TotalStarsObtained", numStars.ToString());
				Debug.Log("Stars " + PersistentSystem.GetValue("TotalStarsObtained"));
			}

			if (int.TryParse(PersistentSystem.GetValue(mapName + "StarsObtained"), out int mapStars))
			{
				++mapStars;
				PersistentSystem.SetValue(mapName + "StarsObtained", mapStars.ToString());
				Debug.Log(mapName + " Stars " + PersistentSystem.GetValue(mapName + "StarsObtained"));
				return mapStars;
			}

			return -1;
		}

		private void DetermineStarsDisplay(string mapName)
		{
			Stars1.SetActive(false);
			Stars2.SetActive(false);
			Stars3.SetActive(false);

			int.TryParse(PersistentSystem.GetValue(mapName + "StarsObtained"), out int mapStars);
			if (mapStars >= 3)
				Stars3.SetActive(true);
			if (mapStars >= 2)
				Stars2.SetActive(true);
			if (mapStars >= 1)
				Stars1.SetActive(true);
		}


        public void KeepInv(bool trigger)
        {
            Entity moley = ECSManager.FindEntityByName("Moley");
            Entity holey = ECSManager.FindEntityByName("Holey");

            if (ECSManager.IsValidEntity(moley.ID))
            {
                moley.GetComponent<MoleyController>().keepInventory = trigger;
            }
            if (ECSManager.IsValidEntity(holey.ID))
            {
                holey.GetComponent<HoleyController>().keepInventory = trigger;
                Debug.Log("KEEPINVENTORY MODE: " + holey.GetComponent<HoleyController>().keepInventory);
            }
        }

        public void CreativeMode(bool trigger)
        {
            Entity moley = ECSManager.FindEntityByName("Moley");
            Entity holey = ECSManager.FindEntityByName("Holey");

            if (ECSManager.IsValidEntity(moley.ID))
            {
                moley.GetComponent<MoleyController>().creativeMode = trigger;
            }
            if (ECSManager.IsValidEntity(holey.ID))
            {
                holey.GetComponent<HoleyController>().creativeMode = trigger;
                Debug.Log("CREATIVE MODE: " + holey.GetComponent<HoleyController>().creativeMode);
            }
        }
	}
}