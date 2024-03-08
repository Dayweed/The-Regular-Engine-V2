using System;
using System.Collections.Generic;
using System.Reflection;
using System.Runtime.CompilerServices;
using static System.Runtime.CompilerServices.RuntimeHelpers;
using System.Threading;
using System.Linq;

namespace TRE
{
	using AS = AudioSystem;

	public class CutsceneStartLogic : Entity
	{
		Entity Frame_1;
		Entity Frame_2;
		Entity Frame_3;
		Entity Frame_4;
		Entity Frame_5;
		Entity Frame_6;
		Entity SpaceToContinue;
		Entity SpaceToContinueBlack;
		Entity Dialogue_1;
		Entity Dialogue_2;
		Entity Dialogue_3;
		Entity Dialogue_4;
		Entity Dialogue_5;

		float currentTime = 0;
		float delayFrame = 5f;
		float delayScene = 7f;

		int currentFrame = 0;

		bool endCutscene = false;

		private ulong birdSFX;
		private ulong dialogueSFX;
		private ulong invitationSFX;
		private ulong endBGM;
		private ulong BGM;
		private bool invitationSFXPlayed = false;

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
			SpaceToContinueBlack = ECSManager.FindEntityByName("SpaceToContinueBlack");
			Dialogue_1 = ECSManager.FindEntityByName("InvitationDialogue1");
			Dialogue_2 = ECSManager.FindEntityByName("InvitationDialogue2");
			Dialogue_3 = ECSManager.FindEntityByName("InvitationDialogue3");
			Dialogue_4 = ECSManager.FindEntityByName("InvitationDialogue4");
			Dialogue_5 = ECSManager.FindEntityByName("InvitationDialogue5");

			Frame_1.SetActive(false);
			Frame_2.SetActive(false);
			Frame_3.SetActive(false);
			Frame_4.SetActive(false);
			Frame_5.SetActive(false);
			Frame_6.SetActive(false);
			SpaceToContinue.SetActive(false);
			SpaceToContinueBlack.SetActive(false);
			Dialogue_1.SetActive(false);

			frames = new List<Entity>() { Frame_1, Frame_2, Frame_3, Frame_4, Frame_5, Frame_6 };
			nextScenes = new List<string>() { "Frame3", "Frame4", "Frame5" };
			forcedScenes = new List<string>() { "Frame4" };

			birdSFX = ECSManager.FindIDFromName("SFX_Bird");
			dialogueSFX = ECSManager.FindIDFromName("SFX_DIalogue");
			invitationSFX = ECSManager.FindIDFromName("SFX_Invitation");
			endBGM = ECSManager.FindIDFromName("BGM_EndLoop");
			BGM = ECSManager.FindIDFromName("BGM");

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
				if (ECSManager.IsValidEntity(BGM))
					AS.Stop(BGM);

				if (ECSManager.IsValidEntity(endBGM))
					AS.Play(endBGM);

				Scene.TransitionScene("Tutorial", delayScene);
				endCutscene = true;
			}

			if (currentFrame == 2 )
			{
				if (ECSManager.IsValidEntity(birdSFX))
					AS.Stop(birdSFX);
			}

			if(currentFrame == 2 && !invitationSFXPlayed)
			{
				if (ECSManager.IsValidEntity(invitationSFX))
					AS.Play(invitationSFX);
					invitationSFXPlayed = true;
			}

			if (currentFrame == 3)
			{
				if (ECSManager.IsValidEntity(dialogueSFX))
					AS.Play(dialogueSFX);
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
				if (pressedSpace || (currentFrame < frames.Count && !forcedScenes.Contains(frames[currentFrame].name) &&
					frames[currentFrame].GetComponent<VFX_FadeIn>().DoneFading() && currentTime <= 0))
				{
					frames[currentFrame].GetComponent<VFX_FadeIn>().ForceComplete();
					++currentFrame;

					if (ECSManager.IsValidEntity(dialogueSFX))
						AS.Stop(dialogueSFX);

					//if frame 4, set space to continue to black
					if (currentFrame >= frames.Count - 1 || forcedScenes.Contains(frames[currentFrame].name))
					{
						if (currentFrame != 3)
							SpaceToContinue.SetActive(true);
						else
							SpaceToContinueBlack.SetActive(true);
					}
					else if (SpaceToContinue.GetActive() || SpaceToContinueBlack.GetActive())
					{
						SpaceToContinue.SetActive(false);
						SpaceToContinueBlack.SetActive(false);
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
