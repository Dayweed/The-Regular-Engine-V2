using System;
using System.Collections.Generic;
using System.Reflection;
using System.Runtime.CompilerServices;
using System.Security.Policy;
using static System.Runtime.CompilerServices.RuntimeHelpers;
using System.Threading;

namespace TRE
{
	public class SceneLogic : Entity
	{
		private string currentSceneName;
		private string nextSceneName;

		public List<HoleCheckDisplay> triggerComplete;

		public List<List<HoleCheckDisplay>> triggerStars;

		public SpriteRenderer courseComplete;
		public static float currentTime;
		public float waitingTime = 0.90f;

		public bool forceGoToNextScene = false;

		VFX_Emerge StarEmerge;

		Entity StarParticle;

		private ulong endsceneBGM;
		private ulong mainBGM;

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
					MoleyController moley = ECSManager.FindEntityByName("Moley").GetComponent<MoleyController>();
					moley.GetComponent<Transform>().Position = new GlmSharp.vec3(180, 50, -330);

					HoleyController holey = ECSManager.FindEntityByName("Holey").GetComponent<HoleyController>();
					holey.GetComponent<Transform>().Position = new GlmSharp.vec3(180, 50, -310);
				}

				if ((InputSystem.GetKeyHold(InputKeys.LeftControl) || InputSystem.GetKeyHold(InputKeys.RightControl)) && InputSystem.GetKeyPress(InputKeys.D2))
				{
					MoleyController moley = ECSManager.FindEntityByName("Moley").GetComponent<MoleyController>();
					moley.GetComponent<Transform>().Position = new GlmSharp.vec3(600, 120, -170);

					HoleyController holey = ECSManager.FindEntityByName("Holey").GetComponent<HoleyController>();
					holey.GetComponent<Transform>().Position = new GlmSharp.vec3(600, 120, -150);
				}
			}

			if ((InputSystem.GetKeyHold(InputKeys.LeftControl) || InputSystem.GetKeyHold(InputKeys.RightControl)) && InputSystem.GetKeyPress(InputKeys.D3))
			{
				Entity moley = ECSManager.FindEntityByName("Moley");

				Entity blueberry = ECSManager.Instantiate(new Entity(7670209894207584463));
				GlmSharp.vec3 newPos = new GlmSharp.vec3(moley.GetComponent<Transform>().Position.x, moley.GetComponent<Transform>().Position.y + 30f, moley.GetComponent<Transform>().Position.z);
				blueberry.GetComponent<Transform>().Position = newPos;

				Entity strawberry = ECSManager.Instantiate(new Entity(13004780274330328106));
				strawberry.GetComponent<Transform>().Position = newPos;
			}

			if ((InputSystem.GetKeyHold(InputKeys.LeftControl) || InputSystem.GetKeyHold(InputKeys.RightControl)) && InputSystem.GetKeyPress(InputKeys.D4))
			{
				Entity holey = ECSManager.FindEntityByName("Holey");
				Entity blueberry = ECSManager.Instantiate(new Entity(7670209894207584463));
				GlmSharp.vec3 newPos = new GlmSharp.vec3(holey.GetComponent<Transform>().Position.x, holey.GetComponent<Transform>().Position.y + 30f, holey.GetComponent<Transform>().Position.z);
				blueberry.GetComponent<Transform>().Position = newPos;

				Entity strawberry = ECSManager.Instantiate(new Entity(13004780274330328106));
				strawberry.GetComponent<Transform>().Position = newPos;
			}
			#endregion

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

						if (currentSceneName == "Tutorial")
						{
							StarParticle.GetComponent<Transform>().Position = CameraSystem.GetMainCameraPosition();
							StarParticle.GetComponent<Transform>().Position += CameraSystem.GetMainCameraForwardVec().Normalized * 55f;
							StarParticle.GetComponent<Transform>().Position = new GlmSharp.vec3(StarParticle.GetComponent<Transform>().Position.x, StarParticle.GetComponent<Transform>().Position.y - 10f, StarParticle.GetComponent<Transform>().Position.z);

							StarParticle.GetComponent<Particle>().IsActive = true;
						}
					}
				}
			}

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
	}
}