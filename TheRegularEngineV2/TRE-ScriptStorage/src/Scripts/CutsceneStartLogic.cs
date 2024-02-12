using System;
using System.Collections.Generic;
using System.Reflection;
using System.Runtime.CompilerServices;
using static System.Runtime.CompilerServices.RuntimeHelpers;
using System.Threading;
using System.Linq;

namespace TRE
{
	public class CutsceneStartLogic : Entity
	{
		Entity Frame_1;
		Entity Frame_2;
		Entity Frame_3;
		Entity Frame_4;
		Entity Frame_5;
		Entity Frame_6;
		Entity SpaceToContinue;

		float currentTime = 0;
		float delayFrame = 5f;
		float delayScene = 7f;

		int currentFrame = 0;

		bool endCutscene = false;

		List<Entity> frames = new List<Entity>();
		List<string> nextScenes = new List<string>();
		List<string> forcedScenes = new List<string>(); // This requires the user to press space manually to go to the next scene


		public void Start()
		{
			Frame_1 = ECSManager.FindEntityByName("Frame1");
			Frame_2 = ECSManager.FindEntityByName("Frame2");
			Frame_3 = ECSManager.FindEntityByName("Frame3");
			Frame_4 = ECSManager.FindEntityByName("Frame4");
			Frame_5 = ECSManager.FindEntityByName("Frame5");
			Frame_6 = ECSManager.FindEntityByName("Frame6");
			SpaceToContinue = ECSManager.FindEntityByName("SpaceToContinue");

			Frame_1.SetActive(false);
			Frame_2.SetActive(false);
			Frame_3.SetActive(false);
			Frame_4.SetActive(false);
			Frame_5.SetActive(false);
			Frame_6.SetActive(false);
			SpaceToContinue.SetActive(false);

			frames = new List<Entity>() { Frame_1, Frame_2, Frame_3, Frame_4, Frame_5, Frame_6 };
			nextScenes = new List<string>() { "Frame3", "Frame4", "Frame5" };
			forcedScenes = new List<string>() { "Frame4" };

			currentFrame = 0;
			frames[currentFrame].SetActive(true);
			frames[currentFrame].GetComponent<VFX_FadeIn>().FadeIn();
			currentTime = delayFrame;
		}

		public void Update()
		{
			bool pressedSpace = InputSystem.GetKeyPress(InputKeys.Space);

			// Go to next scene
			if (pressedSpace && SpaceToContinue.GetActive() && currentFrame == frames.Count - 1)
			{
				Scene.TransitionScene("MainMenu", delayScene);
				endCutscene = true;
			}

			// Close Game
			if (InputSystem.GetKeyHold(InputKeys.Escape))
			{
				Game.CloseGame();
			}

			// Ignore if last frame alr
			if (currentFrame >= frames.Count) return;

			// Timer to go to next frame
			if (currentTime > 0)
			{
				currentTime -= Time.deltaTime;
			}

			if (!endCutscene)
			{
				if (pressedSpace || (currentFrame < frames.Count && !forcedScenes.Contains(frames[currentFrame].name) && frames[currentFrame].GetComponent<VFX_FadeIn>().DoneFading() && currentTime <= 0))
				{
					frames[currentFrame].GetComponent<VFX_FadeIn>().ForceComplete();
					++currentFrame;

					if (currentFrame >= frames.Count - 1 || forcedScenes.Contains(frames[currentFrame].name))
					{
						SpaceToContinue.SetActive(true);
					}
					else if (SpaceToContinue.GetActive())
					{
						SpaceToContinue.SetActive(false);
					}

					if (currentFrame >= frames.Count) return;

					// Check if deactivate all frames if counted as next scene
					if (nextScenes.Contains(frames[currentFrame].name))
					{
						FadeAllActive();
						currentTime = delayScene;
					}
					else
					{
						currentTime = delayFrame;
					}

					// Fade In next frame
					frames[currentFrame].SetActive(true);
					frames[currentFrame].GetComponent<VFX_FadeIn>().FadeIn();
				}
			}
		}

		private void FadeAllActive()
		{
			for (int i = 0; i < frames.Count; ++i)
			{
				if (frames[i].GetComponent<VFX_FadeIn>().DoneFading())
				{
					frames[i].GetComponent<VFX_FadeOut>().FadeOut();
				}
			}
		}
	}
}
