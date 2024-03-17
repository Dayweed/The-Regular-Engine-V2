using System.Collections.Generic;

namespace TRE
{
	using AS = AudioSystem;
	public class CutsceneEndLogic : Entity
	{
		Entity Frame_1;
		Entity Frame_2;
		Entity Frame_3;
		Entity SpaceToContinue;

		float currentTime = 0;
		const float delayFrame = 5f;
		const float delayScene = 7f;

		int currentFrame = 0;
		int pressSpaceCounter = 0;

		bool endCutscene = false;
		bool pressedSpaceTwice = false;

		private ulong birdSFX;
		private ulong dialogueSFX;
		private ulong invitationSFX;
		private ulong endBGM;
		private ulong BGM;
		private bool invitationSFXPlayed = false;

		List<Entity> frames = new List<Entity>();
		List<string> nextScenes = new List<string>();

		public void Start()
		{
			Frame_1 = ECSManager.FindEntityByName("Frame1");
			Frame_2 = ECSManager.FindEntityByName("Frame2");
			Frame_3 = ECSManager.FindEntityByName("Frame3");
			SpaceToContinue = ECSManager.FindEntityByName("SpaceToContinue");

			Frame_1.SetActive(false);
			Frame_2.SetActive(false);
			Frame_3.SetActive(false);
			SpaceToContinue.SetActive(false);

			frames = new List<Entity>() { Frame_1, Frame_2, Frame_3 };
			nextScenes = new List<string>() { "Frame_3" };

			BGM = ECSManager.FindIDFromName("BGM");

			currentFrame = 0;
			frames[currentFrame].SetActive(true);
			frames[currentFrame].GetComponent<VFX_FadeIn>().FadeIn();
			currentTime = delayFrame;
		}

		public void Update()
		{
			bool pressedSpace = InputSystem.GetKeyPress(InputKeys.Space);
			bool pressA = InputSystem.GetControllerButtonTriggered(0, InputSystem.Button.A);
			bool hasPlayerPressed = pressedSpace || pressA;
			//bool paragraphIsHalfWay = false;

			//Go to Credits Scene
			if (hasPlayerPressed && /*SpaceToContinue.GetActive() && */currentFrame == frames.Count - 1)
			{
				if (ECSManager.IsValidEntity(BGM))
					AS.Stop(BGM);

				Scene.TransitionScene("Credits_Scene", 5f);
				endCutscene = true;
			}

			if (currentFrame >= frames.Count) return;

			if (currentTime > 0)
			{
				currentTime -= Time.deltaTime;
			}

			if (!endCutscene)
			{
				//Every frame will go through this if statement (when its going to the next frame)
				if (hasPlayerPressed || (currentFrame < frames.Count &&
												 frames[currentFrame].GetComponent<VFX_FadeIn>().DoneFading() && currentTime <= 0))
				{
					frames[currentFrame].GetComponent<VFX_FadeIn>().ForceComplete();

					if (currentFrame >= 3 && currentFrame <= 7)
						++pressSpaceCounter;

					//player can press double space to go next frame OR once the paragraph is done press space once to go next frame
					if (currentFrame >= 3 && currentFrame <= 7 && (hasPlayerPressed && pressedSpaceTwice))
					{
						++currentFrame;
						pressedSpaceTwice = false;
						pressSpaceCounter = 0;
					}

					else if (!(currentFrame >= 3 && currentFrame <= 7))
						++currentFrame;

					if (ECSManager.IsValidEntity(dialogueSFX))
						AS.Stop(dialogueSFX);

					//currentframe == 3/4/5/6/7, SpaceToContinueBlack
					//letter frames + last frame + out of bounds frame will go through this if statement
					if (currentFrame >= frames.Count - 1)
					{
						SpaceToContinue.SetActive(true);
					}
					//disable SpaceToContinue
					else if (SpaceToContinue.GetActive())
					{
						SpaceToContinue.SetActive(false);
					}

					if (currentFrame >= frames.Count) return;

					// Check if deactivate all frames if counted as next scene
					//Every new frame that appears
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
					frames[i].GetComponent<VFX_FadeOut>().FadeOut();
			}
		}
	}
}
